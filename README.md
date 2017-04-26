# DIPlib 3.0

*DIPlib* is a library for quantitative image analysis. It has been in development
at Delft University of Technology in The Netherlands since 1995. The 3.0 release
represents a complete rewrite in C++ of the library infrastructure, with most of
the image processing and analysis algorithms ported unmodified from the previous
version.

[Status](https://diplib.github.io/diplib-docs/workplan.html):
we have built most of the new infrastructure, and are in the process of
porting algorithms over. It's a slow process, but
[you can help](https://diplib.github.io/diplib-docs/workplan.html)!

See the [documentation](https://diplib.github.io/diplib-docs/) for more information.

## Building the library

To build the library under Unix or MacOS, you will need a C++ compiler and CMake.
Use the following commands to build:

    mkdir target
    cd target
    cmake /path/to/dip/root/directory
    make -j install

To change which compilers CMake uses:

    CC=gcc-5 CXX=g++-5 cmake /path/to/dip/root/directory

Available `make` targets:

    <default>   # builds all targets
    DIP         # builds the DIPlib library only
    DIPviewer   # builds the DIPviewer module only (plus the DIPlib library)
    PyDIP_bin   # builds the PyDIP Python module only (plus the DIPlib library)
    DIPimage    # builds the DIPimage MATLAB toolbox only (plus the DIPlib library)
    check       # builds the unit_tests program and runs it
    apidoc      # Doxygen HTML documentation for the library API
    install     # builds and installs all targets
    docs        # makes some PDF project documentation files
    tests       # deprecated -- old test functions

Important `cmake` command-line arguments:

    -DCMAKE_INSTALL_PREFIX=$HOME/dip        # choose an instal location
    -DCMAKE_BUILD_TYPE=Debug                # by default it is release
    -DDIP_EXCEPTIONS_RECORD_STACK_TRACE=Off # to disable stack trace generation on exception
    -DDIP_ENABLE_ASSERT=Off                 # to disable asserts
    -DDIP_ENABLE_DOCTEST=Off                # to disable doctest within DIPlib
    -DDIP_ENABLE_UNICODE=Off                # to disable UFT-8 strings within DIPlib
    -DDIP_BUILD_DIPVIEWER=Off               # to not build/install the DIPviewer module
    -DDIP_BUILD_PYDIP=Off                   # to not build/install the PyDIP Python module
    -DDIP_BUILD_DIPIMAGE=Off                # to not build/install the DIPimage MATLAB toolbox
    -DPYBIND11_PYTHON_VERSION=3             # to compile PyDIP agains Python 3

Under Windows you can follow a similar process, but I have never used CMake under
Windows, so I'll let someone else write this bit.

## License

*DIPlib* is distributed under the Apache 2.0 license. See the LICENCE.txt file.

DIPlib includes the following libraries / external code:

- Eigen 3
  Mozilla Public License Version 2.0
  see dependencies/eigen3/LICENSE.txt
  (we do not use any of the components with more restrictive licenses)

- DocTest
  Copyright (c) 2016 Viktor Kirilov
  The MIT License (MIT)
  see dependencies/doctest/LICENSE.txt

- The DFT algorithm out of OpenCV 3.1
  Copyright (C) 2000, Intel Corporation
  Intel License Agreement For Open Source Computer Vision Library
  see src/transform/opencv_dxt.h

- pybind11 (not used in the DIPlib library, only used in the Python bindings)
  Copyright (c) 2016 Wenzel Jakob <wenzel.jakob@epfl.ch>, All rights reserved.
  BSD-style license
  see dependencies/pybind11/LICENSE

Note that all of these have permissive open-source licenses similar in spirit
to the Apache License.
