DIPlib 3.0 {#mainpage}
==========

Preface
-------

The purpose of this project is to provide a one-stop library and
development environment for quantitative image analysis, be it applied
to microscopy, radiology, astronomy, or anything in between. Development
of DIPlib started in 1995, at the capable hands of Geert van Kempen and
Michael van Ginkel, under direction of Lucas van Vliet, at the Pattern
Recognition Group (now Quantitative Imaging Group) of Delft University
of Technology. Due to the lack of a C++ standard at the time, they
developed the library in C, recreating much of the C++ functionality
(templates, function overloading, exceptions, data hiding, automatic
destructor call at the end of a function's scope) using preprocessor
macros and other tricks. DIPlib was originally used on HPUX, Solaris
and IRIX, and later on Windows, Linux, Mac OS 9 and Mac OS X.

The project also encompasses the MATLAB toolbox DIPimage. We started
its development as a front-end to DIPlib in 1999. It contains a
user-friendly GUI and a flexible and intuitive command-line syntax.

DIPlib 3.0 represents the first major rewrite of the DIPlib code base.
We have rewritten the infrastructure in C++11, using all of the original
ideas and concepts, but adding tensor images, color support, and other
ideas we have developed within the DIPimage toolbox.
Hopefully, the new infrastructure is much easier to read, maintain, and
contribute to. All of the image analysis routines (have been / will be)
ported to use the new infrastructure.

The DIPimage toolbox will change to optimally use DIPlib 3.0. This means
that some of the MATLAB code will be replaced with calls to DIPlib, and
some (most?) of the low-level interface will change. The internal
representation of images will also change. However, we will strive
to keep the high-level toolbox functions identical.

Main contributors to the DIPlib project were:
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


Library philosophy
------------------

There are other image processing/analysis libraries available, some of
them hugely popular. Why do we keep investing time in developing and
improving DIPlib? The short answer is that we believe DIPlib offers
functionality that is not available elsewhere. This functionality is
intricately related to our philosophy:

1. The library user should not have to worry about image data types
(i.e. pixel representation) unless he/she wants to.

2. Algorithms are more elegant when they can be implemented in a
dimensionality-independent way.

3. Precision is more important than speed.

This means that most functions will accept an input image of any data
type and any number of dimensions, and produce a correct output in
a dynamically determined data type (output data types can differ from
input data types to avoid loss of precision). At the time of compilation,
the data type and dimensionality of an image is not necessarily known.
Most other C++ image libraries use a template for the image class,
requiring the programmer to specify the data type and dimensionality of
all declared images.

OpenCV, the most widely used image library, is geared towards computer
vision rather than quantitative image analysis. Many applications require
real-time computation, and therefore the library introduces imprecision
for the sake of speed. DIPlib is implemented with speed in mind,
implementing the most efficient known algorithms, using parallelism
where possible, etc., but will not cut corners for the extra time gain.
For example, the Gaussian smoothing filter produces a floating-point
output, even if the input is 8-bit integer, unless the user specifies
otherwise. Floating-point values occupy more space, and thus take more
time to process, than 8-bit integers, but the added precision in the
output makes it possible to measure more precisely.

There are many other unique things about DIPlib, we encourage you to
explore the documentation to learn more about it.


