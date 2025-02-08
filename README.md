# *DIPlib 3*

[![Build Status](https://github.com/DIPlib/diplib/actions/workflows/cmake.yml/badge.svg?branch=master)](https://github.com/DIPlib/diplib/actions/workflows/cmake.yml)
[![CodeQL](https://github.com/DIPlib/diplib/actions/workflows/codeql.yml/badge.svg?branch=master)](https://github.com/DIPlib/diplib/security/code-scanning)

The *DIPlib* project contains:

- **[*DIPlib*](https://diplib.org/diplib-docs/)**, a C++ library for quantitative image analysis.
It has been in development at Delft University of Technology in The Netherlands since 1995.
The 3.0 release of *DIPlib* represented a complete rewrite in modern C++ of the library infrastructure,
with most of the image processing and analysis algorithms ported from the previous version,
and some of them improved significantly.
Read [the *DIPlib* documentation](https://diplib.org/diplib-docs/).

- **[*DIPimage*](https://diplib.org/DIPimage.html)**, a *MATLAB* toolbox for quantitative
image analysis. It has been in development at Delft University of Technology in The Netherlands since 1999.
Read [the *DIPimage* User Manual](https://diplib.org/diplib-docs/dipimage_user_manual.html).

- **[*PyDIP*](https://diplib.org/PyDIP.html)**, *Python* bindings to *DIPlib*.
This is currently a thin wrapper that exposes the C++ functionality with little change.
Read [the *PyDIP* User Manual](https://diplib.org/diplib-docs/pydip_user_manual.html).

- ***DIPviewer***, an interactive image display utility. It is usable from C++, *Python* and *MATLAB* programs.
Within *DIPimage* this is an optional alternative to the default *MATLAB*-native interactive display utility.
Read [the *DIPviewer* documentation](https://diplib.org/diplib-docs/dipviewer.html).

- ***DIPjavaio***, an interface to
[*OME Bio-Formats*](https://www.openmicroscopy.org/bio-formats/), a Java-based library that reads
hundreds of image file formats. This module is usable from C++ and *Python* (*DIPimage* interfaces
to *Bio-Formats* natively).
Read [the *DIPjavaio* documentation](https://diplib.org/diplib-docs/dipjavaio.html)

See [the *DIPlib* website](https://diplib.org/) for more information.


## Building and using the project

To build the library you will need a C++14 compliant compiler and *CMake*.
See [Building the *DIPlib* library](https://diplib.org/diplib-docs/building_diplib.html) for detailed instructions.

When linking against the *DIPlib* library without using *CMake*, it is important to set a few preprocessor
macros. See [Linking against the *DIPlib* library](https://diplib.org/diplib-docs/building_diplib.html#linking_diplib)
for details.


## Contributing

### Reporting a bug

We use the [issue tracker on GitHub](https://github.com/DIPlib/diplib/issues) to manage bug reports.
See [`CONTRIBUTING.md`](CONTRIBUTING.md) for instructions on how to report a bug.

### Bug fixes, algorithm improvements, new algorithms

Feel free to submit a [pull request on GitHub](https://github.com/DIPlib/diplib/pulls). Please follow
our [style guide](https://diplib.org/diplib-docs/styleguide.html) and make sure to read
[`CONTRIBUTING.md`](CONTRIBUTING.md) first.

### Documentation, tutorials

Documentation can always be improved (also, *PyDIP* has hardly any at all!). If you want to help write documentation,
or create tutorials for how to use the library, read [`CONTRIBUTING.md`](CONTRIBUTING.md), then submit a
[pull request on GitHub](https://github.com/DIPlib/diplib/pulls).

If you found an error in the documentation, we consider this a bug. See above how to report it.


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

*DIPlib* includes the following libraries / external code:

- *Eigen 3*  
  Copyright 2008 Gael Guennebaud  
  Copyright 2007-2011 Benoit Jacob  
  Mozilla Public License Version 2.0  
  see [`dependencies/eigen3/LICENSE.txt`](dependencies/eigen3/LICENSE.txt)  
  (we do not use any of the components with more restrictive licenses)

- *DocTest* (its use can be disabled)  
  Copyright 2016-2023 Viktor Kirilov  
  MIT License  
  see [`dependencies/doctest/LICENSE.txt`](dependencies/doctest/LICENSE.txt)

- *PocketFFT*  
  Copyright 2010-2021 Max-Planck-Society  
  Copyright 2019-2020 Peter Bell  
  3-Clause BSD License  
  see [`dependencies/pocketfft/LICENSE.md`](dependencies/pocketfft/LICENSE.md)

- *PCG Random Number Generation for C++*  
  Copyright 2014-2022 Melissa O'Neill and the PCG Project contributors  
  Apache 2.0 License, or The MIT License, at your option  
  see [`include/diplib/private/pcg_*.hpp`](include/diplib/private/pcg_random.hpp)

- *robin-map*, a fast hash map and hash set  
  Copyright 2017 Thibaut Goetghebuer-Planchon  
  MIT License  
  see [`include/diplib/private/robin_*.h`](include/diplib/private/robin_map.h)

- A fast 2D labeling algorithm out of *YACCLAB*  
  Copyright 2016-2017 Costantino Grana, Federico Bolelli, Lorenzo Baraldi and Roberto Vezzani  
  3-Clause BSD License  
  see [`src/regions/labelingGrana2016.h`](src/regions/labelingGrana2016.h)

- A few color maps from *colorcet*  
  Copyright 2017 Peter Kovesi  
  1-Clause BSD-like License  
  see [`src/display/colormap.cpp`](src/display/colormap.cpp) (about half-way down the file)

- Glyph images obtained by rendering the *Open Sans* font  
  Unknown copyright, designed by Steve Matteson  
  Apache 2.0 License  
  see [`src/generation/draw_text_builtin.cpp`](src/generation/draw_text_builtin.cpp) or [Google Fonts](https://fonts.google.com/specimen/Open+Sans#about)
 
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

- *libjpeg* (its use can be disabled)  
  Copyright 1991-2018, Thomas G. Lane, Guido Vollbeding  
  Custom BSD-like Licence  
  see [`dependencies/libjpeg/README`](dependencies/libjpeg/README)

- *libspng* (its use can be disabled)  
  Copyright 2018-2023, Randy  
  2-Clause BSD-like Licence  
  see [`dependencies/libspng/LICENSE`](dependencies/libspng/LICENSE)

- *zlib* (used by *libics*, *LibTIFF* and *libspng*, its use can be disabled)  
  Copyright 1995-2022 Jean-loup Gailly and Mark Adler  
  zlib Licence  
  see [`dependencies/zlib/LICENSE`](dependencies/zlib/LICENSE)

- *pybind11* (only used in the *Python* bindings)  
  Copyright 2016 Wenzel Jakob  
  3-Clause BSD License  
  see [`dependencies/pybind11/LICENSE`](dependencies/pybind11/LICENSE)

- *python-javabridge* (only used in the *Python* bindings)  
  Copyright 2003-2009 Massachusetts Institute of Technology  
  Copyright 2009-2013 Broad Institute  
  3-Clause BSD License  
  see [`pydip/src/loadjvm.py`](pydip/src/loadjvm.py)

- A few *MATLAB* scripts from *OME Bio-Formats* (used in *DIPimage*)  
  Copyright 2012-2021 Open Microscopy Environment  
  2-Clause BSD License  
  see [`dipimage/private/bf*.m`](dipimage/private/bfGetReader.m)  
  (the full *Bio-Formats* library needs to be installed for these to be useful)

- A few Java files from *OME Bio-Formats* (used only to resolve references during the build
  process of *DIPjavaio*, not used otherwise)  
  Copyright (C) 2005 - 2017 Open Microscopy Environment  
  2-Clause BSD License  
  see [`javaio/java/bioformats/readme.md`](javaio/java/bioformats/readme.md)  
  (the full *Bio-Formats* library needs to be installed to use *DIPjavaio*)

- Modified HTML templates and CSS files from *dox++*, which originally came from *m.css* (for documentation)  
  Copyright 2020-2021, Cris Luengo  
  Copyright 2017-2020 Vladimír Vondruš  
  MIT License

Note that all of these have permissive open-source licenses similar in spirit
to the Apache License.

*DIPlib* also optionally links against:

- *FFTW3* (as installed on your system, not used by default)  
  Copyright 2003, 2007-14 Matteo Frigo  
  Copyright 2003, 2007-14 Massachusetts Institute of Technology  
  GNU General Public License, Version 2 (**not compatible with the Apache License**)

- *FreeType 2* (as installed on your system, not used by default)  
  Copyright 1996-2021 David Turner, Robert Wilhelm, and Werner Lemberg  
  FreeType License (BSD style)

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

*DIPjavaio* links against:

- *JDK 8* (as installed on your system)  
  Copyright, probably Sun and/or Oracle  
  GNU General Public License, Version 2 (**not compatible with the Apache License**)

- *OME Bio-Formats* (optional, as installed on your system)  
  Copyright 2012-2017 Open Microscopy Environment  
  GNU General Public License, Version 2 (**not compatible with the Apache License**)

Note that [*OME Bio-Formats*](https://www.openmicroscopy.org/bio-formats/) is optional,
but *DIPjavaio* is currently useless without it.
The *DIPjavaio* sources provided with this project are shared under the same licence
as the rest of the project (Apache 2.0), but by linking with *Bio-Formats*, your
whole program will become GPL. This module is not suitable for use in commercial,
closed-source software.
