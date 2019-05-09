# PyDIP 3 {#PyDIP}

[//]: # (DIPlib 3.0)

[//]: # ([c]2018, Cris Luengo.)

[//]: # (Licensed under the Apache License, Version 2.0 [the "License"];)
[//]: # (you may not use this file except in compliance with the License.)
[//]: # (You may obtain a copy of the License at)
[//]: # ()
[//]: # (   http://www.apache.org/licenses/LICENSE-2.0)
[//]: # ()
[//]: # (Unless required by applicable law or agreed to in writing, software)
[//]: # (distributed under the License is distributed on an "AS IS" BASIS,)
[//]: # (WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.)
[//]: # (See the License for the specific language governing permissions and)
[//]: # (limitations under the License.)

Currently, *PyDIP* is a thin wrapper that makes most of the functionality in *DIPlib*
accessible from within Python.
Function names and arguments are mostly identical to the C++ functions. The only
exception is for indexing into images, which uses the Python slicing syntax.
Image objects created by the interface expose their data buffer, and so can be
used with most *NumPy* functions. Likewise, any object that exposes a data buffer
(e.g. *NumPy* arrays) can be used as input to *PyDIP* functions.

Documentation hasn't been added to these Python functions yet.

Images can be shown using the `Show` method, which uses *matplotlib*.
The `PyDIP.PyDIPviewer` sub-module gives access to \ref viewer.
When Python is started through the `examples/pydip.py` script, the `Show` function
will use the *DIPviewer* interactive display.
