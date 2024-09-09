\comment (c)2017-2024, Cris Luengo.

\comment Licensed under the Apache License, Version 2.0 [the "License"];
\comment you may not use this file except in compliance with the License.
\comment You may obtain a copy of the License at
\comment
\comment    http://www.apache.org/licenses/LICENSE-2.0
\comment
\comment Unless required by applicable law or agreed to in writing, software
\comment distributed under the License is distributed on an "AS IS" BASIS,
\comment WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
\comment See the License for the specific language governing permissions and
\comment limitations under the License.


\page pum_installation Installation

To install the package from PyPI, use
```bash
pip install diplib
```
Windows users might need to install the
[Microsoft Visual C++ Redistributable for Visual Studio 2015, 2017, 2019 and 2022](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist).

To read images through the Bio-Formats library (optional), you will need to download it separately:
```bash
python -m diplib download_bioformats
```
Bio-Formats also requires a working [Java installation](https://www.java.com/en/).

!!! attention "If you have problems"
    The `diplib` package on PyPI vendors the *OpenMP* library for some platforms
    (`libomp.dylib` on macOS, `libgomp.so` on Linux). It is possible, though rare, for another package
    to vendor an incompatible *OpenMP* library, and for the combined use to cause Python to crash.
    See for example [this issue](https://github.com/DIPlib/diplib/issues/130). If you happen to run into
    this problem, please [let us know!](https://github.com/DIPlib/diplib/issues/new/choose).
    You can find more information about the simultaneous use of multiple *OpenMP* libraries
    [on this page](https://github.com/joblib/threadpoolctl/blob/master/multiple_openmp.md).
