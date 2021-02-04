\comment DIPlib 3.0

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


\page using_iterators Using iterators to implement filters

*DIPlib* provides a set of iterators that can be used to efficiently visit all
pixels in an image, independently of the dimensionality of the image.
These iterators can be used to implement filters and other monadic operators.
This page shows how to use these iterators to write different types of image
processing functionality. However, most of the filters in *DIPlib* are not written
using these iterators, but using the functions in the \ref dip::Framework namespace.
Those functions take care of:

 - parallel processing,
 - overloading for different data types,
 - boundary conditions, and
 - allocating the data segment for the output image(s).

Note that using iterators is not necessarily less efficient than using
the framework functions. Depending on the application of the image processing
function you are writing, and on the complexity you are willing to deal with,
you can choose to use either an iterator or a framework function.


\comment --------------------------------------------------------------

\section iterate_one_image Visiting each pixel in a single image

To loop over each pixel in an image and modify its value, you can use the
\ref dip::ImageIterator :

```cpp
DIP_THROW_IF( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
dip::ImageIterator< dip::uint16 > it( img );
do {
   *it *= 2;
} while( ++it );
```

This construct works for an image of any dimensionality and size, but only for an
image of type `dip::uint16`. Instead of throwing an error if the image doesn't
match expectation, you can convert it:

```cpp
img.Convert( dip::DT_UINT16 );
```

To handle images of different data types with the same loop, write the loop in a
templated function, then call the right version of the function with the
\ref DIP_OVL_CALL_REAL macro or any of its relatives defined in \ref "diplib/overload.h":

```cpp
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
```

The \ref DIP_OVL_CALL_REAL macro will throw an exception if the image is of a type
that does not correspond to the `REAL` group, so there is no need to test for that
separately.

A wholly different way of handling images of different data types is using the
\ref dip::GenericImageIterator, which is a simpler, non-templated version of the
\ref dip::ImageIterator that cannot be dereferenced, but instead provides a `void`
pointer to the sample. The code inside the loop must be able to handle such a
pointer, so this is only useful in very specific circumstances.

Note that the code above does not handle tensor images, presuming the image is
scalar. A test for that should be included, or alternatively the tensor elements
should be processed. There are two ways of doing so:

- The simplest is to create a copy of the input image (which would use the same
  data segment), and modify this copy so that the tensor dimension becomes a
  spatial dimension. The loop above would then iterate over each sample, rather
  than each pixel:

        :::cpp
        dip::Image tmp = img.QuickCopy();
        tmp.TensorToSpatial();
        dip::ImageIterator< dip::uint16 > it( tmp );
        ...

- The second way is to explicitly iterate over the tensor elements within the
  main loop. This allows the different tensor elements to be treated differently.
  Simply note that the iterator `it` points at the first element of the tensor,
  and using the `[]` indexing operator yields the other tensor elements:

        :::cpp
        dip::ImageIterator< T > it( img );
        do {
           for( dip::uint te = 0; te < img.TensorElements(); ++te ) {
              it[ te ] *= 2;
           }
        } while( ++it );

    Alternatively, iterate over the tensor elements using the corresponding
    iterator (see \ref dip::SampleIterator):

        :::cpp
        dip::ImageIterator< T > it( img );
        do {
           for( auto te = it.begin(); te != it.end(); ++te ) {
              *te *= 2;
           }
        } while( ++it );



\comment --------------------------------------------------------------

\section iterate_two_images Processing an image using a separate output image

The \ref dip::JointImageIterator loops over multiple images at the same time. The images
must all have the same sizes:

```cpp
DIP_THROW_IF( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
dip::Image out = img.Similar( dip::DT_SFLOAT );
dip::JointImageIterator< dip::uint16, dip::sfloat > it( { img, out } );
do {
   it.Out() = 2 * it.In();
} while( ++it );
```

Note that `it.In()` returns a reference to the sample in the first image, and `it.Out()`
to the same sample in the second image. These are aliases for the generic `it.Sample<N>()`.
The joint image iterator cannot be dereferenced, as it points at multiple samples at the
same time.

To access the various tensor elements, use the `it.InSample(index)`, `it.OutSample(index)`,
or the generic `it.Sample<N>(index)` methods.

There is also a \ref dip::GenericJointImageIterator, which, just like
\ref dip::GenericImageIterator, is a version of the iterator that
provides a `void` pointer to each pixel.


\comment --------------------------------------------------------------

\section iterate_lines Processing an image line by line

Some processing requires access to a whole image line at the time. Both the
\ref dip::ImageIterator and the \ref dip::JointImageIterator allow to specify one
dimension over which is not looped. In combination with the \ref dip::LineIterator,
one can create functions that process one line at a time:

```cpp
DIP_THROW_IF( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
dip::ImageIterator< dip::uint16 > it( img, 0 );
do {
   auto lit = it.GetLineIterator();
   dip::uint sum = 0;
   do {
      sum += *lit;
   } while( ++lit );
   dip::uint mean = sum / lit.Length();
   lit = it.GetLineIterator();
   do {
      dip::uint res = mean == 0 ? 0 : ( *lit * 1000 ) / mean;
      *lit = dip::clamp_cast< dip::uint16 >( res );
   } while( ++lit );
} while( ++it );
```

A one-dimensional filter can be implemented using the line iterator as an array:

```cpp
DIP_THROW_IF( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
dip::Image out = img.Similar( dip::DT_SFLOAT );
constexpr dip::uint N = 2;
std::array< double, 2 * N + 1 > filter{ { 1.0 / 9.0, 2.0 / 9.0, 3.0 / 9.0, 2.0 / 9.0, 1.0 / 9.0 } };
dip::JointImageIterator< dip::uint16, dip::sfloat > it( { img, out }, 0 );
do {
   auto iit = it.GetLineIterator< 0 >();
   auto oit = it.GetLineIterator< 1 >();
   // At the beginning of the line the filter has only partial support within the image
   for( dip::uint ii = N; ii > 0; --ii, ++oit ) {
      *oit = std::inner_product( filter.begin() + ii, filter.end(), iit, 0.0f );
   }
   // In the middle of the line the filter has full support
   for( dip::uint ii = N; ii < oit.Length() - N; ++ii, ++iit, ++oit ) {
      *oit = std::inner_product( filter.begin(), filter.end(), iit, 0.0f );
   }
   // At the end of the line the filter has only partial support
   for( dip::uint ii = 1; ii <= N; ++ii, ++iit, ++oit ) {
      *oit = std::inner_product( filter.begin(), filter.end() - ii, iit, 0.0f );
   }
} while( ++it );
```

Note that separable filters use such line by line operations along each dimension
to compose full filters.


\comment --------------------------------------------------------------

\section iterate_neighborhood Applying an arbitrary neighborhood filter

```cpp
DIP_THROW_IF( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
dip::Image in = dip::ExtendImage( img, { 2, 2 }, {}, { "masked" } ); // a copy of the input image with data ouside of its domain
dip::Image out = in.Similar( dip::DT_UINT16 );
dip::PixelTable kernel( "elliptic", { 5, 5 }, 0 );
dip::PixelTableOffsets offsets = kernel.Prepare( in );
dip::JointImageIterator< dip::uint16, dip::uint16 > it( { in, out }, 0 );
dip::sint inStride = in.Stride( 0 );
do {
   auto iit = it.GetLineIterator< 0 >();
   auto oit = it.GetLineIterator< 1 >();
   // Compute the sum across all pixels in the kernels for the first point on the line only
   dip::uint value = 0;
   for( auto offset : offsets ) {
      value += *( iit.Pointer() + offset );
   }
   *oit = static_cast< dip::uint16 >( value / kernel.NumberOfPixels() );
   ++oit;
   do {
      // Subtract the pixels that will exit the kernel when it moves
      // Add the pixels that will enter the kernel when it moves
      for( auto run : offsets.Runs() ) {
         value -= *( iit.Pointer() + run.offset );
         value += *( iit.Pointer() + run.offset + static_cast< dip::sint >( run.length ) * inStride );
      }
      *oit = static_cast< dip::uint16 >( value / kernel.NumberOfPixels() );
   } while( ++iit, ++oit ); // the two images are of the same size, the line iterators reach the end at the same time
} while( ++it );
```

We first create a copy of the input image with expanded domain. The image `in` is identical to
`img`, but we can read outside the bounds where data have been filled in. Not having to check for
reads being within the image domain saves a lot of time, more than making up for creating the
copy of the input image.

The outer loop iterates over all lines in the image (in this case, the lines lie along dimension
0). For the first pixel in the line we compute the sum of values over
the kernel (note that we directly index by adding the offset to the data pointer, there's no
checking). But for the rest of the pixels in the line, we subtract the values for the first
pixel in each run of the kernel, then move the kernel over, and add the values for the last pixel
in each run. This bookkeeping makes the operation much cheaper. Being able to loop over the lines
of the image, instead of only over each pixel in the image, allows for simple implementation of
many efficient algorithms.


\comment --------------------------------------------------------------

\section iterate_slices Processing an image slice by slice

Take for example the case of a time series image, a 3D image where the 3^rd^ dimension is
time. One might want to filter each of the 2D slices in the same way, but not mix
information from one slice to another. Some image processing functions allow to specify
which dimensions are to be processed, one can choose to process only the first two dimensions.
With other filters one can set the size of the neighborhood to 1 along the 3^rd^
dimension, so that effectively no filtering is applied in that direction. But a few
filters are written explicitly for 2D images, or do not make it possible to restrict
processing dimensions. \ref dip::EuclideanSkeleton is an example of the latter. The
skeleton is always computed across the full image. To apply it to each of the slices
in the time series one can use the \ref "dip::ImageSliceIterator":

```cpp
dip::ImageSliceIterator it( img, 2 );
do {
   dip::EuclideanSkeleton( *it, *it, "natural", true );
} while( ++it );
```

Here, the slice iterator `it` points at a 2D subimage of the 3D image `img`,
in which the 3^rd^ dimension (dimension number 2) of `img` is removed. It is
possible to read and write to the pixels of this slice, but it is not possible
to strip or reforge it.
When constructed, the iterator points at the slice for index 0. Each time it is
incremented, the next slice is indexed. As with the other image iterators,
testing it results in `false` when the iterator points past the last slice and
should not be dereferenced, making the loop logic quite compact.

This iterator allows adding or subtracting any integer, meaning that it is
possible to navigate to any slice. This is a very cheap operation, and requires
no data copies. Note that the loop above is much more efficient than a similar
loop using indexing:

```cpp
// Identical result to the previous code block, but less efficient
dip::RangeArray ra( 3 );
for( dip::sint ii = 0; ii < img.Size( 2 ); ++ii ) {
   ra[ 2 ] = Range( ii );
   dip::Image slice = img.At( ra );
   slice.Squeeze();
   dip::EuclideanSkeleton( slice, slice, "natural", true );
}
```

The difference in efficiency might or might not be important depending on
the cost of the function being applied to each 2D slice.


\comment --------------------------------------------------------------

\section iterate_tensor Processing an image tensor element by tensor element

The \ref dip::ImageSliceIterator can also be used to iterate over the tensor elements
of an image. In this case, dereferencing the iterator yields a scalar image
corresponding to `img[ ii ]`. The two following pieces of code are equivalent:

```cpp
auto it = dip::ImageTensorIterator( img );
do {
   dip::Dilation( *it, *it, 7 );
} while( ++it );

for( dip::uint ii = 0; ii < img.TensorElements(); ++ii ) {
   dip::Dilation( img[ ii ], img[ ii ], 7 );
}
```

As in the previous section, the difference in efficiency (the iterator is more
efficient) might not be important depending on the cost of the function being
called.
