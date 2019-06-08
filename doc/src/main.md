# About DIPlib 3 {#mainpage}

[//]: # (DIPlib 3.0)

[//]: # ([c]2016-2019, Cris Luengo.)
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

\section main_introduction Introduction

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
complex images, etc.) and any number of dimensions (algorithms that are limited to
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

See also the [`examples/`](https://github.com/DIPlib/diplib/tree/master/examples)
directory for a series of simple C++ programs that demonstrate how to use
various features of the library.

\section main_modules Modules, interfaces and bindings

Currently, *DIPlib 3* has interfaces or bindings to the following packages:

\m_class{m-spaced-list}

- *MATLAB*: \ref DIPimage is a MATLAB toolbox that gives access to most functionality
  in *DIPlib*, but goes beyond that by providing a lot of additional functionality
  as M-file functions.

- *Python*: \ref PyDIP is a thin wrapper of most functionality in *DIPlib*.

- *Bio-Formats*: \ref javaio is an interface to Java-based image readers. It
  is designed to allow *DIPlib* to read hundreds of image file formats through
  [*OME Bio-Formats*](https://www.openmicroscopy.org/bio-formats/), but is
  generic enough to be used with other Java libraries as well.

- *OpenCV*: the \ref dip_opencv_interface provides copyless conversion to and from
  [*OpenCV*](https://opencv.org) images, for *OpenCV* version 2 and newer.

- *Vigra*: the \ref dip_vigra_interface provides copyless conversion to and from
  [*Vigra*](http://ukoethe.github.io/vigra/) images.

The *DIPlib* project further contains these additional modules:

- *DIPviewer*: \ref viewer is an interactive image display utility.

\section main_support Support

The *DIPlib* project was originally developed at the Pattern Recognition
Group of Delt University of Technology, in the Netherlands. *DIPlib 3* is being
developed primarily by volunteers, but has had some financial support from:

- A European Research Council grant to Bernd Rieger, TU Delft
- Flagship Biosciences, Inc.
- Quantitative Imaging Systems, Inc.

See \ref history for a list of contributors.

\section main_contributing Contributing

If you want to contribute to the *DIPlib* project, there are many different
ways of doing so:

- Write new algorithms. If you have an algorithm that you'd like to contribute
  to the project, we'll be happy to see it!

- Create an interface to another library or scripting language.

- Create tutorials for how to use *DIPlib*, *DIPimage* and/or *PyDIP*.

- Fix bugs or improve documentation.

- Add code to the unit tests.

- Create a nice *Doxygen* theme for the documentation, or create a logo for the project.

In any of these cases, see <a href="https://github.com/DIPlib/diplib/blob/master/CONTRIBUTING.md">`CONTRIBUTING.md`</a>
to learn how to make optimal use of your time.
Don't be offended if you receive requests for modifications before your work is merged
with the project.

Your documentation and code contributions will carry the same licencing terms as the rest
of the library, you keep the copyright to any substantial contribution.

\section main_license License

\m_class{m-noindent}

Copyright 2014-2019 Cris Luengo and contributors<br>
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

\subsection main_simple_license Non-legalese description of license

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
