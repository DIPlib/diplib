# DIPlib 3.0 {#mainpage}

[//]: # (DIPlib 3.0)

[//]: # ([c]2016-2017, Cris Luengo.)
[//]: # (Based on original DIPlib code: [c]1995-2014, Delft University of Technology.)

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

## Introduction

The purpose of the *DIPlib* project is to provide a one-stop library and
development environment for quantitative image analysis, be it applied
to microscopy, radiology, astronomy, or anything in between.

There are other image processing/analysis libraries available, some of
them hugely popular. Why do we keep investing time in developing and
improving *DIPlib*? The short answer is that we believe *DIPlib* offers
things that are not available elsewhere. The library is built on the
following three principles:

1. **Precision:**

   We implement the most precise known methods, and output often defaults to
   floating-point samples. The purpose of these algorithms is quantification,
   not approximation.

2. **Ease of use**

   We use modern C++ features to provide a simple and intuitive interface
   to algorithms, with expressive syntax, default values, and little
   boiler-plate code required from the user. There is no need to be aware of
   an image's data type to use the algorithms effectively.

   Furthermore, developing an image analysis program involves a lot of trial-and-error,
   rapid prototyping approaches are applicable: the edit-compile-run loop
   should be quick. We aim for short compile times with pre-compiled algorithms
   and few public templates.

3. **Efficiency**

   We implement the most efficient known algorithms, as long as they don't
   compromise precision. Ease-of-use features might also incur a slight overhead
   in execution times. The library can be used in high-throughput quantitative analysis
   pipelines, but is not designed for real-time video processing.

Algorithms in *DIPlib* typically accept input images of any data type (though,
of course, some algorithms are specific to binary images, or cannot handle
complex images) and any number of dimensions (algorithms that are limited to
one specific dimensionality typically show so in their name). The image data
type and dimensionality do not need to be known at compile time. Images can
have pixels that are vectors or matrices, for some examples on how this
relates to the three points above, see \ref why_tensors.

There are many other unique things about *DIPlib*, we encourage you to
explore the documentation to learn more about it. A good place to start
are the following documentation pages:

- The `dip::Image` class, everything else revolves around it.
- All functionality is categorized into <a href="modules.html">modules</a>.
- \ref using_iterators, in case existing algorithms are not sufficient.
- \ref design, might help understand the library architecture.

## A short history

Development of *DIPlib* started in 1995, at the capable hands of Geert
van Kempen and Michael van Ginkel, under direction of Lucas van Vliet,
at the Pattern Recognition Group (now Quantitative Imaging Group) of
Delft University of Technology. Most of the algorithms that had been
developed there were included in the library, together with a large
collection of standard algorithms. Due to the lack of a C++ standard at the
time, they developed the library in C, recreating much of the C++
functionality (templates, function overloading, exceptions, data
hiding, memory management)
using preprocessor macros and other tricks. *DIPlib* was originally used
on HPUX, Solaris and IRIX, and later on Windows, Linux, Mac OS 9 and
Mac OS X.

Originally, the *DIPlib* library was used from within the *SCIL_Image*
image processing software. In 1999, Cris Luengo (with a lot of help from
Michael van Ginkel) wrote an interface to *MATLAB*, defining a flexible
and intuitive command-line syntax for the development of image analysis
algorithms. That same year, a user-friendly GUI in the spirit of
*SCIL_Image* was written, as well as interactive image display tools. This
*MATLAB* toolbox, called *DIPimage*, became the primary interface to the
*DIPlib* library.

*DIPlib* 3.0 represents the first major rewrite of the *DIPlib* code base.
We have rewritten the infrastructure in C++14, using all of the original
ideas and concepts, but adding tensor images, color support, and other
ideas we had developed within the *DIPimage* toolbox. C++14 allows the
user to write code that is almost as simple as the equivalent *MATLAB*
code, making it simple to use the library even for rapid prototyping.
Hopefully, the new infrastructure is much easier to read, maintain, and
contribute to. We are in the process of porting all of the image analysis
routines to use the new infrastructure. See \ref workplan for progress and
a detailed list of what still needs to be done.

The *DIPimage* toolbox is being updated to optimally use *DIPlib* 3.0. This means
that some of the *MATLAB* code is being replaced with calls to *DIPlib*, and
the low-level interface (`dip_*` functions) disappears. The internal
representation of images has also changed. However, we strive
to keep backwards-compatibility in the high-level toolbox functions.

Main contributors to the original *DIPlib* project were:
* Geert van Kempen
* Michael van Ginkel
* Lucas van Vliet
* Cris Luengo
* Bernd Rieger

With additional contributions by:
* Ben Verwer
* Hans Netten
* J.W. Brandenburg
* Judith Dijk
* Niels van den Brink
* Frank Faas
* Kees van Wijk
* Tuan Pham
* Ronald Ligteringen

See \ref contributors.

## Contributing

If you want to contribute to the *DIPlib* project, there are many different
ways of doing so:

- Help port algorithms from the old *DIPlib* to the new infrastructure. Please
  coordinate with Cris Luengo before you get started. He can share the old code
  for the algorithms that need porting. See \ref workplan for a list of stuff to do.

- Write new algorithms. If you have an algorithm that you'd like to contribute
  to the project, we'll be happy to see it!

- Create an interface to another library or scripting language

- Fix bugs, improve documentation, add code to the unit tests

- Create a nice Doxygen theme for the documentation, create a logo for the project

In any of these cases, fork the project, create a new branch for your contribution,
and send a pull request. Do also read \ref styleguide, and make sure
you adhere to it. Don't be offended if you receive requests for modifications
before your work is merged with the project.

Your contributions will carry the same licencing terms as the rest of the library,
you keep the copyright to any substantial contribution.

## License

Copyright 2014-2017 Cris Luengo<br>
Copyright 1995-2014 Delft University of Technology

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

### Non-legalese description of license

The apache 2.0 license is a permissive open-source license. In short, this means that
you can use this software as you see fit, including making modifications, and distribute
this software, parts of it, and/or your modifications to it, either in source form or as
binaries. You are free to keep your modifications private, you are not required to
distribute sources with your binaries. HOWEVER, you must include proper attribution, as
well as the copyright notices, with any such distribution. You cannot pretend that you
wrote this software, and you cannot make it look like we endorse any software that you
wrote.

If you make modifications to this software, you are not required to share those with us,
but we certainly would appreciate any such contribution!

Note that this short description of the license does not replace the license text and
might not correctly represent all the legalese in the licence. Please read the actual
licence text if you plan to redistribute this software.
