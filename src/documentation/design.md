DIPlib 3.0 design decisions {#design}
===

This page lists some of the design decisions that went into DIPlib 3.0.
Many of these decisions are inherited from the previous version of the library,
and some new ones are made possible by the port to C++.


Function signatures
---

There are two possible function signature styles for use in an image analysis
library:

1. `void Filter( Image &in, Image &out, int size );`

2. `Image Filter( Image &in, int size );`

Both of these options have advantages and disadvantages. Style 1 allows for
in-place operation:

    Image img = ...
    Filter( img, img, 1 );

The function here is able to write the results in the image's pixel buffer,
without having to allocate a temporary pixel buffer as would be the case for:

    Image img = ...
    img = Filter( img, 1 );

This is a huge advantage both in speed and memory usage. However, resulting
programs are not as easy to read (which parameters are inputs and which are
outputs?) and not as pretty as with style 2. For example, style 2 allows
for a very elegant chaingin of operations:

    Image img = ...
    img = Filter2( Filter1( img, 3 ), 1 );

Furthermore, style 2 makes it much easier to automatically generate interfaces
to languages (such as MATLAB) that do not allow a function to modify its input
arguments. Such an automatic interface generation tool needs to know which
arguments are inputs and which are outputs.

In the previous version of DIPlib (written in C), all functions returned an
error code, and so both input and output values were function arguments
(style 1). But in C++ we have exceptions to handle error conditions, and so
are free to have an image as the return value of the function (style 2).
However, the advantage of style 1 is too large to ignore. Therefore, we have
kept the function signature style (and argument order) of the old DIPlib.
However, we have written a small, inline wrapper function for most of the image
filters that follow the signature style 2. Such a wrapper is very straight-forward:

    inline Image Filter( Image &in, int size ) {
        Image out;
        Filter( in, out, size );
        return out;
    }

We have chosen not to pollute the documentation with these wrapper functions.
However, if a function `Filter( in, out )` exists, then you can assume that
there will also be a function `out = Filter( in )`.

