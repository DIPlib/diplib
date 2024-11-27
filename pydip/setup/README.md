# Python bindings to DIPlib 3 (a.k.a. PyDIP)

## Introduction

The purpose of the *DIPlib* project is to provide a one-stop library and
development environment for quantitative image analysis, be it applied
to microscopy, radiology, astronomy, or anything in between.

As opposed to all other image processing/analysis libraries and packages out
there, *DIPlib* focuses on quantification. The first priority is precision,
all other principles have a lower priority. Our principles are:

1. **Precision:**

   We implement the most precise known methods, and output often defaults to
   floating-point samples. The purpose of these algorithms is quantification,
   not approximation.

2. **Ease of use**

   Although our Python bindings are not much more than a thin wrapper of the
   C++ library functionality, the image analysis functionality is always easy
   to use. For example, the user does not, in general, need to be aware of
   the data type of the image to use these algorithms effectively.

3. **Efficiency**

   We implement the most efficient known algorithms, as long as they don't
   compromise precision. Ease-of-use features might also incur a slight overhead
   in execution times. The library can be used in high-throughput quantitative analysis
   pipelines, but is not designed for real-time video processing.

Besides an extensive collection of image processing and analysis algorithms,
this package contains *DIPviewer*, an interactive multi-dimensional image viewer,
and *DIPjavaio*, an interface to the
[*OME Bio-Formats*](https://www.openmicroscopy.org/bio-formats/) library.
The package is compatible with *NumPy* and any image processing package that uses
a *NumPy*-compatible way of representing images.

See [the *DIPlib* website](https://diplib.org/) for more information.

**Note!** We consider the Python bindings (*PyDIP*) to be in development. We aim at
not making breaking changes, but will sometimes do so when we feel it significantly
improves the usability of the module. These changes will always be highlighted in
the change logs and the release notification on the *DIPlib* website.
We recommend that you pin your project to use a specific version of the package
on PyPI, and carefully read the change logs before upgrading.

## Installation

To install, simply type

    pip install diplib

Windows users might need to install the
[Microsoft Visual C++ Redistributable for Visual Studio](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist).

To read images through the *Bio-Formats* library, you will need to download it
separately:

    python -m diplib download_bioformats

**Note:** The `diplib` package on PyPI vendors the *OpenMP* library for some platforms
(`libomp.dylib` on macOS, `libgomp.so` on Linux). It is possible, though rare, for another package to vendor
an incompatible *OpenMP* library, and for the combined use to cause Python to crash.
See for example [this issue](https://github.com/DIPlib/diplib/issues/130). If you happen to run into this problem,
please [let us know!](https://github.com/DIPlib/diplib/issues/new/choose).
You can find more information about the simultaneous use of multiple *OpenMP* libraries
[on this page](https://github.com/joblib/threadpoolctl/blob/master/multiple_openmp.md).

## Usage

The interface only has automatically generated docstrings that show the names of
each of the parameters, together with the first paragraph of the function's documentation,
except where the syntax differs from that of *DIPlib*.
Use the *DIPlib* reference to learn how to use each function.
Get started by reading [the *PyDIP* User Manual](/diplib-docs/pydip_user_manual.html).

These Jupyter notebooks replicate much of what is shown in the User Manual:

- [01_pydip_basics.ipynb](https://github.com/DIPlib/diplib-notebooks/blob/main/01_pydip_basics.ipynb)
- [02_filtering.ipynb](https://github.com/DIPlib/diplib-notebooks/blob/main/02_filtering.ipynb)
- [03_numpy_interaction.ipynb](https://github.com/DIPlib/diplib-notebooks/blob/main/03_numpy_interaction.ipynb)
- [04_tensor_images.ipynb](https://github.com/DIPlib/diplib-notebooks/blob/main/04_tensor_images.ipynb)

## License

Copyright 2014-2024 Cris Luengo and contributors  
Copyright 1995-2014 Delft University of Technology

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this library except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0  
   (or see the [`LICENSE.txt`](LICENSE.txt) file in this distribution)

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
