\comment (c)2017-2020, Cris Luengo.
\comment Based on original DIPimage user manual: (c)1999-2014, Delft University of Technology.

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


\page sec_dum_introduction Introduction

\section sec_dum_introduction_the_dipimage_toolbox The *DIPimage* toolbox

*MATLAB* is a software package designed for (among other things) data
processing. It contains a huge amount of numerical algorithms, and very
good data-visualization abilities. This makes it adequate for image
processing. However, *MATLAB*'s virtues do not end there. It is also an
ideal tool for rapid prototyping, since it handles a compact but simple
notation and it is very easy to add functions to it. The drawback is
that *MATLAB*, since it is an interpreted language, is slow for some
constructs like loops; it also is not very efficient with memory (for
example, all *MATLAB* data uses 8-byte floats). This makes it, by itself,
a bit less useful beyond the prototyping stage.

*DIPimage* is a *MATLAB* toolbox for quantitative image analysis, and is based on
the C++ library *DIPlib*. It is meant as a tool for research and development,
as well as teaching image processing at various levels. This toolbox is made with
user-friendliness, ease of implementation of new features, and
compactness of notation in mind. The toolbox contains many efficient algorithms,
though we always prioritize precision over speed. This means that, even though
this toolbox contains many very efficient algorithms, there might be other
(non-*MATLAB*) alternatives to think of if speed is your number one priority.

\section sec_dum_introduction_the_diplib_library The *DIPlib* library

*DIPlib* is a quantitative image analysis library written in C++. It
contains a large number of functions for processing and analyzing
multi-dimensional image data. The library provides functions for
performing transforms, filter operations, object generation, and
statistical analysis of images. It is also very efficient (with both
memory and time).

*DIPimage*, the *MATLAB* interface to *DIPlib* is more than a simple "glue" layer,
often changing the syntax of the *DIPlib* function calls to be more
natural within the *MATLAB* environment. Nonetheless, the help text for
functions will list the *DIPlib* function that is called.
\ref index "The on-line *DIPlib* reference" for the function can then be read
to learn more details about the algorithm and the meaning of the parameters.

More information on *DIPlib* can be found at [diplib.org](https://diplib.org)
and [its documentation](index.html).

\section sec_dum_introduction_image_analysis Image Analysis

This manual is meant as an introduction and reference to the *DIPimage*
toolbox, not as a tutorial on image analysis. Although
\ref sec_dum_gettingstarted shows some image analysis basics, it
is not complete. We refer to
["The Fundamentals of Image Processing"](https://ftp.imphys.tudelft.nl/DIPimage/docs/FIP2.3.pdf).

\section sec_dum_introduction_conventions Documentation conventions

The following conventions are used throughout this manual:

- Example code: in `typewriter` font

- File names: in `typewriter` font

- Function names/syntax: in `typewriter` font

- Keys: like **this**{ .m-label .m-warning }

- Mathematical expressions: in *italic*

- Menu names, menu items, and controls: "inside quotes"

\section sec_dum_introduction_acknowledgments Acknowledgments

*DIPlib* was written mainly by Michael van Ginkel and Geert van Kempen,
at the (then called) Pattern Recognition Group of Delft University of
Technology.
Cris Luengo rewrote the infrastructure in C++ for the 3.0 release.

The original *DIPimage* toolbox was written mainly by Cris Luengo, Lucas van Vliet,
Bernd Rieger and Michael van Ginkel, also at Delft University of Technology.
Tuan Pham, Kees van Wijk, Judith Dijk, Geert van Kempen and Peter Bakker
contributed functionality.
The 3.0 release was a rewrite of most components, to use the new version
of the *DIPlib* library, by Cris Luengo.
