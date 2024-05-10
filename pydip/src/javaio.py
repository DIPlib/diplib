# (c)2022-2024, Cris Luengo, Wouter Caarls.
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

"""
The portion of the PyDIP module that contains the DIPjavaio functionality.
"""

import importlib.util

# Here we import PyDIPjavaio if it exists
if importlib.util.find_spec('diplib.PyDIPjavaio', __name__) is not None:
    _lib = None
    try:
        from . import loadjvm
        _lib = loadjvm.load_jvm()
        from .PyDIPjavaio import *
    except Exception as e:
        javaioError = str(e)
        if _lib:
            javaioError += f"\nload_jvm returned '{_lib}'"
        else:
            javaioError += "\nlibjvm not found"
        raise RuntimeError(javaioError)
else:
    raise RuntimeError("PyDIPjavaio was not included in the build of the package.")
