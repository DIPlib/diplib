Using iterators to implement filters {#iterators}
===

DIPlib provides a set of iterators that can be used to efficiently visit all
pixels in an image, independently of the dimensionality of the image.
These iterators can be used to implement filters and other monadic operators.
This page shows how to use these iterators to write different types of image
processing functionality. However, most of the filters in DIPlib are not written
using these iterators, but using the functions in the `dip::Framework` namespace.
Those functions take care of many things, avoiding the need to replicate that
functionality in each filter:
 - parallel processing,
 - overloading for different data types,
 - boundary conditions, and
 - allocating the data segment for the output image(s).

But do note that using iterators is not necessarily less efficient than using
the framework functions. Depending on the application of the image processing
function you are writing, you can choose to use either an iterator or a
framework function.


Visiting each pixel in a single image
---

To loop over each pixel in an image and modify its value, you can use the
`dip::ImageIterator`:

    dip_ThrowIf( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
    dip::ImageIterator< dip::uint16 > it( img );
    do {
       *it *= 2;
    } while( ++it );

This construct works for an image of any dimensionality and size, but only for an
image of type `dip::uint16`. Instead of throwing an error if the image doesn't
match expectation, you can convert it:

    img.Convert( dip::DT_UINT16 );

To handle images of different data types with the same loop, write the loop in a
templated function, then call the right version of the function with the
`DIP_OVL_CALL_REAL` macro or any of its relatives defined in `diplib/overload.h`:

    template< typename T >
    void ProcessImage_subfunc( dip::Image img ) {
       dip::ImageIterator< T > it( img );
       do {
          *it *= 2;
       } while( ++it );
    }
    
    void ProcessImage( dip::Image img ) {
       DIP_OVL_CALL_REAL( processImage_subfunc, (img), img.dataType() );
    }

The `DIP_OVL_CALL_REAL` macro will throw an exception if the image is of a type
that does not correspond to the `REAL` group, so there is no need to test for that
separately.

A wholly different way of handling images of different data types is using the
`dip::GenericImageIterator`, which is a simpler, non-templated version of the
`dip::ImageIterator` that cannot be dereferenced, but instead provides a `void`
pointer to the sample. The code inside the loop must be able to handle such a
pointer, so this is only useful in very specific circumstances.

Note that the code above does not handle tensor images, presuming the image is
scalar. A test for that should be included, or alternatively the tensor elements
should be processed. There are two ways of doing so:

 - The simplest is to create a copy of the input image (which would use the same
   data segment), and modify this copy so that the tensor dimension becomes a
   spatial dimension. The loop above would then iterate over each sample, rather
   than each pixel:
   
       dip::Image tmp = img.QuickCopy();
       tmp.TensorToSpatial();
       dip::ImageIterator< dip::uint16 > it( tmp );
       ...
    
 - The second way is to explicitly iterate over the tensor elements within the
   main loop. This allows the different tensor elements to be treated differently.
   Simply note that the iterator `it` points at the first element of the tensor,
   and using the `[]` indexing operator yields the other tensor elements:
   
       dip::ImageIterator< T > it( img );
       do {
          for( dip::uint te = 0; te < img.TensorElements(); ++te ) {
             it[ te ] *= 2;
          }
       } while( ++it );
       
   Alternatively, iterate over the tensor elements using the corresponding
   iterator (see `dip::SampleIterator`):

       dip::ImageIterator< T > it( img );
       do {
          for( auto te = it.begin(); te != it.end(); ++te ) {
             *te *= 2;
          }
       } while( ++it );


Processing an image using a separate output image
---

The `dip::JointImageIterator` loops over both an input and an output image
at the same time. The two images must have the same sizes:

    dip_ThrowIf( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
    dip::Image out( img, dip::DT_SFLOAT );
    dip::JointImageIterator< dip::uint16, dip::sfloat > it( img, out );
    do {
       it.Out() = 2 * it.In();
    } while( ++it );

Note that `it.In()` returns a const reference to the sample, indicating that
it is meant as the input image. The joint image iterator cannot be dereferenced,
as it points at two samples at the same time.

To access the various tensor elements, use the `InElement()` and `OutElement()`
methods.

There is also a `dip::GenericJointImageIterator`, which, just like
`dip::GenericImageIterator`, is a non-templated version of the iterator that
provides a `void` pointer to each pixel.


Processing an image line by line
---

Some processing requires access to a whole image line at the time. Both the
`dip::ImageIterator` and the `dip::JointImageIterator` allow to specify one
dimension over which is not looped. In combination with the `dip::LineIterator`,
one can create functions that process one line at a time:

    dip_ThrowIf( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
    dip::ImageIterator< dip::uint16 > it( img, 0 );
    do {
       auto lit = it.GetLineIterator();
       dip::uint sum = 0;
       do {
          sum += *lit;
       } while( ++lit );
       dip::uint mean = sum / it.Length();
       lit = it.GetLineIterator();
       do {
          *lit = ( *lit * 1000 ) / mean;
       } while( ++lit );
    } while( ++it );

A one-dimensional filter can be implemented using the line iterator as an array:

    dip_ThrowIf( img.DataType() != dip::DT_SFLOAT, "Expecting single-precision float image" );
    dip::Image out( img, dip::DT_SFLOAT );
    constexpr dip::uint N = 2;
    std::array< float, 2*N+1 > filter {{ 1.0/9.0, 2.0/9.0, 3.0/9.0. 2.0/9.0, 1.0/9.0 }};
    dip::JointImageIterator< dip::sfloat, dip::sfloat > it( img, out, 0 );
    do {
       auto iit = it.GetInLineIterator();
       auto oit = it.GetOutLineIterator();
       // At the beginning of the line the filter has only partial support within the image
       for( dip::uint ii = N; ii > 0; --ii, ++oit ) {
          *oit = std::inner_product( filter.begin() + ii, filter.end(), iit, 0 );
       }
       // In the middle of the line the filter has full support
       for( dip::uint ii = N; ii < oit.Length() - N; ++ii, ++iit, ++oit ) {
          *oit = inner_product( filter.begin(), filter.end(), iit, 0 );
       }
       // At the end of the line the filter has only partial support
       for( dip::uint ii = 1; ii <= N; ++ii, ++iit, ++oit ) {
          *oit = inner_product( filter.begin(), filter.end() - ii, iit, 0 );
       }
    } while( ++it );

Note that separable filters use such line by line operations along each dimension
to compose full filters.


Applying an arbitrary neighborhood filter
---

Simpler:

    dip_ThrowIf( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
    dip::Image out( img, dip::DT_UINT16 );
    dip::PixelTable kernel( "elliptic", { 5, 5 } );
    dip::JointImageIterator< dip::uint16, dip::uint16 > it( img, out );
    do {
       dip::uint value = 0;
       for( auto kit = kernel.begin(); kit != kernel.end(); ++kit ) {
          dip::uint16 pix; // If the image is not scalar, we need to provide an array here.
          it.PixelAt( kit.Coordinates(), &pix );
          value += pix;
       }
       it.Out() = value / kit.NumberOfPixels();
    } while( ++it );

We iterate over every pixel in the input and output images. At each pixel we read all pixels
in the kernel, with each read checking for the location to be inside the image domain, and
applying the default boundary condition if not. We write the average pixel value within the
kernel at each output pixel.

Better:

    dip_ThrowIf( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
    dip::Image in = dip::ExtendImage( img, { 2, 2 }, {}, true ); // a copy of the input image with data ouside of its domain
    dip::Image out( img, dip::DT_UINT16 );
    dip::PixelTable kernel( "elliptic", { 5, 5 }, 0 );
    dip::JointImageIterator< dip::uint16, dip::uint16 > it( img, out, 0 );
    do {
       auto iit = it.GetInLineIterator();
       auto oit = it.GetOutLineIterator();
       // Compute the sum across all pixels in the kernels for the first point on the line only
       dip::uint value = 0;
       for( auto kit = kernel.begin(); kit != kernel.end(); ++kit ) {
          value += *( iit.Pointer() + kit.Offset() );
       }
       *oit = value / kit.NumberOfPixels();
       ++oit;
       do {
          // Subtract the pixels that will exit the kernel when it moves
          for( auto kit = kernel.Runs().begin(); kit != kernel.Runs().end(); ++kit ) {
             value -= *( iit.Pointer() + kit->First() );
          }
          ++iit;
          // Add the pixels that entered the kernel when it moved 
          for( auto kit = kernel.Runs().begin(); kit != kernel.Runs().end(); ++kit ) {
             value += *( iit.Pointer() + kit->Last() );
          }
          *oit = value / kit.NumberOfPixels();
       } while( ++oit ); // (the two images are of the same size, the line iterators reach the end at the same time 
    } while( ++it );

We first create a copy of the input image with expanded domain. The image `in` is identical to
`img`, but we can read outside the bounds where data has been filled in. Not having to check for
reads being within the image domain saves a lot of time, more than making up for creating the
copy of the input image.
The outer loop iterates over all lines in the image (in this case, the lines lie along dimension
0). For the first pixel in the line we do the same as before: we compute the sum of values over
the kernel. But for the rest of the pixels in the line, we subtract the values for the first
pixel in each run of the kernel, then move the kernel over, and add the values for the last pixel
in each run. This bookkeeping makes the operation much cheaper. Being able to loop over the lines
of the image, instead of only over each pixel in the image, allows for simple implementation of
many efficient algorithm.

**TODO**
`kernel.Runs()` is a `std::vector< dip::PixelRun >`. So `*kit` is an object of class
`dip::PixelRun`, and `kit->First()` calls `dip::PixelRun::First()`.
`dip::PixelRun` has members `offset`, `coordinates`, `length`, but to compute `Last()` we
also need access to the stride.

`kernel.begin()` returns a `dip::PixelTableIterator`, which iterates over every pixel
represented in the table. It has members `run`, `index`, and `pixelTable&`. Incrementing
the iterator increments `index` until it reaches `length` for the given `run`, then
increments `run`. It should be able to return the `Coordinates()` and the `Offset()`
for the given pixel.

The `dip::PixelTable` constructor takes either a filter shape and size as input, or
a binary image. An optional additional input is the directon along which the runs are
created. By default it makes runs along dimension 0, but maybe we can instead pick the
optimal dimension, which is the one that makes the fewest runs. If the user doesn't
specify a dimension, it is not important which dimension is used anyway, and we might
as well pick a dimension that yields a more efficient iterator.
Its members are `std::vector< dip::PixelRun > runs`, `processingDimension`, `stride`,
and `numberOfPixels`.
