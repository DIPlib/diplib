# A short history {#history}

[//]: # (DIPlib 3.0)

[//]: # ([c]2016-2018, Cris Luengo.)
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

Development of *DIPlib* started in 1995, at the capable hands of Geert
van Kempen and Michael van Ginkel, under direction of Lucas van Vliet,
at the Pattern Recognition Group (now Quantitative Imaging Group) of
Delft University of Technology (TU Delft). Most of the algorithms that had been
developed there were included in the library, together with a large
collection of standard algorithms. Due to the lack of a C++ standard at the
time, they developed the library in C, recreating much of the C++
functionality (templates, function overloading, exceptions, data
hiding, memory management) using preprocessor macros and other tricks.
*DIPlib* was originally used on HPUX, Solaris and IRIX, and later on
Windows, Linux, Mac OS 9 and Mac OS X.

Originally, the *DIPlib* library was used from within the *SCIL_Image*
image processing software. In 1999, Cris Luengo (with a lot of help from
Michael van Ginkel) wrote an interface to *MATLAB*, defining a flexible
and intuitive command-line syntax for the development of image analysis
algorithms. That same year, a user-friendly GUI in the spirit of
*SCIL_Image* was written, as well as interactive image display tools. This
*MATLAB* toolbox, called *DIPimage*, became the primary interface to the
*DIPlib* library.

*DIPlib 3* represents the first major rewrite of the *DIPlib* code base.
We have rewritten the infrastructure in C++14, using all of the original
ideas and concepts, but adding tensor images, color support, and other
ideas we had developed within the *DIPimage* toolbox. C++14 allows the
user to write code that is almost as simple as the equivalent *MATLAB*
code, making it simple to use the library even for rapid prototyping.
Hopefully, the new infrastructure is much easier to read, maintain, and
contribute to. We are in the process of porting all of the image analysis
routines to use the new infrastructure. See \ref workplan for progress and
a detailed list of what still needs to be done.

The *DIPimage* toolbox is being updated to optimally use *DIPlib 3*. This means
that some of the *MATLAB* code is being replaced with calls to *DIPlib*, and
the low-level interface (`dip_*` functions) disappears. The internal
representation of images has also changed. However, we strive
to keep backwards-compatibility in the high-level toolbox functions.

Main contributors to the original *DIPlib* project were (all at TU Delft):
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

The DIPlib 3 project was initiated and is directed by Cris Luengo, with support from:

* Bernd Rieger, TU Delft
* Damir Sudar, Lawrence Berkeley National Laboratory
* Michel Nederlof, Quantitative Imaging Systems

The following people have contributed to the *DIPlib 3* project:

* Cris Luengo
* Wouter Caarls
* Ronald Ligteringen
* Erik Schuitema
* Erik Wernersson
* Filip Malmberg
