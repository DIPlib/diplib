\comment (c)2016-2020, Cris Luengo.

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


\mainpage About *DIPlib 3*

# Introduction

*DIPlib* is an extensive C++ library for quantitative image analysis. It:

- contains hundreds of image processing and analysis algorithms,
- is designed for precision foremost,
- tries to be fast, but is not meant for real-time video processing,
- is easy to use, providing a simple and intuitive interface to algorithms,
- allows for writing programs that don't know at compile time what the data
  type or number of dimensions of the image to be processed are,
- reduces the edit-compile-run loop compared to other image processing libraries,
- makes it easy to express complex mathematical computations in code,
  see \ref why_tensors.

There are many other unique things about *DIPlib*, we encourage you to
explore the documentation to learn more about it. Good places to start
are the following documentation pages:

- The \ref dip::Image class, everything else revolves around it.
- All functionality is categorized into <a href="modules.html">modules</a>.
- The [`examples/`](https://github.com/DIPlib/diplib/tree/master/examples)
  directory in the repository, which contains simple C++ programs that demonstrate
  how to use various features of the library.
- \ref using_iterators, in case existing algorithms are not sufficient.
- \ref design, might help understand the library architecture.

# Modules, interfaces and bindings

Currently, *DIPlib 3* has interfaces or bindings to the following packages:

- *MATLAB*: [*DIPimage*](https://diplib.org/DIPimage.html) is a *MATLAB*
  toolbox that gives access to most functionality in *DIPlib*, but goes beyond
  that by providing a lot of additional functionality as M-file functions.

- *Python*: [*PyDIP*](https://diplib.org/PyDIP.html) is a thin wrapper of
  most functionality in *DIPlib*.

- *Bio-Formats*: \ref dipjavaio is an interface to Java-based image readers. It
  is designed to allow *DIPlib* to read hundreds of image file formats through
  [*OME Bio-Formats*](https://www.openmicroscopy.org/bio-formats/), but is
  generic enough to be used with other Java libraries as well.

- *OpenCV*: the \ref dip_opencv_interface provides copyless conversion to and from
  [*OpenCV*](https://opencv.org) images, for *OpenCV* version 2 and newer.

- *Vigra*: the \ref dip_vigra_interface provides copyless conversion to and from
  [*Vigra*](http://ukoethe.github.io/vigra/) images.

The *DIPlib* project further contains these additional modules:

- \ref dipviewer: an interactive image display utility.

# Building the project

See \ref building_summary for a quick summary and links to detailed, step-by-step
instructions.
