\ingroup infrastructure

[comment]: # (The `ingroup` statement above avoids the generation of an empty page for this file. The actual group we add the file to is irrelevant, nothing is actually added.)

[//]: # (DIPlib 3.0)

[//]: # ([c]2014-2017, Cris Luengo.)
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

\class dip::Image diplib.h

\brief Represents an image with all associated information.

A `%dip::Image` object is the core of the *DIPlib* library, as all functionality
revolves around images. Some image manipulation is provided as class
methods, but most image processing and analysis functionality is provided
in functions defined in the `#dip` namespace.

\tableofcontents

[//]: # (--------------------------------------------------------------)

\section image_representation Image representation

An `%dip::Image` object can have any number of dimensions (limited by the integer
representation used), though 2D and 3D are the most often used dimensionalities.
Most functions in the library accept images with any number of
dimensions, for the functions that are limited in this respect there is
a note in the documentation. A 0D image is an image with a single pixel.

We use the term **pixel** to refer to the collection of samples taken at the
same spatial location, irrespective of the number of dimensions that the
image has (that is, we don't use the term voxel for pixels in 3D images).
A pixel is represented by a **tensor**. We have limited the tensors, in the
current implementation, to have no more than two dimensions (a matrix),
as there doesn't seem to be much use for higher-dimensional tensors in
image analysis. However, there is no limit to the number of **tensor elements**
(other than available memory and the integer representation used).

Each element of the tensor at a pixel is referred to as a **sample**. A tensor
element is synonymous with sample, and we use the one or the other term
in the documentation and code depending on context. We say that an image
with 1 million pixels and 3 samples per pixel has a total of 3 million samples.

If the tensor is 0D (a single sample), the image is a standard gray-value image
(which we refer to as scalar image). A 1D tensor (a vector) can be used to represent
color images (e.g. an RGB image has three samples per pixel), but also for example
the image gradient (see `dip::Gradient`). With a 2D tensor (a matrix) it is possible
to represent concepts such as the Hessian and the structure tensor (see \ref why_tensors).
For example, the Hessian of a 3D image has 9 samples per pixel. For more details
on how tensor elements are stored, see the section on \ref tensors.

A `%dip::Image` object can contain samples of a wide variety of numeric types,
including binary, unsigned and signed integers, floating point, and
complex. For a complete list see the description of the `dip::DataType` class.
All the image's samples must have the same type.

All of these image properties are dynamic. That is, they can be
determined and changed at runtime, they are not fixed at compile time.
An `%Image` object has two states: **raw** and **forged**. When an image is
**raw**, it has no associated data segment. In the raw state, all image
properties can be changed. The `dip::Image::Forge` method allocates the data
segment (the memory block that holds the pixel values). Once the image
is **forged**, its properties are fixed. It is possible to call the
`dip::Image::Strip` method to revert to the raw state. The reason behind this
dynamic image structure is that it allows flexibility: one can read the
data in a file without knowing what the file's data type is going to
be; the file reading function can adjust the `%Image` object's data type
(and dimensionality) at run time to accommodate any data that the file
might contain. Another advantage is that the programmer does not need
to think about, for example, what data type is appropriate as output of
a specific function. However, when desired, it is possible to control
the data types of images.


[//]: # (--------------------------------------------------------------)

\section strides Strides

For maximum flexibility in the relationship between image coordinates
and how the samples are stored in memory, a `%dip::Image` object specifies a
**stride** array (`dip::Image::Strides`). This array indicates, for each
dimension, how many samples to skip to get to the neighboring pixel in the
given dimension.
For example, to go from a pixel at coordinates (`x`,`y`) to the neighbour
at coordinates (`x+1`,`y`), you would need to increment the data pointer
with `strides[0]`. In a 2D image, the pixel at coordinates (`x`,`y`) can be
reached by (assuming `dip::DT_UINT8` data type):

```cpp
    dip::uint8* origin = img.Origin();
    dip::IntegerArray strides = img.Strides();
    dip::uint8 value = *( origin + x * strides[0] + y * strides[1] );
```

This concept naturally scales with image dimensionality. Strides can be
negative, and need not be ordered in any particular way. This allows a
`%dip::Image` object to contain a regular subset of the pixels of another
image, but still point to the same physical memory block. Here are some
examples:

  * An `%Image` object can contain a region of interest (ROI), a smaller
  region within a larger image. In this case, the strides are identical
  to those of the larger image, but the origin and the image size
  differs.

  * An `%Image` object can contain a subsampled image, where the strides
  are a multiple of the strides of the original image.

  * An `%Image` object can contain a single slice of a 3D image, where the
  strides are identical to the 3D image's strides, but the origin and
  the image size and dimensionality are different.

  * An `%Image` object can contain a mirrored image, where the stride in
  the mirrored dimension is negative, and the origin is different.

  * An `%Image` object can contain a rotated image. For example, rotating
  over 90 degrees involves swapping the two dimensions (i.e. swapping the
  sizes and strides associated to these dimensions), and inverting one
  of the dimensions (as in the case of the mirrored image).

Arbitrary strides also allow data segments from other software to be
encapsulated by an `%Image` object. For example, *MATLAB* stores images
with columns contiguous in memory, requiring `strides[1] == 1`.

All routines in the library support images with arbitrary strides.

The various elements of a tensor are also accessed through a stride,
which can be obtained through `dip::Image::TensorStride`. Even for a 2D
tensor, all tensor elements can be visited using a single stride value.
See the section \ref tensors for more information on accessing tensor
elements. And see the section \ref pointers for more information about
accessing samples.


[//]: # (--------------------------------------------------------------)

\section tensors Tensor images

A tensor image (generalization of the vector and matrix image) has a tensor
for each pixel. A tensor collects all samples corresponding to the same spatial
location into a specific shape. `dip::Image::TensorElements` indicates how
many samples per pixel the image has.

A tensor image is stored into a single memory block. In the same way that
strides indicate how to skip from one pixel to another (as described in the
section \ref strides), the `dip::Image::TensorStride` indicates how to skip
from one tensor element to another. This allows e.g. a multi-channel image to
be stored as either interleaved per pixel, per line or per plane, and as long
as an algorithm uses the strides, it does not need to know how the channels
are interleaved.

All tensor elements are stored as if they composed a single spatial dimension.
Therefore, it is possible to change the image such that the tensor elements
form a new spatial dimension (`dip::Image::TensorToSpatial`), or such that one
spatial dimension is converted to a tensor (`dip::Image::SpatialToTensor`),
without moving the samples. It is also possible to change the shape of the
tensor without moving data (`dip::Image::ReshapeTensorAsVector`,
`dip::Image::Transpose`).

The shape of the tensor is represented by the enumerator `dip::Tensor::Shape`
(obtained through `dip::Image::TensorShape`).
The chosen way of storing tensor elements allows us, for example, to store
a symmetric 2D tensor such as the Hessian matrix without repeating the
repeating the duplicated values. We also have a specific shape for diagonal
matrices and triangular matrices.


[//]: # (--------------------------------------------------------------)

\section pointers On pixel coordinates, indices, offsets and data pointers

Given

```cpp
    dip::Image img( { 10, 12, 20, 8, 18 }, 1, dip::DT_UINT16 );
```

Then \link dip::Image::Origin `img.Origin()`\endlink is a `void*` pointer to
the first pixel (or rather the first sample of in the image).
This pointer needs to be cast to the type given by
\link dip::Image::DataType `img.DataType()`\endlink to be used, as in:

```cpp
    (dip::uint16*)img.Origin() = 0;
```

A pixel's **offset** is the number of samples to move away from the origin
to access that pixel:

```cpp
    dip::uint16* ptr = (dip::uint16*)img.Origin();
    ptr + offset = 1;
```

Alternatively, it is possible to compute the pixel's pointer without casting
to the right data type (this leads to a more generic algorithm) by using the
`dip::DataType::SizeOf` operator (we cast to `dip::uint8` pointer to do
pointer arithmetic in bytes):

```cpp
    (dip::uint8*)img.Origin() + offset * img.DataType().SizeOf();
```

This computation is performed by
\link dip::Image::Pointer `img.Pointer( offset )`\endlink.

Note that the offset is a signed integer, and can be negative, because strides
can be negative also.
The offset is computed from coordinates using the image's strides:

```cpp
    dip::UnsignedArray coords { 1, 2, 3, 4, 5 };
    dip::sint offset = 0;
    for( dip::uint ii = 0; ii < img.Dimensionality(); ++ii ) {
      offset += coords[ii] * img.Stride( ii );
    }
```

This computation is performed by
\link dip::Image::Offset `img.Offset( coords )`\endlink.
\link dip::Image::Pointer `img.Pointer( coords )`\endlink simply chains this
operation with the previous one. The inverse operation is performed by
\link dip::Image::OffsetToCoordinates `img.OffsetToCoordinates( offset )`\endlink.
Two images of the same size do not necessarily share offset values.
Both the dimensions and the strides must be identical for the offset to be
compatible between the images.

The coordinates to a pixel simply indicate the number of pixels to skip along
each dimension. The first dimension (dimension 0) is typically `x`, but this
is not evident anywhere in the library, so it is the application using the
library that would make this decision. Coordinates start at 0, and should be
smaller than the `img.Sizes()` value for that dimension. They are encoded
using a `dip::UnsignedArray`. However, some functions take coordinates as
a `dip::IntegerArray`. These are the functions that do not expect the coordinates
to indicate a pixel inside the image domain.

The **index** to a pixel (a.k.a. "linear index") is a value that increases
monotonically as one moves from one pixel to the next, first along dimension 0,
then along dimension 1, etc. The index computed from a pixel's coordinates is
as follows:

```cpp
    dip::UnsignedArray coords { 1, 2, 3, 4, 5 };
    dip::uint dd = img.Dimensionality();
    dip::uint index = 0;
    while( dd > 0 ) {
      --dd;
      index *= img.Size( dd );
      index += coords[dd];
    }
```

This computation is performed by \link dip::Image::Index `img.Index( coords )`\endlink.
It is the *n*D equivalent to `x + y * width`. An index, as opposed to an
offset, is always non-negative, and therefore stored in an unsigned integer. The
index is shared among any images with the same dimensions.

It is not efficient to use indices to access many pixels, as the relationship
between the index and the offset is non-trivial. One can determine the
coordinates corresponding to an index through
\link dip::Image::IndexToCoordinates `img.IndexToCoordinates( index )`\endlink,
which then leads to an offset or a pointer.
The function `dip::Image::At` with a scalar argument uses linear indices, and
consequently is not efficient for images with dimensionality of 2 or more.

Oftentimes it is possible to determine a **simple stride** that will allow you to
access every pixel in an image. When an image is a view into another image,
this is not necessarily possible, but any default image (i.e. with normal strides)
has this possibility. This simple stride allows one to view the image as a
1D image. The function `dip::Image::Flatten` will create this 1D image (without
copying any data, if there exists such a simple stride). Walking along this
one dimension will, however, not necessarily access the pixels in the same order
as given by the linear index. This order is only consistent if the image has
normal strides. See `dip::Image::HasNormalStrides`,
`dip::Image::HasSimpleStride`, `dip::Image::GetSimpleStrideAndOrigin`.

To walk along all pixels in an arbitrary image (i.e. arbitrary dimensionality
and strides) in the order given by the linear index, use the **image iterators**
defined in `diplib/iterators.h`
(see \ref using_iterators "Using iterators to implement filters"):

```cpp
    dip::ImageIterator< dip::uint16 > it( img );
    dip::uint16 ii = 0;
    do {
      *it = ii++;
    } while( ++it );
```

The functionality in the `dip::Framework` namespace is the recommended way of
building generic functions that access all pixels in an image. These functions
allow you to loop over multiple images simultaneously, using multi-threading,
while taking care of different data types, checking input images, allocating
output images, etc. Different framework functions do pixel-based processing,
line-based processing (e.g. separable filters, projections) and
neighborhood-based processing (i.e. non-separable filters). There is currently
no plans for framework functionality to support priority queue algorithms,
if you figure out how to make such a function generic enough please contribute!

For images with more than one sample per pixel, the above discussion shows
only how to access the first sample in each pixel. Coordinates always indicate
a pixel, and therefore `dip::Image::Offset` always gives the offset to the
first sample of a pixel. The other samples can be accessed by adding
`n * img.TensorStride()` to the offset, where `n` is the tensor element. Tensor
elements are then accessed in a specific order, depending on the shape of the
tensor. See `dip::Tensor::Shape` for a description of the order of the tensor
elements in memory. For the iterators, use `it[n]` to access the `n`th tensor
element.


[//]: # (--------------------------------------------------------------)

\section assignment Creation, assignment and copy

To create a new image with specific properties, one can either set each of
the properties individually, or use one of the constructors. For example,
the two following images are the same:

```cpp
    dip::Image img1;
    img1.SetSizes( { 256, 256 } );
    img1.SetDataType( dip::DT_UINT8 );
    img1.SetTensorSizes( 1 );
    img1.Forge();

    dip::Image img2( UnsingedArray{ 256, 256 }, 1, dip::DT_UINT8 );
```

The first method is more flexible, as it allows to set all properties
before forging the image (such as strides).
Note that the created image has uninitialized pixel data. You can use the
`dip::Image::Fill` method to set all pixels to a specific value.

To create a new image with same sizes and tensor shape as another one,
use the `dip::Image::Similar` method:

```cpp
    img2 = img1.Similar();
```

Again, the new image will have uninitialized pixel data. An optional second
argument can be used to specify the data type of the new image:

```cpp
    img2 = img1.Similar( dip::DT_SCOMPLEX );
```

Both methods copy all image properties, including the strides array and the
external interface; see `dip::Image::CopyProperties`.

A similar method is `dip::Image::ReForge`, which modifies the properties of
an image and creates a new data segment if the old one is of the wrong size
to support the new properties. In function, these three sets of statements
are equivalent:

```cpp
    img2 = img1.Similar();

    img2.Strip();
    img2.CopyProperties( img1 );
    img2.Forge();

    img2.ReForge( img1 );
```

However, `ReForge` might not strip and forge if it is not necessary
(also, it does not use the source image's strides), and so
is the recommended way of modifying an image to match another one.
`dip::Image::ReForge` has two other forms that can be useful, see the
documentation.

Lastly, it is possible to create a 0D image (an image with a single pixel)
with the constructor that takes a scalar value (integer, float or complex),
or an initializer list containing scalar values of the same type. With the
initializer list, the image will be a vector image with as many samples
as elements in the initializer list:

```cpp
    dip::Image img1( 10 );
    dip::Image img2( { 0, 1, 2, 3 } );
```

The assignment operator creates a copy of the image, but does not actually
copy the data. Instead, the new copy will share the data segment with the
original image:

```cpp
    img2 = img1;
```

Both `img1` and `img2` point at the same data, meaning that changing one
image's pixel values also affects the other image. The data segment will
exist as long as one image references it. That is, if `img1` goes out
of scope, `img2` will still point at a valid data segment, which will not
be freed until `img2` goes out of scope (or is stripped). This is useful
behavior, but can cause unexpected results at times. See \ref aliasing
for how to write image filters that are robust against images with shared
data. However, if the image assigned into has an external interface set, a
data copy might be triggered, see \ref external_interface.

`dip::Image::QuickCopy` can be used here if the image copy does need any of
the image metadata (color space and pixel size). The overhead of copying
the metadata information is small, but we often use this function
internally when the input image to a function needs to be reshaped for
processing, but we do not want to modify the input image itself:

```cpp
    dip::Image tmp = img1.QuickCopy();
    tmp.Squeeze();
    ...
```

The copy constructor behaves the same way as the assignment operator, making
the new image share data with the input image. The following three statements
all invoke the copy constructor:

```cpp
    img2 = dip::Image( img1 );
    dip::Image img3( img1 );
    dip::Image img4 = img1;
```

To make a copy of an image with its own copy of the data segment, use the
`dip::Image::Copy` method:

```cpp
    img2.Copy( img1 );
```

or equivalently the `dip::Copy` function:

```cpp
    img2 = dip::Copy( img1 );
```

In both cases, `img2` will be identical to `img1`, with identical pixel values,
and with its own data segment.

When the `dip::Image::Copy` method is used on a forged image, it is expected to
be of the same size as the image to be copied. Pixel values will be copied to
the existing data segment, casting to the target image's data type with clamping
(see `diplib/library/clamp_cast.h`):

```cpp
    img2 = img1.Similar( dip::DT_UINT8 );
    img2.Copy( img1 );
```

Or equivalently:

```cpp
    img2 = dip::Convert( img1, dip::DT_UINT8 );
```

The `dip::Image::Convert` method, as opposed to the `dip::Convert` function, converts
the image itself to a new data type. This process creates a new data segment if
the data type is of different size, and works in place if the data type is of the
same size (e.g. `dip::DT_SINT32` and `dip::DT_SFLOAT` have the same size, as do
`dip::DT_DFLOAT` and `dip::DT_SCOMPLEX`). However, if the data segment is shared,
it will never work in place, as that could cause important problems.

Assigning a constant to an image is equivalent to calling the `dip::Image::Fill`
method, writing that constant to each sample in the image. The constant is
cast to the image's data type with saturation (see `dip::clamp_cast`). Note that
the image must be forged:

```cpp
    img1 = 10;
    img2.Fill( 0 );
```

Additionally, one can assign an initializer list to an image. The list should
have one element for each tensor element. Each pixel in the image will then
be set to the tensor values in the initializer list:

```cpp
    img2 = dip::Image( UnsignedArray{ 256, 256 }, 10, dip::DT_SFLOAT );
    img2 = { 1, 2, 3, 4 };
```

`img2` will have all pixels set to the same vector `[ 1, 2, 3, 4 ]`.


[//]: # (--------------------------------------------------------------)

\section indexing Indexing

There are three main modes of indexing pixels in a `%dip::Image` object: single
pixel indexing, regular grid indexing, and irregular pixel indexing. All three
modes have similar, consistent syntax, but differ by the return type and properties.
Additionally, one can index into the tensor dimension. The next four sub-sections
describe these different indexing modes.

All indexing modes share in common that the returned object can be assigned to,
modifying the original image. When indexing a single pixel, the return type is
a `dip::Image::Pixel`, which references a single pixel in the original image.
When indexing multiple pixels (either by indexing the tensor dimension or by
regular or irregular indexing), the return type is a `dip::Image::View`, which
references multiple pixels in the original image. Both these objects have
some overloaded methods and can be iterated over. Additionally, the `dip::Image::View`
object implicitly casts back to a `%dip::Image`, so it can be used as arguments
to functions that take an `%Image`. Here is where the difference between regular
and irregular indexing comes into play: the `%Image` that results from regular
indexing (and from tensor indexing) shares the data with the orignal image, whereas
when the indexing was irregular, the data needs to be copied to create a new image
with only those pixels.


\subsection tensor_indexing Tensor dimensions

To address one channel in a color image is a rather common operation, and therefore
we have delegated the C++ indexing operator (`[]`) to the task of extracting a
tensor component from an image (remember that a color channel is a tensor component).
For example:

```cpp
    dip::Image red = colorIm[ 0 ];
```

For a two-dimensional matrix it is possible to index using two values in an array:

```cpp
    dip::Image out = tensorIm[ dip::UnsignedArray{ i, j } ];
```

The indexing operation, as explained above, returns a `dip::Image::View` object,
which can be assigned to to modify the referenced pixels:

```cpp
    colorIm[ 0 ] = colorIm[ 1 ];
```

When cast tack to an image, the image created shares the pixels with the original image,
meaning that it is possible to write to a channel in this way:

```cpp
    dip::Gauss( colorIm[ 2 ], colorIm[ 2 ], { 4 } );
```

Note that the single-index version of the tensor indexing uses linear indexing into
the tensor element list: if the tensor is, for example, a 2x2 symmetric matrix, only
three elements are actually stored, with indices 0 and 1 representing the two diagonal
elements, and index 2 representing the two identical off-diagonal elements. See
`dip::Tensor` for information on how the tensor is stored.

To extract multiple tensor elements one can use a `dip::Range` to index. Other useful
methods here are `dip::Image::Diagonal`, `dip::Image::TensorRow` and
`dip::Image::TensorColumn`, which all yield a vector image.

The methods `dip::Image::Real` and `dip::Image::Imaginary` are somewhat related to these
last three methods in that they return a view over a subset of the data, except that
they reference only the real or imaginary component of the complex sample values.


\subsection pixel_indexing Single-pixel indexing

The function `dip::Image::At` extracts a single pixel from the image. It has different
forms, accepting either a `dip::UnsignedArray` representing the coordinates of the pixel,
one to three indices for 1D to 3D images, or a (linear) index to the pixel. The latter
form is less efficient because the linear index needs to be translated to coordinates,
as the linear index is not necessarily related to the order in which pixels are stored
in memory.

```cpp
    image1D.At( 5 );          // indexes pixel at coordinate 5
    image2D.At( 0, 10 );      // indexes the pixel at coordinates (0, 10)
    image2D.At( { 0, 10 } );  // indexes the pixel at coordinates (0, 10)
    image2D.At( 20 );         // indexes the pixel with linear index 20
```

These forms result in an object of type `dip::Image::Pixel`. The object contains a
reference to the original image pixels, so writing to the reference changes the pixel
values in the image:

```cpp
    image.At( 0, 10 ) += 1;
```

The single-pixel forms of `dip::Image::At` have an alternative, templated form. In that
form, they return a `dip::Image::CastPixel< T >` instead, which is identical to a
`dip::Image::Pixel`, but implicitly casts to type `T`. Thus:

```cpp
    int v1 = image.At( 0, 10 );             // Does not compile, no implicit conversion to `int`.
    int v2 = image.At< int >( 0, 10 );      // OK
    int v3 = image.At( 0, 10 ).As< int >(); // Same as `v2`.
```

This implicit cast makes its use a little simpler in a setting combined with numbers
of that type. However, the object itself is not tied to the type, and it is still possible
to access (read or write) the pixel as other types. For example, this code will print
"( 1.0, -3.0 )" to the standard output, the template argument to `%At` has no influence
on the results:

```cpp
    dip::Image image( { 256, 256 }, 1, dip::DT_SCOMPLEX );
    image.At< dip::uint8 >( 85, 43 ) = dip::dcomplex{ 1.0, -3.0 );
    std::cout << image.At< dip::sint32 >( 85, 43 );
```

Tensor and spatial indexing can be combined in either order, but the results are
not identical:

```cpp
    image.At( 0, 10 )[ 1 ];
    image[ 1 ].At( 0, 10 );
```

The first line above uses `[]` to index into the `dip::Image::Pixel` object, yielding
a `dip::Image::Sample`. Again, this is a reference to the sample in the image, and can
be written to to change the image. The second line first uses `[]` to create a scalar
image view, and then extracts a pixel from it. The result is a `dip::Image::Pixel` object,
not a `dip::Image::Sample`, though the pixel object references a single sample in the
input image. In practice, these are equally useful, though the first form is slightly
more efficient because the intermediate object generated has less overhead.

See the documentation to `dip::Image::Sample` and `dip::Image::Pixel` for more information
on how these objects can be used. Note however, that this is not an efficient way of
accessing pixels in an image. It is much more efficient to use pointers into the
data segment. However, pointers require knowledge of the data type at compile time.
The indexing described here is a convenient way to read a particular value in a
data-type agnostic way. To access all pixels in a data-type agnostic way, use the
`dip::GenericImageIterator`.


\subsection regular_indexing Regular indexing (windows, ROI processing, subsampling)

The function `dip::Image::At` is also used for creating views that represent a subset of
pixels. In this form, it accepts a set of `dip::Range` objects, or a `dip::RangeArray`,
representing regular pixel intervals along one dimension through a start, stop and
step value. That is, a range indicates a portion of an image line, with optional
subsampling. For example, indexing into a 1D image:

```cpp
    image1D.At( dip::Range{ 5 } );          // indexes pixel at coordinate 5
    image1D.At( dip::Range{ 0, 10 } );      // indexes the first 11 pixels
    image1D.At( dip::Range{ 0, -1, 2 } );   // indexes every second pixel
```

Note that negative values index from the end, without needing to know the exact
size of the image.

For indexing into multidimensional images, simply provide one range per dimension.
For more than 3 dimensions, provide a `dip::RangeArray`.

As is the case with the `[]` indexing, these operations yield a `dip::Image::View` that
can be assigned to to change the referenced pixels; and when cast back to a `dip::Image`,
yields an image that shares data with the original image. For example:

```cpp
    image.At( dip::Range{ 0, 10 }, dip::Range{ 20, 30 } ) += 1;
```

Tensor and spatial indexing can be combined in either order, with identical
results:

```cpp
    image.At( { 0, 10 }, { 20, 30 } )[ 1 ];
    image[ 1 ].At( { 0, 10 }, { 20, 30 } );
```

The method `dip::Image::Cropped` is a convenience function to extract a view to a
rectangular widow of a given size and with a given anchor (image center or corner).
The `dip::Image::Pad` method does the opposite operation, but creates a new image and
copies the data over.


\subsection irregular_indexing Irregular indexing

An arbitrary subset of pixels can be indexed in three ways, using one of:
 * a mask image
 * a list of pixel coordinates
 * a list of linear indices into the image

The first two forms are again available through the `dip::Image::At` method, the
third one through the `dip::Image::AtIndices` method (since the input argument would
not be distinguiseable from indexing a single pixel through its coordinates). As with
regular indexing, the returned object is a `dip::Image::View`, which can be assigned
to to change the referenced pixels. But as opposed to regular indexing, when cast
back to a `%dip::Image`, the operation results in a 1D image with a copy of the sample
values. Thus, once cast back to an `%Image`, pixels are no longer shared and can
be modified without affecting the original image. This is by necessity, as the `%Image`
object cannot reference samples that are not stored in memory on a regular grid.

```cpp
    image.At( mask ) += 1;
    dip::Image out = image.At( mask );
    out.Fill( 0 );                       // does not affect `image`.
```

Another typical example:

```cpp
    image.At( mask ) = otherImage.At( mask );
```


[//]: # (--------------------------------------------------------------)

\section reshaping Reshaping

There is a large collection of methods to reshape the image without physically
moving samples around in memory. These functions accomplish their goal simply
by modifying the sizes and strides arrays, and the tensor representation
values. All of these functions modify the object directly, and return a reference
so that they can be chained.

For example, `dip::Image::Rotation90` rotates the image in 90 degree increments
by mirroring and swapping dimensions. More generic methods are `dip::Image::Mirror`,
and `dip::Image::SwapDimensions`. `dip::Image::PermuteDimensions` reorders the
image's dimensions, an optionally adds or removes singleton dimensions. Singleton
dimensions are dimensions with a size of 1, and can be added or removed without
affecting the data layout in memory.

Several of these methods are meant to manipulate singleton dimensions.
`dip::Image::AddSingleton` and `dip::Image::Squeeze` add and remove singleton
dimensions. `dip::Image::ExpandDimensionality` adds singleton dimensions at the
end to increase the number of image dimensions. It is also possible to expand
singleton dimensions so they no longer are singleton, by replicating the data
along that dimensions, again without physically replicating or modifying the
data in memory. See \ref singleton_expansion. The methods
`dip::Image::ExpandSingletonDimension`, `dip::Image::ExpandSingletonDimensions`
and `dip::Image::ExpandSingletonTensor` are used for this.
`dip::Image::UnexpandSingletonDimensions` does the oposite operation.

The method `dip::Image::StandardizeStrides` undoes all rotation, mirroring,
dimension reordering, and singleton expansion, by making all strides positive
and sorting them from smallest to largest.
The `dip::Image::Flatten` method converts the image to 1D (though if the image
does not have contiguous data, it will have to be copied to form a 1D image).

A group of methods manipulate the image's tensor shape:
`dip::Image::ReshapeTensor` requires the target tensor shape to have as many
tensor elements as the image already has. For example, a 3-vector image can be
converted into a 2x2 symmetric tensor image. `dip::Image::ReshapeTensorAsVector` and
`dip::Image::ReshapeTensorAsDiagonal` turn the image into the given tensor shapes.
The `dip::Image::Transpose` method belongs in this set, though it has a mathematical
meaning. `%Transpose` changes the row-major matrix into a column-major matrix and
vice-versa, thereby transposing the tensor.

It is also possible to turn the tensor dimension into a spatial dimension and
back, using `dip::Image::TensorToSpatial` and `dip::Image::SpatialToTensor`.
Turning the tensor dimension into a spatial dimension can be useful for example
to compute the maximum tensor element per pixel. The complex method is:

```cpp
    out = dip::Supremum( { image[ 0 ], image[ 1 ], image[ 2 ] } );
```

Though short to type, it is a complex method if the number of tensor elements is
not known at compile time. In this case, a loop needs to create the
`dip::ImageConstRefArray` that is the input to `dip::Supremum`.
The easier alternative is:

```cpp
    dip::Image tmp = image.QuickCopy().TensorToSpatial( 0 );
    dip::BooleanArray process( tmp.Dimensionality(), false );
    process[ 0 ] = true;
    out = dip::Maximum( tmp, {}, process ).Squeeze( 0 );
```

The first line creates a temporary copy of the input image, which is modified
such that the tensor elements are along the first spatial dimension. The next
two lines create a *process* array, which specifies that only the first
dimension should be processed. Next, the maximum projection along the first
dimension is computed, and the resulting singleton dimension is removed. The
result is then a scalar image of the same sizes as `image`.

Similar in purpose and function to `dip::Image::TensorToSpatial` are functions
that split the complex-valued samples into two floating-point--valued samples,
and present these as either a new spatial dimension, or the tensor dimension:
`dip::Image::SplitComplex`, and `dip::Image::SplitComplexToTensor`. The inverse
operation is accomplished with `dip::Image::MergeComplex` and
`dip::Image::MergeTensorToComplex`.


[//]: # (--------------------------------------------------------------)

\section protect The "protect" flag

An image carries a "protect" flag. When set, the `dip::Image::Strip` function
throws an exception. That is, when the flag is set, the data segment cannot
be stripped (freed) or reforged (reallocated). It does not, however, protect
the image from being assigned into. For example:

```cpp
    dip::Image img1( UnsignedArray{ 256, 256 }, 3, dip::DT_SFLOAT );
    img1.Protect();
    //img1.Strip();  // Throws!
    img1 = img2;     // OK
```

The main purpose of the protect flag is to provide a simple means of specifying
the data type for the output image or a filter. Most filters and operations in
*DIPlib* choose a specific data type for their output based on the input data
type, and in such a way that little precision is lost. For example, the Gaussian
filter will produce a single-precision floating point output image by default
when the input image is an 8-bit unsigned integer. Under the assumption that
this default choice is suitable most of the time, we have chosen to not give
all these functions an additional input argument to specify the data type for
the output image. Instead, if the output image has the protect flag set, these
functions will not modify its data type. Thus, you can set the output image's
data type, protect it, and receive the result of the filter in that data type:

```cpp
    dip::Image img = ...
    dip::Image out;
    out.SetDataType( dip::DT_SINT16 );
    out.Protect();
    dip::Gauss( img, out, { 4 } );
    // out is forged with correct sizes to receive filter result, and as 16-bit integer.
```

This is especially simple for in-place operations where we want to receive the
output in the same data segment as the input:

```cpp
    dip::Image img = ...
    img.Protect();
    dip::Gauss( img, img, { 4 } );
```

If the filter is called with a protected, forged image as output, as is the case
in the last code snipped above, the filter will not be able to strip and re-forge
the image, meaning that the image must have the correct sizes and tensor elements
to receive the output. However, if the image is not forged, as in the first
code snippet, then the filter can set its sizes and forge it.


[//]: # (--------------------------------------------------------------)

\section const_correctness Const correctness

When an image object is marked `const`, the compiler will prevent modifications
to it, it cannot be assigned to, and it cannot be used as the output argument
to a filter function. However, the \ref indexing "indexing operators", the copy
assignment operator, and `dip::Image::QuickCopy` all allow the user to make
a non-const object that points to the same data, making it possible to
modify the pixel values of a const image (see \ref design_const_correctness
for our reasons to allow this). Because of that, it did not really make sense
either to have `dip::Image::Data`, `dip::Image::Origin`, and `dip::Image::Pointer`
return const pointers when applied to a const image.

Thus, there is nothing prevent you from modifying the pixel values of a const image.
However, none of the functions in *DIPlib* will do so. A const image (usually
the input images to functions are marked const) will not be modified.


[//]: # (--------------------------------------------------------------)

\section arithmetic Arithmetic and comparison operators

Arithmetic operations, logical operations, and comparisons
can all be performed using operators, but there are also functions defined
that perform the same function with more flexibility. See \ref math_arithmetic
and \ref math_comparison for a full list of these functions.

For example, to add two images `a` and `b`, one can simply do:

```cpp
    dip::Image c = a + b;
```

But it is also possible to control the output image by using the `dip::Add`
function:

```cpp
    dip::Image c;
    dip::Add( a, b, c, dip::DT_SINT32 );
```

The fourth argument specifies the data type of the output image. The computation
is performed in that data type, meaning that both inputs are first cast to that
data type (with clamping). The operation is then performed with saturation. This
means that adding -5 and 10 in an unsigned integer format will not yield 5, but 10,
because the -5 is first cast to unsigned, becoming 0. Also, adding 200 and 200 in an 8-bit
unsigned integer format will yield 255, there is no dropping of the higher-order
bit as in standard C++ arithmetic.

As is true for most image processing functions in *DIPlib* (see \ref design_function_signatures),
the statement above is identical to

```cpp
    dip::Image c = dip::Add( a, b, dip::DT_SINT32 );
```

However, the former version allows for writing to already-allocated memory space
in image `c`, or to an image with an external interface (see \ref external_interface).

For in-place addition, use

```cpp
    dip::Add( a, b, a, a.DataType() );
```

or simply

    a += b;

All dyadic operations (arithmetic, logical, comparison) perform \ref singleton_expansion.
They also correctly handle tensor images of any shape. For example, it is possible
to add a vector image and a tensor image, but it is not possible to add two vector
images of different lengths. The multiplication operation always performs matrix
multiplication on the tensors, use the `dip::MultiplySampleWise` function to do
sample-wise multiplication. Other operators are sample-wise by definition (including
the division, which is not really defined for tensors).


[//]: # (--------------------------------------------------------------)

\section color Color images

A color image is a vector image where each vector element represents a color
channel. `dip::Image::SetColorSpace` assigns a string into the image object that
flags it as a color image. The string identifies the color space. An empty string
indicates that the image is not a color image.

There are no checks made when
marking the image as a color image. For example, and RGB image is expected to
have three channels (`img.TensorElements() == 3`).
However, the call `img.SetColorSpace("RGB")` will mark `img` as an RGB image
no matter how many tensor elements it has (this is because the function has
no knowledge of color spaces). If the image has other than three
tensor elements, errors will occur when the color space information is used,
for example when trying to convert the image to a different color space.

Nonetheless, when manipulating an image in such a way that the number of tensor
elements changes, the color space information in the image is reset, turning it
into a non-color image.

An object of type `dip::ColorSpaceManager` is used to convert an image from one
known color space to another. This object is the only place in the library where
there is knowledge about color spaces. Create one of these objects for any
application that requires color space knowledge. It is possible to register new
color spaces and color space conversion functions with this object. Other functions
that use specific color spaces will have knowledge only of those specific color
spaces, and will expect their input to be in one of those color spaces.


[//]: # (--------------------------------------------------------------)

\section pixel_size Pixel size

Each image carries with it the size of its pixels as a series of physical
quantities (`dip::PhysicalQuantity`), one for each image dimension. The advantage
of keeping the pixel size with the image rather than as a separate value taken
manually from the image file metadata is that it is automatically updated through
manipulations such as scaling (zoom), dimension permutations, etc. When the pixel
size in a particular dimension is not set, it is always presumed to be of size 1
(a dimensionless unit). See `dip::PixelSize` for details.

There are three ways in which the pixel size can be used:

1. The measurement function will return its measurements as physical quantities,
   using the pixel sizes, if known, to derive those from measurements in pixels.

2. The `dip::Image::PhysicalToPixels` method converts a filter size in physical
   units to one in pixels, suitable to pass to a filtering function. For example,
   to apply a filter with a sigma of 1 micron to an image:
   ```cpp
       dip::PhysicalQuantityArray fsz_phys{ 1 * dip::PhysicalQuantity::Micrometer() };
       dip::Filter( img, img, img.PhysicalToPixels( fsz_phys ));
   ```

3. The `dip::Image::PixelsToPhysical` method converts coordinates in pixels to
   coordinates in physical units. For example, to determine the position in
   physical units of a pixel w.r.t. the top left pixel:
   ```cpp
       dip::FloatArray pos_pix{ 40, 24 };
       dip::PhysicalQuantityArray pos_phys = img.PixelsToPhysical( pos_pix );
   ```

It is currently possible to add, subtract, multiply and divide two physical quantities,
and elevate a physical quantity to an integer power. Other operations should be
added as necessary.


[//]: # (--------------------------------------------------------------)

\section external_data_segment Controlling data segment allocation

It is possible to create `%dip::Image` objects whose pixels are not allocated
by *DIPlib* using two different methods:

1. Create an image around an existing data segment. This is used in the case
when passing data from other imaging libraries, or from interpreted languages
that have their own array type. Just about any data can be encapsulated by a
a `%dip::Image` without copy.

2. Define an image's allocator, so that when *DIPlib* forges the image, the
user's allocator is called instead of `std::malloc`. This is used in the
case when the result of *DIPlib* functions will be passed to another
imaging library, or to an interpreted language.

In both cases, `dip::Image::IsExternalData` returns true.

Note that when the image needs to be reforged (sizes and/or data type do not
match what is required for output by some function), but the data segment is
of the correct size, `dip::Image::ReForge` would typically re-use the data
segment if it is not shared with another image. However, when the data segment
is external, it is always reforged.

\subsection use_external_data Create an image around existing data

One of the `%dip::Image` constructors takes a pointer to the first pixel of a
data segment (i.e. pixel buffer), and image sizes and strides, and creates a new
image that references that data. The only requirement is that all pixels are
aligned on boundaries given by the size of the sample data type. That is,
*DIPlib* strides are not in bytes but in samples. The resulting image is identical
to any other image in all respects, except the behaviour of `dip::Image::ReForge`,
as mentioned in the previous paragraph.

For exmple, in this bit of code we take the data of a `std::vector` and build an
image around it, which we can use as both an input or an output image:

```cpp
    std::vector<unsigned char> src( 256 * 256, 0 ); // existing data
    dip::Image img(
       NonOwnedRefToDataSegment( src.data() ),
       src.data(),    // origin
       dip::DT_UINT8, // dataType
       { 256, 256 },  // sizes
       { 1, 256 }     // strides
    );
    img.Protect();          // Prevent `dip::Gauss` from reallocating its output image
    dip::Gauss( img, img ); // Apply Gaussian filter, output is written to input array
```

The first argument to this constructor is a `dip::DataSegment` object, which is just
a shared pointer to void. If you want the `%dip::Image` object to own the resources,
pass create a shared pointer with an appropriate deleter function. Otherwise, use the
function `dip::NonOwnedRefToDataSegment` to create a shared pointer wihtout a deleter
function (as in the example above), indicating that ownership is not to be transferred.
In the example below, we do the same as above, but we transfer ownership of the
`std::vector` to the image. When the image goes out of scope (or is reallocated) the
`std::vector` is deleted:

```cpp
    auto src = new std::vector<unsigned char>( 256 * 256, 0 ); // existing data
    dip::Image img(
       DataSegment{ src }, // transfer ownership of the data
       src->data(),   // origin
       dip::DT_UINT8, // dataType
       { 256, 256 },  // sizes
       { 1, 256 }     // strides
    );
    dip::Gauss( img, img ); // Apply Gaussian filter, output is written to a different data segment
```

After the call to `dip::Gauss`, `*src` no longer exists. `dip::Gauss` has reforged
`img` to be of type `dip::DT_SFLOAT`, triggering the deletion of object pointed to
by `src`.

\subsection external_interface Define an image's allocator

The the previous sub-section we saw how to encapsulate external data. The
resulting image object can be used as input and output, but most *DIPlib*
functions will reforge their output images to be of approriate size and data type.
Unless the allocated image is of suitable size and data type, and the image is
protected, a new data segment will be allocated. This means that the output of the
function call will not be written into the buffer we had provided.

Instead we can define an allocator class, derived from `dip::ExternalInterface`,
and assign a pointer to an object of the allocator class to an image using
`dip::Image::SetExternalInterface`. When that image is forged or reforged,
the `dip::ExternalInterface::AllocateData` method is called. This method is
free to allocate whatever objects it needs, and sets the image's origin pointer
and strides (much like in the previous section). As before, it is possible to
set the `dip::DataSegment` also, so that ownership of the allocated objects is
transferred to the image object. As can be seen in the *DIPlib--MATLAB* interface
(see `dml::MatlabInterface` in `include/dip_matlab_interface.h`), the
`dip::DataSegment` can contain a custom deleter function that again calls the
external interface object to take care of proper object cleaning.

The external interface remains owned by the calling code, and can be linked to
many images.

Note that an image with an external interface behaves differently when assigned
to: usual assignment causes the source and destination images to share the data
segment (i.e. no copy of samples is made); if the destination has an external
interface that is different from the source's, the samples are copied.
Additionally, these images behave differently when reforged,
\ref external_data_segment "as mentioned above".

The example below is a simple external interface that allocates data using a
`std::vector`. The `AllocateData` method returns without setting the `origin`,
indicating to the calling `dip::Image::Forge` function that it cannot allocate
such an image. `%Forge` will then allocate the data itself. If you prefer the
forging to fail, simply throw an exception.

```cpp
    class VectorInterface : public dip::ExternalInterface {
       public:
          virtual dip::DataSegment AllocateData(
                void*& origin,
                dip::DataType datatype,
                dip::UnsignedArray const& sizes,
                dip::IntegerArray& strides,
                dip::Tensor const& tensor,
                dip::sint& tstride
          ) override {
             if(( sizes.size() != 2 ) || ( !tensor.IsScalar() )) {
                return nullptr; // We do not want to handle such images
             }
             auto data = new std::vector<unsigned char>( sizes[ 0 ] * sizes[ 1 ], 0 );
             origin = data.data();
             strides = dip::UnsignedArray{ 1, sizes[ 0 ] };
             tstride = 1;
             return dip::DataSegment{ data };
          }
    }
```

See `dml::MatlabInterface` for a more realistic example of this feature.
That class defines an additional method that can be used to extract the
allocated object that owns the data segment, so that it can be used after
the `%dip::Image` object is destroyed. A simple explanation for how that
works is as follows: The custom deleter function in the `dip::DataSegment`
object only deletes the object pointed to if that object is in a list
kept by the `dml::MatlabInterface` object. If one wants to keep the data
segment, it is removed from the list, so that when the custom deleter
function runs, it does nothing.


[//]: # (--------------------------------------------------------------)

\section singleton_expansion Singleton expansion

When two images need to be of the same size, a process we refer to as
singleton expansion is performed. First, singleton dimensions (dimensions
with a size of 1) are added to the image with the fewer dimensions. This
process does not require a data copy. Next, all singleton dimensions
are expanded to match the size of the corresponding dimension in the other
image. This expansion simply repeats the value all along that dimension,
and is accomplished by setting the image's size in that dimension to
the expected value, and the corresponding stride to 0 (again, no data is
physically copied). This will cause an algorithm to read the same value,
no matter how many steps it takes along this dimension. For example:

```cpp
    dip::Image img1( UnsignedArray{ 50, 1, 60 } );
    dip::Image img2( UnsignedArray{ 50, 30 } );
    dip::Image img3 = img1 + img2;
```

Here, the dimension array for `img2` will be extended to `{ 50, 30, 1 }`
in the first step. In the second step, the arrays for both images will
be changed to `{ 50, 30, 60 }`. `img1` gets its second dimension expanded,
whereas `img2` will get its new third dimension expanded. The output image
`img3` will thus have 50x30x60 pixels.
