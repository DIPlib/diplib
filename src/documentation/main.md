# DIPlib 3.0 {#mainpage}

## Introduction

The purpose of the *DIPlib* project is to provide a one-stop library and
development environment for quantitative image analysis, be it applied
to microscopy, radiology, astronomy, or anything in between.

There are other image processing/analysis libraries available, some of
them hugely popular. Why do we keep investing time in developing and
improving *DIPlib*? The short answer is that we believe *DIPlib* offers
functionality that is not available elsewhere. This functionality is
intricately related to our philosophy:

1. The library user should not have to worry about image data types
(i.e. pixel representation) unless he/she wants to.

2. Algorithms are more elegant when they can be implemented in a
dimensionality-independent way.

3. Speed is important, but not at the cost of precision.

This means that most functions will accept an input image of any data
type and any number of dimensions, and produce a correct output in
a dynamically determined data type (output data types can differ from
input data types to avoid loss of precision). At the time of compilation,
the data type and dimensionality of an image are not necessarily known.
Most other C++ image libraries use a template for the image class,
requiring the programmer to specify the data type and dimensionality of
all declared images.

*OpenCV*, the most widely used image library, is geared towards computer
vision rather than quantitative image analysis. Many applications require
real-time computation, and therefore the library introduces imprecision
for the sake of speed. *DIPlib* is implemented with speed in mind,
implementing the most efficient known algorithms, using parallelism
where possible, etc., but will not cut corners for the extra time gain.
For example, the Gaussian smoothing filter produces a floating-point
output, even if the input is 8-bit integer, unless the user specifies
otherwise. Floating-point values occupy more space, and thus take more
time to process, than 8-bit integers, but the added precision in the
output makes it possible to measure more precisely.

There are many other unique things about *DIPlib*, we encourage you to
explore the documentation to learn more about it. A good place to start
are the following documentation pages:

- The `dip::Image` class, everything else revolves around it.
- All image analysis functions are defined in the `#dip` \ref dip "namespace".
- \ref using_iterators, in case existing algorithms are not sufficient.
- \ref design, might help understand the library architecture.

## A short history

Development of *DIPlib* started in 1995, at the capable hands of Geert
van Kempen and Michael van Ginkel, under direction of Lucas van Vliet,
at the Pattern Recognition Group (now Quantitative Imaging Group) of
Delft University of Technology. Most of the algorithms that had been
developed there were included in the library, together with a large
array of standard algorithms. Due to the lack of a C++ standard at the
time, they developed the library in C, recreating much of the C++
functionality (templates, function overloading, exceptions, data
hiding, automatic destructor call at the end of a function's scope)
using preprocessor macros and other tricks. *DIPlib* was originally used
on HPUX, Solaris and IRIX, and later on Windows, Linux, Mac OS 9 and
Mac OS X.

Originally, the *DIPlib* library was used from within the *SCIL_Image*
image processing software. In 1999, Cris Luengo (with a lot of help from
Michael van Ginkel) wrote an interface to MATLAB, defining a flexible
and intuitive command-line syntax for the development of image analysis
algorithms. That same year, a user-friendly GUI in the spirit of
SCIL_Image was written, as well as interactive image display tools. This
MATLAB toolbox, called *DIPimage*, became the primary interface to the
*DIPlib* library.

*DIPlib* 3.0 represents the first major rewrite of the *DIPlib* code base.
We have rewritten the infrastructure in C++11, using all of the original
ideas and concepts, but adding tensor images, color support, and other
ideas we had developed within the *DIPimage* toolbox.
Hopefully, the new infrastructure is much easier to read, maintain, and
contribute to. All of the image analysis routines (have been / will be)
ported to use the new infrastructure. See \ref workplan for progress and
a detailed list of what still needs to be done.

The *DIPimage* toolbox will change to optimally use *DIPlib* 3.0. This means
that some of the MATLAB code will be replaced with calls to *DIPlib*, and
some (most?) of the low-level interface will change. The internal
representation of images will also change. However, we will strive
to keep the high-level toolbox functions identical.

Main contributors to the *DIPlib* project were:
* Geert van Kempen
* Michael van Ginkel
* Lucas van Vliet
* Cris Luengo
* Bernd Rieger

Additional contributions by:
* Ben Verwer
* Frank Faas
* Hans Netten
* J.W. Brandenburg
* Judith Dijk
* Kees van Wijk
* Niels van den Brink
* Ronald Ligteringen
* Tuan Pham

## Contributing

If you want to contribute to the *DIPlib* project, there are many different
ways of doing so:

- Help port algorithms from the old *DIPlib* to the new infrastructure. Please
  coordinate with Cris Luengo before you get started. He can share the old code
  for the algorithms that need porting. See \ref workplan for a list of stuff to do.

- Write new algorithms. If you have an algorithm that you'd like to contribute
  to the project, we'll be happy to see it!

- Fix bugs, improve documentation, add code to the unit tests, add interfaces
  to other libraries or scripting languages.

In any of these cases, fork the project, create a new branch for your contribution,
and send a pull request to Cris Luengo. Do also read \ref styleguide, and make sure
you adhere to it. Don't be offended if you receive requests for modifications
before your work is merged with the project.

Your contributions will carry the same licencing terms as the rest of the library.

## License

Copyright 2014-2017 Cris Luengo  
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
might not correctly represent all the legaleese in the licence. Please read the actual
licence text if you plan to redistribute this software.
