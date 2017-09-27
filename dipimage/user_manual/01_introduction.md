---
chapters: true
numberSections: true
sectionsDepth: 1
figPrefix:
  - "Figure"
  - "Figures"
tblPrefix:
  - "Table"
  - "Tables"
secPrefix:
  - "Section"
  - "Sections"
linkReferences: true
...

Introduction
============

The *DIPimage* toolbox
----------------------

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
(non-MATLAB) alternatives to think of if speed is your number one priority.

The *DIPlib* library
--------------------

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
[The on-line *DIPlib* reference](https://diplib.github.io/diplib-docs/)
for the function can then be read to learn more details
about the algorithm and the meaning of the parameters.

More information on *DIPlib* can be found at
<https://diplib.github.io/diplib-docs/> and <http://www.diplib.org/>.

Image Analysis
--------------

This manual is meant as an introduction and reference to the *DIPimage*
toolbox, not as a tutorial on image analysis. Although
[@sec:dum_gettingstarted] shows some image analysis basics, it
is not complete. We refer to
["The Fundamentals of Image Processing"](ftp://ftp.tudelft.nl/pub/DIPimage/docs/FIP2.3.pdf).

Documentation Conventions
-------------------------

The following conventions are used throughout this manual:

-   Example code: in `typewriter` font

-   File names: in `typewriter` font

-   Function names/syntax: in `typewriter` font

-   Keys: in **bold**

-   Mathematical expressions: in *italic*

-   Menu names, menu items, and controls: "inside quotes"

-   Description of incomplete features: in *italic*

Acknowledgments
---------------

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
