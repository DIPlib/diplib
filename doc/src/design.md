\comment DIPlib 3.0

\comment (c)2014-2020, Cris Luengo.
\comment Based on original DIPlib code: (c)1995-2014, Delft University of Technology.

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


\page design *DIPlib* 3 design decisions

This page gives reasons behind some of the design choices of *DIPlib 3*.
Many of these decisions are inherited from the previous version of the library,
and some new ones are made possible by the port to C++.


\comment --------------------------------------------------------------

\section design_function_signatures Function signatures

There are two possible function signature styles for use in an image analysis
library:

1. `void Filter( dip::Image &in, dip::Image &out, int size );`

2. `dip::Image Filter( dip::Image &in, int size );`

Both of these options have advantages and disadvantages. Style 1 allows for
in-place operation:

```cpp
dip::Image img = ...
Filter( img, img, 1 );
```

The function here is able to write the results in the image's pixel buffer,
without having to allocate a temporary pixel buffer as would be the case for:

```cpp
dip::Image img = ...
img = Filter( img, 1 );
```

This is a huge advantage both in speed and memory usage. However, resulting
programs are not as easy to read (which parameters are inputs and which are
outputs?) and not as pretty as with style 2. For example, style 2 allows
for a very elegant chaining of operations:

```cpp
dip::Image img = ...
img = Filter2( Filter1( img, 3 ), 1 );
```

Furthermore, style 2 makes it much easier to automatically generate interfaces
to languages (such as *MATLAB*) that do not allow a function to modify its input
arguments. Such an automatic interface generation tool needs to know which
arguments are inputs and which are outputs.

In *DIPlib 2*, (written in C), all functions returned an
error code, and so output values had to be function arguments
(style 1). But in C++ we have exceptions to handle error conditions, and so
are free to have an image as the return value of the function (style 2).
However, the advantage of style 1 is too large to ignore. Therefore, we have
kept the function signature style (and argument order) of *DIPlib 2*.
However, we have written a small, inline wrapper function for most of the image
filters that follow the signature style 2. Such a wrapper is very straight-forward:

```cpp
inline dip::Image Filter( dip::Image &in, int size ) {
    dip::Image out;
    Filter( in, out, size );
    return out;
}
```

We have chosen not to pollute the documentation with these wrapper functions.
However, if a function `Filter( in, out )` exists, then you can assume that
there will also be a function `out = Filter( in )`.


\comment --------------------------------------------------------------

\section design_method_vs_function Class method vs function

Some libraries put all image processing/analysis functionality into the
image object as methods. The idea is to filter an image by
`img.Gauss(sigma)`. This is a terrible idea for many reasons: it's ugly, one
never knows if the image object is modified by the method or not, and the core
include file for the library changes when adding any type of functionality,
forcing recompilation of the whole library. Filters should be functions, not
methods.

In *DIPlib*, methods to the core \ref dip::Image class query and manipulate image
properties, not pixel data (with the exception of \ref dip::Image::Copy and
\ref dip::Image::Fill). Filters and other algorithms that manipulate image data are
always functions or function objects.

We use function objects sparingly in *DIPlib*. *ITK*, for example, has taken
the object-oriented approach to an extreme, where each algorithm is encapsulated
in an object, and parameters to the algorithm are set through object methods.
This leads to very verbose code that is not readable. For example, compare the
two code snippets below (function object version is not *ITK* code, but a simplified
version of that that ignores *ITK*'s templates and processing pipeline):

```cpp
lib::GaussianFilter gauss;
gauss.SetInput( image );
gauss.SetSigma( FloatArray{ 5, 1 } );
gauss.Execute();
outim = gauss.GetOutput();

outim = dip::Gauss( image, FloatArray{ 5, 1 } );
```


\comment --------------------------------------------------------------

\section design_dynamic_types Compile-time vs run-time pixel type identification

*DIPlib* uses run-time identification of an image's pixel type, and functions
dispatch internally to the appropriate sub-function. These sub-functions are
generated at compile time through templates

The alternative, seen in most C++ image analysis libraries (*ITK*, *Vigra*,
*CImg*, etc.), is to define the image class, as well as most functions, as
templates. The user declares an image having a specific data type (and dimensionality),
and the compiler then creates an image class with that data type for the pixels, as well
as instances of all functions called with this image as input. This takes time.
Compiling even a trivial program that uses *CImg* takes a minute, rather than a
fraction of a second it takes to compile a similar program that uses *DIPlib*.
Writing most functionality as templates implies that most code is actually in
the header files, rather than in the source files. This functionality then ends
up in the application executable, rather than in an independent library file
(shareable among many applications).

However, the largest disadvantage with templated functions happens when creating
an (even slightly) general image analysis program: you need to write code that
allows the user of your program to select the image data type, and write code
that does all the right dispatching depending on the data type (see, for example,
the *SimpleITK* interface to *ITK*). Alternatively,
you have to restrict the data type to one choice. A library is meant to take
away work from the programmer using the library, so it is logical that *DIPlib*
should allow all data types and do the dispatching as necessary. After all,
*DIPlib* is meant as a foundation for *DIPimage* and similar general-purpose
image analysis tools, where you cannot determine in advance which data type the
user will want to use. Think about how complicated each of the *DIPlib*-MEX-files
would be if *DIPlib* had compile-time typing: each MEX-file would have to check
the data type of the input array (or arrays), convert it to a *DIPlib* image
class of the same type, then call one of 8 or 10 instances of a function.
Furthermore, this MEX-file would need to know which image data types are
meaningful for the function being called (integer only? binary only? does it
work on complex values?). Instead, we simply convert the input array to a
*DIPlib* image and call a function. The MEX-file is trivial, the *DIPlib*
function itself takes care of everything.

*DIPlib* does expose a few templated functions to the user. However, these
templates typically abstract the type of a constant (see, for example, the
function `dip::Add` with a templated `rhs` argument), and never that of an image.
Such a template is always a trivial function that simplifies the library user's
code.


\comment --------------------------------------------------------------

\section design_options Passing options to a function

Many algorithms require parameters that select a mode of operation. *DIPlib 3*
uses strings for such parameters when the function is intended to be usable
from interfaces. By not defining C++ constants, the interface code can be
kept simple. For example, `dip::Dilation` has an option for the shape of the
structuring element. Instead of defining an `enum` (as *DIPlib 2* did)
with various values that the interface code needs to translate, the option
parameter is a string. The user of the interface and the user of the C++ library
see the same parameter and use the function in the same way. The overhead of a
few string comparisons is trivial compared to the computational cost of any
image analysis algorithm.

An other advantage is having fewer possibilities for name clashes when defining
a lot of enumerator constants for the many, many options accumulated of the
large collection of functions and algorithms in *DIPlib*.

A function that has multiple independent options takes a \ref dip::StringSet
(a `std::set<std::string>`) as input. The user can simply join strings using
curly braces, much like in *MATLAB*. The algorithm can easily check if a
specific string is given or not.

However, for infrastructure functions not typically exposed in interfaces (i.e.
the functions that *DIPlib* uses internally to do its work) we do define
numeric constants for options. For example, see the enumerator \ref dip::Option::ThrowException,
or any of the flags defined through \ref DIP_DECLARE_OPTIONS. These are more efficient
in use and equally convenient if limited to the C++ code.


\comment --------------------------------------------------------------

\section design_const_correctness Const correctness

When an image object is marked `const`, the compiler will prevent modifications
to it, it cannot be assigned to, and it cannot be used as the output argument
to a filter function. However, it is possible to create a non-`const` image that
points to the same data segment as a `const` image. The assignment operator, the
\ref dip::Image::QuickCopy method, and most indexing operations will do this.
There were two important reasons for this design decision:

1. Making a `const` and a non-`const` version of most of these indexing
   operators is possible, but some are functions (such as \ref dip::DefineROI) taking
   an output image object as function argument. This argument cannot be marked
   `const` because the function needs to modify it. However, the function must
   assign the data pointer from a `const` image into it.

2. Functions such as the framework functions need to make certain type of modifications
   to an input image, such as converting the tensor dimension to a spatial dimension,
   or applying singleton expansion. The simplest way of accomplishing this is to
   create a copy of the input image, and modify the copy. However, it would make no
   sense to make a copy of the data also, since the data are not modified. Thus,
   we need to make a non-`const` copy of a `const` image that shares its data pointer.

Thus, strictly forbidding data pointers from `const` images to be assigned to non-`const`
images would make it impossible to write certain types of functions, and would make
other types of functions much more complicated.

Because a copy of a `const` image can provide non-`const` access to its pixels, we felt
that it did not really make sense either to have the \ref dip::Image::Data,
\ref dip::Image::Origin, and \ref dip::Image::Pointer
methods return `const` pointers when applied to a `const` image. That is, all accesses
to pixel data ignore the constness of the image object. The constness of the image
object applies only to the image's properties (size, strides, tensor shape, color space,
etc.), not to its pixel values.

However, none of the functions in *DIPlib* will modify pixel values of a `const` image.
Input images to functions are always `const` references, and even though it would be
technically possible to modify its pixel values, we have an explicit policy to not do so.

The same applies to the \ref dip::Measurement object, but for a different reason:
Implementing correct handling of `const` objects would require two versions of all
iterators (a `const` one and a non-`const` one). Since these iterators are quite complex,
and the benefit of correct `const` handling is limited, we decided to follow the same
principle as with the \ref dip::Image object: non-`const` data access is always allowed, but
*DIPlib* has an explicit policy to not to change data of a `const` object.


\comment --------------------------------------------------------------

\section design_frameworks Frameworks

The frameworks simplify the writing of many image processing algorithms such that
they work on images of many different data types, are dimensionality independent,
and use multithreading. Framework functions also take on the task of testing input
images and creating output images. Typically, an image processing function only
needs to write a function that processes a single image line.

Such a line function is passed to the framework function, and the framework
function calls the line function for every image line. The line function might
need a state (parameters, intermediate data, output data). Furthermore, intermediate
and output data (i.e. data that the line function writes) must be separate for each
thread calling the line function. *DIPlib 2* did this in the typical C fashion:
a function pointer and a `void*` to the state. The framework function passed the
`void*` to the line function, which would cast it back to its original type. C++11
offers several alternatives that are more type-safe, and offer greater flexibility.

One option (that we did not choose) would be using `std::function`, which is an object
that encapsulates a function pointer, a lambda, or a functor with a predetermined
signature. It would be possible to write a lambda function that captures some variables
by reference, or to use `std::bind` to bind local variables to a function pointer. It
would also be possible to write a more complex class whose objects can be 'called'
with the required signature (functor). `std::function` has some overhead, especially
at creation.

The other option (that we did choose) is through derived classes and virtual functions.
A base class with a pure virtual function serves as the "model". An object of a derived
class, implementing a specific image analysis algorithm, can be referenced using a base
class pointer. The derived class can have variables that the algorithm uses,
including references to local variables in the caller's workspace. A benefit of this
option over the `std::function` is that we were able to define a second function
in the class, which the framework function calls once, before starting the processing,
and after it has decided how many threads to use. This allows the creation of
intermediate state variables for each thread. Without this facility, intermediate
state needs to be created for each potential thread (i.e. by examining the maximum
number of threads setting), which might be wasted effort if fewer threads will be
used.

A `std::function` might have offered more flexibility in how to implement the line
function, and would have allowed to write simple line functions inline, using a
lambda. However, the syntax using a derived class with a virtual function is somewhat
simpler and more straight-forward, and thus more accessible. This was the main reason
for us to choose the approach we chose.


\comment --------------------------------------------------------------

\section design_multithreading Multithreading

We use *OpenMP* for multithreading, mostly because it seems (to me) easier to use
than Intel's *Threading Building Blocks* (*TBB*). *TBB* does not require special
compiler support, but all modern compilers support *OpenMP* (except that XCode's CLang
on MacOS doesn't come with the *OpenMP* library, which needs to be installed separately).
The GNU Compiler Collection has very good support for *OpenMP*, and is available on
all platforms. *TBB* probably also plays better with C++ code than *OpenMP*, which
does not allow exceptions to be thrown across parallel construct boundaries. But
we're dealing only (so far) with trivially parallelizable code, so this is not
a major issue.

The framework functions determine, based on the number of operations to perform,
whether it is worthwhile to create threads for a particular computation. To do so,
they call a `GetNumberOfOperations` method of the line filter object. Each filter
thus needs to have such a method that determines how much work it will be to process
one image line. If the number of operations (clock cycles) is larger than a threshold,
multiple threads are started to process the image. I noticed that there is not a large
difference in overhead between starting one additional thread or starting three, so
it was not worth while to fine-tune the number of threads based on the number of
operations to perform.

The threshold was determined empirically on one single computer, and the way that
the number of operations per line is computed is imprecise and in some cases empirical.
It is more than likely that the threshold will not be optimal on a different machine.
Furthermore, for some filters it is not even possible to determine ahead of time the
number of operations
because it depends on the data (e.g. see the pixel table morphology line filter).

Thus, the point at which multiple threads are launched is imprecise at best.
However, what matters here is that, for very small images, we do not start threads
and double or worse the time spent on the filtering. *DIPlib 2* did not have
any such logic, always started threads within the frameworks, and consequently
behaved poorly with very small images. This system is intended to overcome that
problem.
