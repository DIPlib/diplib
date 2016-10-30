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
       lit = it.GetLineIterator(); // TODO: cannot reset!?
       do {
          *lit = ( *lit * 1000 ) / mean;
       } while( ++lit );
    } while( ++it );

Note that projections and separable filters use such line by line operations.

Applying an arbitrary neighborhood filter
---

Simpler:

    dip_ThrowIf( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
    dip::Image out( img, dip::DT_UINT16 );
    dip::JointImageIterator< dip::uint16, dip::uint16 > it( img, out );
    do {
       dip::uint value = 0;
       for( dip::sint nit = pixelTable.begin(); nit != pixelTable.end(); ++nit ) {
          // *nit is an offset.
          // TODO: it.InPointer( *nit ) uses boundary condition...
          value += *( it.InPointer() + *nit )
       }
       it.Out() = value / nit.NumberOfPixels();
    } while( ++it );

Better:

    dip_ThrowIf( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
    dip::Image out( img, dip::DT_UINT16 );
    dip::JointImageIterator< dip::uint16, dip::uint16 > it( img, out );
    do {
       auto iit = it.GetInLineIterator();
       auto oit = it.GetOutLineIterator();
       do {
          dip::uint value = 0;
          for( auto nit = pixelTable.Runs().begin(); nit != pixelTable.Runs().end(); ++nit ) {
             nit->start, nit->length;
          }
          *oit = value;
       } while( ++iit, ++oit ); // (the two images are of the same size, these iterators reach the end at the same time 
    } while( ++it );
