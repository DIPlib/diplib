# (c)2020, Wouter Caarls
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Contains code from python-javabridge
#
# Copyright (c) 2003-2009 Massachusetts Institute of Technology
# Copyright (c) 2009-2013 Broad Institute
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#     * Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#
#     * Neither the name of the Massachusetts Institute of Technology
#       nor the Broad Institute nor the names of its contributors may be
#       used to endorse or promote products derived from this software
#       without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MASSACHUSETTS
# INSTITUTE OF TECHNOLOGY OR THE BROAD INSTITUTE BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import ctypes
import os
import platform
import shutil
import subprocess


is_win = platform.system() == 'Windows'
is_mac = platform.system() == 'Darwin'
is_linux = platform.system() == 'Linux'


def suggest_javahome():
    if "JAVA_HOME" in os.environ:
        yield os.environ["JAVA_HOME"]

    java_bin = shutil.which("java")
    if java_bin:
        java_bin = os.path.realpath(java_bin)
        yield os.path.join(os.path.dirname(java_bin), "..")
        yield os.path.join(os.path.dirname(java_bin), "..", "jre")

    if is_win:
        import winreg
        java_key_paths = (
            'SOFTWARE\\JavaSoft\\JRE',
            'SOFTWARE\\JavaSoft\\Java Runtime Environment',
            'SOFTWARE\\JavaSoft\\JDK'
        )
        for java_key_path in java_key_paths:
            try:
                kjava = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, java_key_path)
                kjava_values = dict([winreg.EnumValue(kjava, i)[:2]
                                     for i in range(winreg.QueryInfoKey(kjava)[1])])
                current_version = kjava_values['CurrentVersion']
                looking_for = java_key_path + '\\' + current_version
                kjava_current = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, looking_for)
                kjava_current_values = dict([winreg.EnumValue(kjava_current, i)[:2]
                                             for i in range(winreg.QueryInfoKey(kjava_current)[1])])
                yield kjava_current_values['JavaHome']
            except WindowsError as e:
                if e.errno == 2:
                    continue
                else:
                    raise

    if is_mac:
        try:
            result = subprocess.check_output(["/usr/libexec/java_home"])
            yield result.strip().decode("utf-8")
        except subprocess.CalledProcessError:
            yield "/System/Library/Frameworks/JavaVM.framework/Home"


def suggest_jvm_paths():
    archs = ["amd64", "i386", "aarch64", "arm"]

    for jh in suggest_javahome():
        yield os.path.join(jh, "bin", "server")
        yield os.path.join(jh, "lib", "server")
        yield os.path.join(jh, "jre", "bin", "server")
        yield os.path.join(jh, "jre", "lib", "server")
        yield os.path.join(jh, "Libraries")

        for arch in archs:
            yield os.path.join(jh, "lib", arch, "server")
            yield os.path.join(jh, "jre", "lib", arch, "server")


def suggest_jvm_files():
    if is_win:
        yield "jvm.dll"

    if is_mac:
        yield "libjvm.dylib"
        yield "libclient.dylib"
        yield "libserver.dylib"

    if is_linux:
        yield "libjvm.so"


def load_jvm():
    for path in suggest_jvm_paths():
        for file in suggest_jvm_files():
            lib = os.path.join(path, file)

            if os.path.exists(lib):
                if is_win and ctypes.WinDLL(lib) != 0:
                    return lib
                if (is_linux or is_mac) and ctypes.CDLL(lib) != 0:
                    return lib
