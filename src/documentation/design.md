DIPlib 3.0 design decisions {#design}
===

This page lists some of the design decisions that went into *DIPlib 3.0*.
Many of these decisions are inherited from the previous version of the library,
and some new ones are made possible by the port to C++.


Function signatures
---

There are two possible function signature styles for use in an image analysis
library:

1. `void Filter( dip::Image &in, dip::Image &out, int size );`

2. `dip::Image Filter( dip::Image &in, int size );`

Both of these options have advantages and disadvantages. Style 1 allows for
in-place operation:

    dip::Image img = ...
    Filter( img, img, 1 );

The function here is able to write the results in the image's pixel buffer,
without having to allocate a temporary pixel buffer as would be the case for:

    dip::Image img = ...
    img = Filter( img, 1 );

This is a huge advantage both in speed and memory usage. However, resulting
programs are not as easy to read (which parameters are inputs and which are
outputs?) and not as pretty as with style 2. For example, style 2 allows
for a very elegant chaingin of operations:

    dip::Image img = ...
    img = Filter2( Filter1( img, 3 ), 1 );

Furthermore, style 2 makes it much easier to automatically generate interfaces
to languages (such as MATLAB) that do not allow a function to modify its input
arguments. Such an automatic interface generation tool needs to know which
arguments are inputs and which are outputs.

In the previous version of *DIPlib* (written in C), all functions returned an
error code, and so output values had to be function arguments
(style 1). But in C++ we have exceptions to handle error conditions, and so
are free to have an image as the return value of the function (style 2).
However, the advantage of style 1 is too large to ignore. Therefore, we have
kept the function signature style (and argument order) of the old *DIPlib*.
However, we have written a small, inline wrapper function for most of the image
filters that follow the signature style 2. Such a wrapper is very straight-forward:

    inline dip::Image Filter( dip::Image &in, int size ) {
        dip::Image out;
        Filter( in, out, size );
        return out;
    }

We have chosen not to pollute the documentation with these wrapper functions.
However, if a function `Filter( in, out )` exists, then you can assume that
there will also be a function `out = Filter( in )`.


Class method vs function
---

Some libraries put all image processing/analysis functionality into the
image object as methods. The idea is to filter an image by
`img.Gauss(sigma)`. This is a terrible idea for many reasons: it's ugly, one
never knows if the image object is modified by the method or not, and the core
include file for the library changes when adding any type of functionality,
forcing recompilation of the whole library. Filters should be functions, not
methods.

In *DIPlib*, methods to the core `dip::Image` class query and manipulate image
properties, not pixel data (with the exception of `dip::Image::Copy` and
`dip::Image::Set`). Filters and other algorithms that manipulate image data are
always functions or function objects.

We use function objects sparingly in *DIPlib*. *ITK*, for example, has taken
the object-oriented approach to an extreme, where each algorithm is encapsulated
in an object, and parameters to the algorithm are set through object methods.
This leads to very verbose code that is not readable. For example, compare the
two code snippets below (function object version is not *ITK* code, but a simplified
version of that that ignores *ITK*'s templates and processing pipeline):

    lib::GaussianFilter gauss;
    gauss.SetInput( image );
    gauss.SetSigma( FloatArray{ 5, 1 } );
    gauss.Execute();
    outim = gauss.GetOutput();

    outim = dip::Gauss( image, FloatArray{ 5, 1 } );



Compile-time vs run-time pixel type identification
---

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
templates typicaly abstract the type of a constant (see, for example, the
function `dip::Add` with a templated `rhs` argument), and never that of an image.
Such a template is always a trivial function that simplifies the library user's
code.


Passing options to a function
---

Many algorithms require parameters that select a mode of operation. *DIPlib 3.0*
uses strings for such parameters when the function is intended to be useable
from interfaces. By not defining C++ constants, the interface code can be
kept simple. For example, `dip::Dilation` has an option for the shape of the
structuring element. Instead of defining an `enum` (as the old *DIPlib* did)
with various values that the interface code needs to translate, the option
parameter is a string. The user of the interface and the user of the C++ library
see the same parameter and use the function in the same way. The overhead of a
few string comparisons is trivial compared to the computational cost of any
image analysis algorithm.

An other advantage is having fewer possibilities for name clashes when defining
a lot of enumerator constants for the many, many options accumulated of the
large collection of functions and algorithms in *DIPlib*.

However, for infrastrucute functions not typically exposed in interfaces (i.e.
the functions that *DIPlib* uses internally to do its work) we do define
numeric constants for options. For example, see the enumerator `dip::Options::ThrowException`,
or any of the flags defined through \ref `DIP_DECLARE_OPTIONS`. These are more efficient
in use and equally convenient if limited to the C++ code.


Const correctness
---

When an image object is marked `const`, the compiler will prevent modifications
to it, it cannot be assigned to, and it cannot be used as the output argument
to a filter function. However, it is possible to create a non-const image that
points to the same data as a const image. The assignment operator, the `dip::Image::QuickCopy`
method, and most indexing operations will do this. There were two important
reasons for this design decision:

1. Making a const and a non-const version of most of these indexing
   operators is possible, but some are functions (such as `dip::DefineROI`) taking
   an output image object as function argument. This argument cannot be marked
   `const` because the function needs to modify it. However, the function must
   assign the data pointer from a const image into it.

2. Functions such as the framework functions need to make certain type of modifications
   to an input image, such as converting the tensor dimension to a spatial dimension,
   or applying singleton expansion. The simplest way of accomplishing this is to
   create a copy of the input image, and modify the copy. However, it would make no
   sense to make a copy of the data also, since the data is not modified. Thus,
   we need to make a non-const copy of a const image that shares its data pointer.

Thus, strictly forbidding data pointers from const images to be assigned to non-const
images would make it impossible to write certain types of functions, and would make
other types of functions much more complicated.

Because a copy of a const image can provide non-const access to its pixels, we felt
that it did not really make sense either to have the `dip::Image::Data`,
`dip::Image::Origin`, and `dip::Image::Pointer`
methods return const pointers when applied to a const image. That is, all accesses
to pixel data ignore the constness of the image object. The constness of the image
object applies only to the image's properties (size, strides, tensor shape, color space,
etc.), not to its pixel values.

However, none of the functions in *DIPlib* will modify pixel values of a const image.
Input images to functions are always const references, and even though it would be
technically possible to modify its pixel values, we have an explicit policy to not do so.
