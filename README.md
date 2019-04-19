# *DIPlib 3*

[![Build Status](https://travis-ci.org/DIPlib/diplib.svg?branch=master)](https://travis-ci.org/DIPlib/diplib)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/DIPlib/diplib.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/DIPlib/diplib/context:cpp)

*DIPlib* is a library for quantitative image analysis. It has been in development
at Delft University of Technology in The Netherlands since 1995. The 3.0 release
represents a complete rewrite in modern C++ of the library infrastructure, with most of
the image processing and analysis algorithms ported from the previous version, and
some of them improved significantly. See the [change log](src/documentation/changes.md)
for a summary of changes.

*DIPlib 3* comes with MATLAB bindings (in a toolbox called *DIPimage*, in development
since 1999), and new Python bindings (called PyDIP). Many of the improvements to *DIPlib*
are a result of porting over concepts and ideas first implemented in *DIPimage*.

**Status**: We are in the process of creating the first beta release for the 3.0 branch.
Most, but not all functionality from the old *DIPlib* and *DIPimage* is (yet) present in
this branch. See the [open issues](https://github.com/DIPlib/diplib/issues) to find out
how you can help!

See the [documentation](https://diplib.github.io/diplib-docs/) for more information about
the library.


## Building the library

### Linux, MacOS, Cygwin and other Unix-like systems

To build the library you will need a C++14 compliant compiler and *CMake*.
Use the following commands to build:
```bash
mkdir target
cd target
cmake /path/to/dip/root/directory
make -j install
```

For detailed instructions, see [`INSTALL.md`](INSTALL.md).

### Windows

Unless you want to use *Cygwin* or *MinGW* (see above), we recommend *Microsoft Visual Studio 2017*
(version 15). You'll also need *CMake*.

Using *CMake-gui*, choose where the source directory is and where to build the binaries. Then
press "Configure" and select *Visual Studio*. Finally, press "Generate". You should now have
a *Visual Studio* solution file that you can open in *Visual Studio* and build as usual.

For step-by-step instructions, see [`INSTALL_Windows.md`](INSTALL_Windows.md). See also
[`INSTALL.md`](INSTALL.md) for additional information and optional dependencies you might want
to install.


## Linking against the library

When using *CMake*, and importing the `DIP` target into your project in the right way, you will just need
to link against the `DIP` target and everything will be configured correctly. Otherise, there are several
macros that you should define when building any program that links against *DIPlib*:

If *DIPlib* was build with the `DIP_SHARED_LIBRARY` flag was not set, then you need to define the `DIP__IS_STATIC`
macro when compiling the code that links against it. Likewise, if the `DIP_ALWAYS_128_PRNG` flag was set,
then you must define a `DIP__ALWAYS_128_PRNG` macro when compiling your program. Mismatching this flag
could cause your program to not link, or worse, crash at runtime.

The following flags do not need to be matched, but they should be if you want the inline functions to behave
the same as the pre-compiled ones:
 - flag: `DIP_ENABLE_STACK_TRACE` -- macro: `DIP__EXCEPTIONS_RECORD_STACK_TRACE`
 - flag: `DIP_ENABLE_ASSERT` -- macro: `DIP__ENABLE_ASSERT`

For DIPviewer, if `DIP_SHARED_LIBRARY` was not set, define the `DIP__VIEWER_IS_STATIC` macro. Also define
`DIP__HAS_FREEGLUT` or `DIP__HAS_GLFW` depending on which back-end is used.


## Contributing

### Reporting a bug

We use the [issue tracker on GitHub](https://github.com/DIPlib/diplib/issues) to manage bug reports.
See [`CONTRIBUTING.md`](CONTRIBUTING.md) for instructions on how to report a bug.

### Bug fixes, algorithm improvements, new algorithms

Feel free to submit a [pull request on GitHub](https://github.com/DIPlib/diplib/pulls). Please follow
our [style guide](https://diplib.github.io/diplib-docs/styleguide.html) and make sure to read
[`CONTRIBUTING.md`](CONTRIBUTING.md) first.

### Documentation, tutorials

Documentation can always be improved (also, *PyDIP* has hardy any at all!). If you want to help write documentation,
or create tutorials for how to use the library, read [`CONTRIBUTING.md`](CONTRIBUTING.md), then submit a
[pull request on GitHub](https://github.com/DIPlib/diplib/pulls).

If you found an error in the documentation, we consider this a bug. See above how to report it.


## License

Copyright 2014-2019 Cris Luengo and contributors  
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

*DIPlib* includes the following libraries / external code:

- *Eigen 3*  
  Mozilla Public License Version 2.0  
  see [`dependencies/eigen3/LICENSE.txt`](dependencies/eigen3/LICENSE.txt)  
  (we do not use any of the components with more restrictive licenses)

- *DocTest* (its use can be disabled)  
  Copyright 2016-2017 Viktor Kirilov  
  The MIT License (MIT)  
  see [`dependencies/doctest/LICENSE.txt`](dependencies/doctest/LICENSE.txt)

- The DFT algorithm out of *OpenCV* 3.1  
  Copyright 2000 Intel Corporation  
  Intel License Agreement For Open Source Computer Vision Library  
  see [`src/transform/opencv_dxt.cpp`](src/transform/opencv_dxt.cpp)

- *PCG Random Number Generation for C++*  
  Copyright 2014-2017 Melissa O'Neill and the PCG Project contributors  
  Apache 2.0 License, or The MIT License, at your option  
  see [`include/diplib/private/pcg_*.hpp`](include/diplib/private/pcg_random.hpp)

- A fast 2D labeling algorithm out of *YACCLAB*  
  Copyright 2016-2017 Costantino Grana, Federico Bolelli, Lorenzo Baraldi and Roberto Vezzani  
  3-Clause BSD License  
  see [`src/regions/labelingGrana2016.h`](src/regions/labelingGrana2016.h)

- A few color maps from *colorcet*  
  Copyright 2017 Peter Kovesi  
  1-Clause BSD-like License  
  see [`src/display/colormap.cpp`](src/display/colormap.cpp) (about half-way down the file)

- *libics* (its use can be disabled)  
  Copyright 2015-2017 Scientific Volume Imaging Holding B.V.  
  Copyright 2000-2013 Cris Luengo and others  
  GNU Lesser General Public License, Version 2.1  
  see [`dependencies/libics/GNU_LICENSE`](dependencies/libics/GNU_LICENSE)

- *LibTIFF* (its use can be disabled)  
  Copyright 1988-1997 Sam Leffler  
  Copyright 1991-1997 Silicon Graphics, Inc.  
  2-Clause BSD-like Licence  
  see [`dependencies/libtiff/COPYRIGHT`](dependencies/libtiff/COPYRIGHT)

- *zlib* (used by *libics* and *LibTIFF*, its use can be disabled)  
  Copyright 1995-2017 Jean-loup Gailly and Mark Adler  
  zlib Licence  
  see [`dependencies/zlib/LICENSE.txt`](dependencies/zlib/LICENSE.txt)

- *libjpeg* (its use can be disabled)  
  Copyright 1991-2018, Thomas G. Lane, Guido Vollbeding  
  Custom BSD-like Licence  
  see [`dependencies/libjpeg/README`](dependencies/libjpeg/README)

- *pybind11* (only used in the *Python* bindings)  
  Copyright 2016 Wenzel Jakob  
  3-Clause BSD License  
  see [`dependencies/pybind11/LICENSE`](dependencies/pybind11/LICENSE)

- A few *MATLAB* scripts from *OME Bio-Formats*  
  Copyright 2012-2017 Open Microscopy Environment  
  GNU General Public License, Version 2  
  see [`dipimage/private/bf*.m`](dipimage/private/bfGetReader.m)  
  (the full *Bio-Formats* library needs to be installed for these to be useful)

Note that all of these have permissive open-source licenses similar in spirit
to the Apache License, except for *OME Bio-Formats*.

*DIPlib* also optionally links against:

- *FFTW3* (as installed on your system, not used by default)  
  Copyright (c) 2003, 2007-14 Matteo Frigo  
  Copyright (c) 2003, 2007-14 Massachusetts Institute of Technology  
  GNU General Public License, Version 2 (not compatible with the Apache License)

*DIPviewer* links against the following libraries:

- *FreeGLUT* (as installed on your system, alternative to *GLFW*)  
  Copyright 1999-2000 Pawel W. Olszta  
  X-Consortium license

- *GLFW* (as installed on your system, alternative to *FreeGLUT*)  
  Copyright 2002-2006 Marcus Geelnard  
  Copyright 2006-2011 Camilla Berglund  
  BSD-like license

- *OpenGL* (as installed on your system)  
  (free from licensing requirements)

- *GLEW* (as installed on your system, for Windows only)  
  Copyright 2008-2016, Nigel Stewart  
  Copyright 2002-2008, Milan Ikits  
  Copyright 2002-2008, Marcelo E. Magallon  
  Copyright 2002, Lev Povalahev  
  Modified BSD License

*DIPjavaio* links against:

- *JDK 8* (as installed on your system)  
  Copyright, probably Sun and/or Oracle  
  GNU General Public License, Version 2 (not compatible with the Apache License)

- *OME Bio-Formats* (optional, as installed on your system)  
  Copyright 2012-2017 Open Microscopy Environment  
  GNU General Public License, Version 2

Note that [*OME Bio-Formats*](https://www.openmicroscopy.org/bio-formats/) is optional,
but *DIPjavaio* is currently useless without it.
The *DIPjavaio* sources provided with this project are shared unde the same licence
as the rest of the project (Apache 2.0), but by linking with *Bio-Formats*, your
whole program will become GPL. This modue is not suitable for use in commercial,
closed-source software.
