---
title: 'Implementation notes and thoughts'
author: 'Cris Luengo'
copyright: (c)2014-2017, Cris Luengo.
...

# Implementation notes and thoughts

## Fourier Transform

If the *FFTW* library is installed on the user's system, *DIPlib* will link against it
and use it to compute the FT. The CMakeLists script will have a setting that disables
this behaviour, as *FFTW* uses a GNU license and we want *DIPlib* to be totally
unencumbered by licenses. When not using *FFTW*, the built-in FFT algorithm is used.


## Processing images -- how to access and modify pixels

The code already documents the frameworks, iterators and neighbor lists, all
of which offer different ways of accessing pixels by abstracting strides.

One more way of accessing pixels, though not efficient, would be as follows:

The `dip::Pixel` class references a pixel in an image. It does not own the
shared pointer to the data segment, so it depends on the image existing.

```cpp
dip::Pixel px = img.PixelAt( x, y, z );
```

The pixel can then be indexed to access the various tensor elements, in the
same way that the image would be. It could hold a reference to the image's
`dip::Tensor` object and stride array to avoid the copies?

```cpp
dip::Image px1 = img.At( x, y, z );
dip::dfloat sample = static_cast< dip::dfloat >( px1[ i ] );
px1[ i ] = 10;

dip::Pixel px2 = img.PixelAt( x, y, z );
dip::dfloat sample = static_cast< dip::dfloat >( px2[ i ] );
px2[ i ] = 10;
```

The two bits of code above are identical, but `px2` would be quite a bit cheaper
to generate and to assign into.

A disadvantage would be that we need to allow all sorts of arithmetic on
`dip::Pixel` objects, and also in combination with `dip::Image`. It could be
possible to simplifiy this a little bit by implicit conversion of `dip::Pixel`
to `dip::Image`. For this, `dip::Pixel` would have a weak pointer to the
data segment of the image it came from. That way we can create a new shared
pointer when casting to an image:

```cpp
class Pixel {
   private:
      std::weak_ptr< void > dataBlock_; // always check dataBlock_.expired() before dereferencing origin_.
      void* origin_;
      dip::DataType dataType_;
      dip::Tensor const& tensor_; // const reference, so no shenanigans with this one
      dip::sint tensorStride_;
   public:
      Pixel( std::shared_ptr& dataBlock, void* origin, dip::DataType dataType,
             dip::Tensor const& tensor, dip::sint tensorStride ) :
             dataBlock_( dataBlock ), origin_( origin ), dataType_( dataType ),
             tensor_( tensor ), tensorStride_( tensorStride ) {}
      operator Image() {
         return { dataBlock_, origin_, dataType_, {}, {}, tensor_, tensorStride_ };
         // This does set externalData_ = true, which is not true...
         // The alternative is to make a construtor for dip::Image that takes a dip::Pixel.
      }
      // functions to get data type, tensor elements, tensor shape, etc.
      template< typename T > SampleIterator< T > begin() { ... }
      template< typename T > SampleIterator< T > end() { ... }
      dfloat operator[]( dip::uint index ) { ... } // pixel[ i ] => casts to dfloat
      template< typename T > T ValueAt() { ... } // pixel.ValueAt< uint8 >( i ) => casts to any type
      template< typename T > T& At() { ... } // pixel.At< uint8 >( i ) = 8 => type must match
};
```

The implicit cast to `dip::Image` (or non-explicit `dip::Image` constructor) would allow
a `dip::Pixel` to be used everywhere a 0D `dip::Image` can be used, including arithmetic. But:
it's less efficient than it needs to (`pixel1 + pixel2` would create two images, add them, then
return an image!). So, the pixel class would not be meant for computations where a matrix is
really required. It would be better to offer a cast from a `dip::Pixel` to an `eigen::Matrix`
(or `eigen::Map`). This cast would have to be defined in a header file to be explicitly included
when needed, to avoid pulling in Eigen into all compilation units.


## *MATLAB* interface

The *MATLAB* toolbox will be significantly simplified:

-   No need for the `libdml.so` / `libdml.dll` library, as there are no
    globals to store, and not much code to compile.

-   Basically, all the code from that library sits in a single
    header file to be included in each of the MEX-files:

    -   Casting an `mxArray` input to: `Image`, `IntegerArray`,
        `FloatArray`, `String`, `sint`, `uint`, or `double`.
        (**DONE**)

    -   Casting those types to an `mxArray` output. (**DONE**)

-   We won't need to convert strings to `enum`s, as [that will be done
    in the *DIPlib* library][Passing options to a function].

-   Some MEX-files will have more elaborate data conversion, as is the
    case now (e.g. `dip_measure`).

-   The few functions in the current *DIPlib* that use an `ImageArray` as
    input or output will use tensor images instead.

-   It seems that, at least in debug mode, the MEX-files with statically
    linked *DIPlib* are quite large. If the $rpath feature works well, we
    could keep *DIPlib* as a shared object. (**DONE**)

The M-file code will need to be adapted:

-   The `dip_image` object will not also double as `dip_image_array`
    (that was a mistake). Instead, tensor images will be a single data
    block, as they will be in *DIPlib*. Thus, some of the class methods
    will have to be rewritten, and some functions that use image arrays
    will have to be adapted. For an array of images, use a cell array.
    On the other hand, more functionality could be deferred to *DIPlib*.
    (**DONE**)

-   Much of the code for the `dip_measurement` object will change also,
    as its representation will likely change with the changes in
    *DIPlib*.

-   The colour conversion code will be replaced with a single call to
    *DIPlib*, as will some other functionality that will be translated
    from *MATLAB* to C++. Much of the tensor arithmetic should be done
    through *DIPlib* also. (**DONE**)

-   `dipshow` will be simplified, as simple calls to a *DIPlib* display
    function will generate the 2D array for display. (**DONE**)

### Issues with the current *MATLAB* interface

In *DIPimage* version 2.x, complex images need to be copied over in the
interface, as *DIPlib* and *MATLAB* represent them differently. There are
two solutions: *DIPlib* represents them as *MATLAB* does (which seems like
a really bad idea to me), or we cast them to a non-complex array that
*MATLAB* can be happy with even if it doesn't interpret it correctly.
Only when converting the `dip_image` object to a *MATLAB* array will the
copy be necessary. It could be simple to include a 'complex' dimension
as the first dimension of the `mxArray` (for real images, this would have
a size of 1). The new tensor dimension would then be the second dimension
(for scalar images, again of size 1). Thus, the mxArray would always be:
[complex, tensor, x, y, z, ...]. A reshape would get rid of the first
dimension to translate to a *MATLAB* array, in case of a real image.
A complex image would need a simple copy. (**DONE**)


## Image file formats

We will keep the TIFF and ICS readers/writers we currently have, using
[*libtiff*](http://www.remotesensing.org/libtiff/) and
[*libics*](http://libics.sourceforge.net).

For other file types, we could use [*Bio-Formats*](http://www.openmicroscopy.org/site/products/bio-formats).
It is a Java library, we'll probably use
[JNI](http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/jniTOC.html)
to interface between C++ and Java, see for example
[this tutorial](https://www.codeproject.com/Articles/993067/Calling-Java-from-Cplusplus-with-JNI).
See also [this ITK module](https://github.com/scifio/scifio-imageio) for
interfacing to *Bio-Formats*, which uses [SCIFIO](https://github.com/scifio/scifio).

We currently also have JPEG and GIF readers/writers, which are not very
useful and can be done away with. We have simplistic code to write Postscript
and CSV, which also aren't useful enough. The BioRad PIC reader is out of date,
nobody uses that format any more. Finally, we have an LSM reader based on the
TIFF reader, but if we use *Bio-Formats* it won't be necessary to maintain that
one either.
