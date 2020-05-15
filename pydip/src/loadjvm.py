'''Code to load JVM, based on python-javabridge

python-javabridge is licensed under the BSD license.  See the
accompanying file LICENSE for details.

Copyright (c) 2003-2009 Massachusetts Institute of Technology
Copyright (c) 2009-2013 Broad Institute
All rights reserved.

'''

import os, platform
import shutil
import subprocess
import ctypes

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
            looking_for = java_key_path
            try:
                kjava = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, java_key_path)
                looking_for = java_key_path + "\\CurrentVersion"
                kjava_values = dict([winreg.EnumValue(kjava, i)[:2]
                                     for i in range(winreg.QueryInfoKey(kjava)[1])])
                current_version = kjava_values['CurrentVersion']
                looking_for = java_key_path + '\\' + current_version
                kjava_current = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,
                                                looking_for)
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
        except:
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
    if is_mac:
        libc = ctypes.CDLL("/usr/lib/libc.dylib")

    for path in suggest_jvm_paths():
        for file in suggest_jvm_files():
            lib = os.path.join(path, file)
            print("Checking ", lib)
            
            if os.path.exists(lib):
                if is_win and ctypes.WinDLL(lib) != 0:
                    return lib
                if is_mac and libc.dlopen_preflight(lib.encode('utf-8')) != 0:
                    return lib
                if is_linux and ctypes.CDLL(lib) != 0:
                    return lib

path = load_jvm()

if path is None:
    print("Cannot preload JVM")

print(path)
