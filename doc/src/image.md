\comment (c)2014-2020, Cris Luengo.

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


\class dip::Image
\brief Represents an image with all associated information.

A `dip::Image` object is the core of the *DIPlib* library, as all functionality
revolves around images. Some image manipulation is provided as class
methods, but most image processing and analysis functionality is provided
in functions defined in the \ref dip namespace.


\comment --------------------------------------------------------------

\section image_representation Image representation

An `dip::Image` object can have any number of dimensions (limited by the integer
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

If the tensor is 0D (a single sample), the image is a standard grey-value image
(which we refer to as scalar image). A 1D tensor (a vector) can be used to represent
color images (e.g. an RGB image has three samples per pixel), but also for example
the image gradient (see \ref dip::Gradient). With a 2D tensor (a matrix) it is possible
to represent concepts such as the Hessian and the structure tensor (see \ref why_tensors).
For example, the Hessian of a 3D image has 9 samples per pixel. For more details
on how tensor elements are stored, see the section on \ref tensors.

A `dip::Image` object can contain samples of a wide variety of numeric types,
including binary, unsigned and signed integers, floating point, and
complex. For a complete list see \ref pixeltypes.
All the image's samples must have the same type.

All of these image properties are dynamic. That is, they can be
determined and changed at runtime, they are not fixed at compile time.
An `Image` object has two states: **raw** and **forged**. When an image is
**raw**, it has no associated data segment. In the raw state, all image
properties can be changed. The \ref Forge method allocates the data
segment (the memory block that holds the pixel values). Once the image
is **forged**, its properties are fixed. It is possible to call the
\ref Strip method to revert to the raw state. The reason behind this
dynamic image structure is that it allows flexibility: one can read the
data in a file without knowing what the file's data type is going to
be; the file reading function can adjust the `Image` object's data type
(and dimensionality) at run time to accommodate any data that the file
might contain. Another advantage is that the programmer does not need
to think about, for example, what data type is appropriate as output of
a specific function. However, when desired, it is possible to control
the data types of images.


\comment --------------------------------------------------------------

\section strides Strides

For maximum flexibility in the relationship between image coordinates
and how the samples are stored in memory, a `dip::Image` object specifies a
**stride** array (\ref Strides). This array indicates, for each
dimension, how many samples to skip to get to the neighboring pixel in the
given dimension.
For example, to go from a pixel at coordinates (`x`,`y`) to the neighbor
at coordinates (`x+1`,`y`), you would need to increment the data pointer
with `strides[0]`. In a 2D image, the pixel at coordinates (`x`,`y`) can be
reached by (assuming \ref dip::DT_UINT8 data type):

```cpp
dip::uint8* origin = img.Origin();
dip::IntegerArray strides = img.Strides();
dip::uint8 value = *( origin + x * strides[0] + y * strides[1] );
```

This concept naturally scales with image dimensionality. Strides can be
negative, and need not be ordered in any particular way. This allows a
`dip::Image` object to contain a regular subset of the pixels of another
image, but still point to the same physical memory block. Here are some
examples:

- An `Image` object can contain a region of interest (ROI), a smaller
  region within a larger image. In this case, the strides are identical
  to those of the larger image, but the origin and the image size
  differs.

- An `Image` object can contain a subsampled image, where the strides
  are a multiple of the strides of the original image.

- An `Image` object can contain a single slice of a 3D image, where the
  strides are identical to the 3D image's strides, but the origin and
  the image size and dimensionality are different.

- An `Image` object can contain a mirrored image, where the stride in
  the mirrored dimension is negative, and the origin is different.

- An `Image` object can contain a rotated image. For example, rotating
  over 90 degrees involves swapping the two dimensions (i.e. swapping the
  sizes and strides associated to these dimensions), and inverting one
  of the dimensions (as in the case of the mirrored image).

Arbitrary strides also allow data segments from other software to be
encapsulated by an `Image` object. For example, *MATLAB* stores images
with columns contiguous in memory, requiring `strides[1] == 1`.

All routines in the library support images with arbitrary strides.

The various elements of a tensor are also accessed through a stride,
which can be obtained through \ref TensorStride. Even for a 2D
tensor, all tensor elements can be visited using a single stride value.
See the section \ref tensors for more information on accessing tensor
elements. And see the section \ref pointers for more information about
accessing samples. See the section \ref normal_strides for information
on the default strides.


\comment --------------------------------------------------------------

\section tensors Tensor images

A tensor image (generalization of the vector and matrix image) has a tensor
for each pixel. A tensor collects all samples corresponding to the same spatial
location into a specific shape. \ref TensorElements indicates how
many samples per pixel the image has.

A tensor image is stored into a single memory block. In the same way that
strides indicate how to skip from one pixel to another (as described in the
section \ref strides), the \ref TensorStride indicates how to skip
from one tensor element to another. This allows e.g. a multi-channel image to
be stored as either interleaved per pixel, per line or per plane, and as long
as an algorithm uses the strides, it does not need to know how the channels
are interleaved.

All tensor elements are stored as if they composed a single spatial dimension.
Therefore, it is possible to change the image such that the tensor elements
form a new spatial dimension (\ref TensorToSpatial), or such that one
spatial dimension is converted to a tensor (\ref SpatialToTensor),
without moving the samples. It is also possible to change the shape of the
tensor without moving data (\ref ReshapeTensorAsVector,
\ref Transpose).

The shape of the tensor is represented by the enumerator \ref dip::Tensor::Shape
(obtained through \ref TensorShape).
The chosen way of storing tensor elements allows us, for example, to store
a symmetric 2D tensor such as the Hessian matrix without repeating the
repeating the duplicated values. We also have a specific shape for diagonal
matrices and triangular matrices.


\comment --------------------------------------------------------------

\section pointers On pixel coordinates, indices, offsets and data pointers

Given

```cpp
dip::Image img( { 10, 12, 20, 8, 18 }, 1, dip::DT_UINT16 );
```

Then \ref Origin "`img.Origin()`" is a `void*` pointer to
the first pixel (or rather the first sample of in the image).
This pointer needs to be cast to the type given by
\ref DataType "img.DataType()" to be used, as in:

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
\ref dip::DataType::SizeOf operator (we cast to \ref dip::uint8 pointer to do
pointer arithmetic in bytes):

```cpp
(dip::uint8*)img.Origin() + offset * img.DataType().SizeOf();
```

This computation is performed by
\ref Pointer "`img.Pointer( offset )`".

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
\ref Offset "`img.Offset( coords )`".
\ref Pointer "`img.Pointer( coords )`" simply chains this
operation with the previous one. The inverse operation is performed by
\ref OffsetToCoordinates "`img.OffsetToCoordinates( offset )`".
Two images of the same size do not necessarily share offset values.
Both the dimensions and the strides must be identical for the offset to be
compatible between the images.

The coordinates to a pixel simply indicate the number of pixels to skip along
each dimension. The first dimension (dimension 0) is typically `x`, but this
is not evident anywhere in the library, so it is the application using the
library that would make this decision. Coordinates start at 0, and should be
smaller than the `img.Sizes()` value for that dimension. They are encoded
using a \ref dip::UnsignedArray. However, some functions take coordinates as
a \ref dip::IntegerArray. These are the functions that do not expect the coordinates
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

This computation is performed by \ref Index "`img.Index( coords )`".
It is the *n*D equivalent to `x + y * width`. An index, as opposed to an
offset, is always non-negative, and therefore stored in an unsigned integer. The
index is shared among any images with the same dimensions.

It is not efficient to use indices to access many pixels, as the relationship
between the index and the offset is non-trivial. One can determine the
coordinates corresponding to an index through
\ref IndexToCoordinates "`img.IndexToCoordinates( index )`",
which then leads to an offset or a pointer.
The function \ref At with a scalar argument uses linear indices, and
consequently is not efficient for images with dimensionality of 2 or more.

Oftentimes it is possible to determine a **simple stride** that will allow you to
access every pixel in an image. When an image is a view into another image,
this is not necessarily possible, but any default image (i.e. with \ref normal_strides)
has this possibility. This simple stride allows one to view the image as a
1D image. The function \ref Flatten will create this 1D image (without
copying any data, if there exists such a simple stride). Walking along this
one dimension will, however, not necessarily access the pixels in the same order
as given by the linear index. This order is only consistent if the image has
normal strides. See \ref HasNormalStrides,
\ref HasSimpleStride, \ref GetSimpleStrideAndOrigin.

To walk along all pixels in an arbitrary image (i.e. arbitrary dimensionality
and strides) in the order given by the linear index, use the **image iterators**
defined in \ref "diplib/iterators.h"
(see \ref using_iterators "Using iterators to implement filters"):

```cpp
dip::ImageIterator< dip::uint16 > it( img );
dip::uint16 ii = 0;
do {
  *it = ii++;
} while( ++it );
```

The functionality in the \ref dip::Framework namespace is the recommended way of
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
a pixel, and therefore \ref Offset always gives the offset to the
first sample of a pixel. The other samples can be accessed by adding
`n * img.TensorStride()` to the offset, where `n` is the tensor element. Tensor
elements are then accessed in a specific order, depending on the shape of the
tensor. See \ref dip::Tensor::Shape for a description of the order of the tensor
elements in memory. For the iterators, use `it[n]` to access the `n`th tensor
element.


\comment --------------------------------------------------------------

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

dip::Image img2( dip::UnsingedArray{ 256, 256 }, 1, dip::DT_UINT8 );
```

The first method is more flexible, as it allows to set all properties
before forging the image (such as strides).
Note that the created image has uninitialized pixel data. You can use the
\ref Fill method to set all pixels to a specific value.

To create a new image with same sizes and tensor shape as another one,
use the \ref Similar method:

```cpp
dip::Image img2 = img1.Similar();
```

Again, the new image will have uninitialized pixel data. An optional second
argument can be used to specify the data type of the new image:

```cpp
dip::Image img2 = img1.Similar( dip::DT_SCOMPLEX );
```

Both methods copy all image properties, including the strides array and the
external interface; see \ref CopyProperties.

A similar method is \ref ReForge, which modifies the properties of
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
\ref ReForge has two other forms that can be useful, see the
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
data. However, if the image assigned into is protected or has an external
interface set, a data copy might be triggered, see \ref protect and
\ref external_interface.

\ref QuickCopy can be used here if the image copy does not need any of
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
\ref Copy method:

```cpp
img2.Copy( img1 );
```

or equivalently the \ref dip::Copy function:

```cpp
img2 = dip::Copy( img1 );
```

In both cases, `img2` will be identical to `img1`, with identical pixel values,
and with its own data segment.

When the \ref Copy method is used on a forged image, it is expected to
be of the same size as the image to be copied. Pixel values will be copied to
the existing data segment, casting to the target image's data type with clamping
(see \ref "diplib/library/clamp_cast.h"):

```cpp
img2 = img1.Similar( dip::DT_UINT8 );
img2.Copy( img1 );
```

Or equivalently:

```cpp
img2 = dip::Convert( img1, dip::DT_UINT8 );
```

The \ref Convert method, as opposed to the \ref dip::Convert function, converts
the image itself to a new data type. This process creates a new data segment if
the data type is of different size, and works in place if the data type is of the
same size (e.g. \ref dip::DT_SINT32 and \ref dip::DT_SFLOAT have the same size, as do
\ref dip::DT_DFLOAT and \ref dip::DT_SCOMPLEX). However, if the data segment is shared,
it will never work in place, as that could cause important problems.

Assigning a constant to an image is equivalent to calling the \ref Fill
method, writing that constant to each sample in the image. The constant is
cast to the image's data type with saturation (see \ref dip::clamp_cast). Note that
the image must be forged:

```cpp
img1 = 10;
img2.Fill( 0 );
```

Additionally, one can assign an initializer list to an image. The list should
have one element for each tensor element. Each pixel in the image will then
be set to the tensor values in the initializer list:

```cpp
img2 = dip::Image( dip::UnsignedArray{ 256, 256 }, 10, dip::DT_SFLOAT );
img2 = { 1, 2, 3, 4 };
```

`img2` will have all pixels set to the same vector `[ 1, 2, 3, 4 ]`.


\comment --------------------------------------------------------------

\section indexing Indexing

There are three main modes of indexing pixels in a `dip::Image` object: single
pixel indexing, regular grid indexing, and irregular pixel indexing. All three
modes have similar, consistent syntax, but differ by the return type and properties.
Additionally, one can index into the tensor dimension. The next four sub-sections
describe these different indexing modes.

All indexing modes share in common that the returned object can be assigned to,
modifying the original image. When indexing a single pixel, the return type is
a \ref dip::Image::Pixel, which references a single pixel in the original image.
When indexing multiple pixels (either by indexing the tensor dimension or by
regular or irregular indexing), the return type is a \ref dip::Image::View, which
references multiple pixels in the original image. Both these objects have
some overloaded methods and can be iterated over. Additionally, the \ref dip::Image::View
object implicitly casts back to a \ref dip::Image, so it can be used as arguments
to functions that take an `Image` as input. Here is where the difference between regular
and irregular indexing comes into play: the `Image` that results from regular
indexing (and from tensor indexing) shares the data with the original image, whereas
when the indexing was irregular, the data needs to be copied to create a new image
with only those pixels.

The following table summarizes the various indexing types discussed in detail in this
section.

<div markdown="1" class="m-smaller-font m-spaced m-block m-flat">
\comment .m-block.m-flat means that this div will not be framed (flat) but will have a margin at the bottom.
\comment The table, being the last child of the div, will not have a margin at the bottom, and we want some separation there.

&nbsp;                           | Single pixel                                       | Tensor                                                                                                                              | Regular                                          | Mask image          | Set of pixels
---------------------------------|----------------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------|--------------------------------------------------|---------------------|------------------------
Syntax                           | `.At(dip::uint, ...)`<br>`.At(dip::UnsignedArray)` | `[dip::uint]`<br>`[dip::UnsignedArray]`<br>`[dip::Range]`<br>`.Diagonal()`<br>`.TensorRow(dip::uint)`<br>`.TensorColumn(dip::uint)` | `.At(dip::Range, ...)`<br>`.At(dip::RangeArray)` | `.At(dip::Image)`   | `.At(dip::CoordinateArray)`<br>`.AtIndices(dip::UnsignedArray)`
Output                           | `dip::Image::Pixel`                                | `dip::Image::View`                                                                                                                  | `dip::Image::View`                               | `dip::Image::View`  | `dip::Image::View`
Implicitly casts to `dip::Image` | No                                                 | Yes, with shared data                                                                                                               | Yes, with shared data                            | Yes, with data copy | Yes, with data copy

</div>

!!! attention
    The result of an indexing operation can be used as input image to functions, but
    not as output image: output images are taken by reference, which cannot bind a temporary.
    Thus, the following does not compile:

        :::cpp
        dip::Gauss( in[ 0 ], out[ 0 ] ); // Does not compile.

    The output image must always be a `dip::Image` object with a name (i.e. a variable of type `dip::Image`):

        :::cpp
        dip::Image channel = out[ 0 ];
        dip::Gauss( in[ 0 ], channel ); // Writes to `out` (assuming `channel` has the expected properties).

    Do note that some indexing operations, when cast to a `dip::Image`, cause pixels to be copied, and
    hence cannot be used to write into the original image:

        :::cpp
        dip::Image channel = out.At( mask );      // `mask` here is a binary image.
        dip::Threshold( in.At( mask ), channel ); // Warning! `out` is not modified.

    Instead, copy the result of the operation directly into the result of the indexing operation:

        :::cpp
        out.At( mask ) = dip::Threshold( in.At( mask )); // This does incur an extra copy.


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

The indexing operation, as explained above, returns a \ref dip::Image::View object,
which can be assigned to to modify the referenced pixels:

```cpp
colorIm[ 0 ] = colorIm[ 1 ];
```

When cast back to an image, the image created shares the pixels with the original image,
meaning that it is possible to write to a channel in this way:

```cpp
dip::Image blueChannel = colorIm[ 2 ];
blueChannel.Protect();                         // Prevent `dip::Gauss` from reforging this image.
dip::Gauss( blueChannel, blueChannel, { 4 } ); // Modifies `colorIm`.
```

Note that the single-index version of the tensor indexing uses linear indexing into
the tensor element list: if the tensor is, for example, a 2x2 symmetric matrix, only
three elements are actually stored, with indices 0 and 1 representing the two diagonal
elements, and index 2 representing the two identical off-diagonal elements. See
\ref dip::Tensor for information on how the tensor is stored.

To extract multiple tensor elements one can use a \ref dip::Range to index. Other useful
methods here are \ref Diagonal, \ref TensorRow and
\ref TensorColumn, which all yield a vector image.

The methods \ref Real and \ref Imaginary are somewhat related to these
last three methods in that they return a view over a subset of the data, except that
they reference only the real or imaginary component of the complex sample values.


\subsection pixel_indexing Single-pixel indexing

Some forms of the function \ref At extract a single pixel from the image. It has different
forms, accepting either a \ref dip::UnsignedArray representing the coordinates of the pixel,
one to three indices for 1D to 3D images, or a (linear) index to the pixel. The latter
form is less efficient because the linear index needs to be translated to coordinates,
as the linear index is not necessarily related to the order in which pixels are stored
in memory.

```cpp
image1D.At( 5 );                           // Indexes pixel at coordinate 5
image2D.At( 0, 10 );                       // Indexes the pixel at coordinates (0, 10)
image2D.At( dip::UnsignedArray{ 0, 10 } ); // Indexes the pixel at coordinates (0, 10)
image2D.At( 20 );                          // Indexes the pixel with linear index 20
```

These forms result in an object of type \ref dip::Image::Pixel. The object contains a
reference to the original image pixels, so writing to the reference changes the pixel
values in the image:

```cpp
image.At( 0, 10 ) += 1;
```

The single-pixel forms of \ref At have an alternative, templated form. In that
form, they return a \ref dip::Image::CastPixel instead, which is identical to a
\ref dip::Image::Pixel, but implicitly casts to any chosen type. Thus:

```cpp
int v1 = image.At( 0, 10 );             // Does not compile, no implicit conversion to `int`.
int v2 = image.At< int >( 0, 10 );      // OK
int v3 = image.At( 0, 10 ).As< int >(); // Same as `v2`.
```

This implicit cast makes its use a little simpler in a setting combined with numbers
of that type. However, the object itself is not tied to the type, and it is still possible
to access (read or write) the pixel as other types. For example, this code will print
"( 1.0, -3.0 )" to the standard output, the template argument to `At` has no influence
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

The first line above uses `[]` to index into the \ref dip::Image::Pixel object, yielding
a \ref dip::Image::Sample. Again, this is a reference to the sample in the image, and can
be written to to change the image. The second line first uses `[]` to create a scalar
image view, and then extracts a pixel from it. The result is a `dip::Image::Pixel` object,
not a `dip::Image::Sample`, though the pixel object references a single sample in the
input image. In practice, these are equally useful, though the first form is slightly
more efficient because the intermediate object generated has less overhead.

See the documentation to \ref dip::Image::Sample and \ref dip::Image::Pixel for more information
on how these objects can be used. Note however, that this is not an efficient way of
accessing pixels in an image. It is much more efficient to use pointers into the
data segment. However, pointers require knowledge of the data type at compile time.
The indexing described here is a convenient way to read a particular value in a
data-type agnostic way. To access all pixels in a data-type agnostic way, use the
\ref dip::GenericImageIterator.


\subsection regular_indexing Regular indexing (windows, ROI processing, subsampling)

The function \ref At is also used for creating views that represent a subset of
pixels. In this form, it accepts a set of \ref dip::Range objects, or a \ref dip::RangeArray,
representing regular pixel intervals along each dimension through a start, stop and
step value. That is, a range indicates a portion of an image line, with optional
subsampling. For example, indexing into a 1D image:

```cpp
image1D.At( dip::Range{ 5 } );        // Indexes pixel at coordinate 5
image1D.At( dip::Range{ 0, 10 } );    // Indexes the first 11 pixels
image1D.At( dip::Range{ 0, -1, 2 } ); // Indexes every second pixel
```

Note that negative values index from the end, without needing to know the exact
size of the image. Note also that the stop value is always included in the range.

For indexing into multidimensional images, simply provide one range per dimension.
For more than 3 dimensions, use a \ref dip::RangeArray.

As is the case with the `[]` indexing, these operations yield a \ref dip::Image::View that
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

The method \ref Cropped is a convenience function to extract a view to a
rectangular widow of a given size and with a given anchor (image center or corner).
The \ref Pad method does the opposite operation, but creates a new image and
copies the data over.


\subsection irregular_indexing Irregular indexing (mask image, arbitrary set of pixels)

An arbitrary subset of pixels can be indexed in three ways, using one of:

- a mask image
- a list of pixel coordinates
- a list of linear indices into the image

The first two forms are again available through the \ref At method, the
third one through the \ref AtIndices method (since the input argument would
not be distinguishable from indexing a single pixel through its coordinates). As with
regular indexing, the returned object is a \ref dip::Image::View, which can be assigned
to to change the referenced pixels. But as opposed to regular indexing, when cast
back to a `dip::Image`, the operation results in a 1D image with a copy of the sample
values. Thus, once cast back to an `Image`, pixels are no longer shared and can
be modified without affecting the original image. This is by necessity, as the `Image`
object cannot reference samples that are not stored in memory on a regular grid.

```cpp
image.At( mask ) += 1;
dip::Image out = image.At( mask );
out.Fill( 0 ); // Careful! Does not affect `image`.
```

Another typical example:

```cpp
image.At( mask ) = otherImage.At( mask );
```


\comment --------------------------------------------------------------

\section reshaping Reshaping

There is a large collection of methods to reshape the image without physically
moving samples around in memory. These functions accomplish their goal simply
by modifying the sizes and strides arrays, and the tensor representation
values. Thus, they are very cheap. All of these functions modify the object
directly, and return a reference so that they can be chained.

For example, \ref Rotation90 rotates the image in 90 degree increments
by mirroring and swapping dimensions. More generic methods are \ref Mirror,
and \ref SwapDimensions. \ref PermuteDimensions reorders the
image's dimensions, an optionally adds or removes singleton dimensions. Singleton
dimensions are dimensions with a size of 1, and can be added or removed without
affecting the data layout in memory.

Several of these methods are meant to manipulate singleton dimensions.
\ref AddSingleton and \ref Squeeze add and remove singleton
dimensions. \ref ExpandDimensionality adds singleton dimensions at the
end to increase the number of image dimensions. It is also possible to expand
singleton dimensions so they no longer are singleton, by replicating the data
along that dimensions, again without physically replicating or modifying the
data in memory. See \ref singleton_expansion. The methods
\ref ExpandSingletonDimension, \ref ExpandSingletonDimensions
and \ref ExpandSingletonTensor are used for this.
\ref UnexpandSingletonDimensions does the opposite operation.

The method \ref StandardizeStrides undoes all rotation, mirroring,
dimension reordering, and singleton expansion, by making all strides positive
and sorting them from smallest to largest. It also removes singleton dimensions
(as \ref Squeeze).
The \ref Flatten method converts the image to 1D (though if the image
does not have contiguous data, it will have to be copied to form a 1D image).
\ref FlattenAsMuchAsPossible does the same thing, but never copies.
If the data is not contiguous, the image will not be 1D, though it will hopefully
have fewer dimensions than before the method call. \ref SplitDimension
splits a dimension into two.

A group of methods manipulate the image's tensor shape:
\ref ReshapeTensor requires the target tensor shape to have as many
tensor elements as the image already has. For example, a 3-vector image can be
converted into a 2x2 symmetric tensor image. \ref ReshapeTensorAsVector and
\ref ReshapeTensorAsDiagonal turn the image into the given tensor shapes.
The \ref Transpose method belongs in this set, though it has a mathematical
meaning. `Transpose` changes the row-major matrix into a column-major matrix and
vice-versa, thereby transposing the tensor.

It is also possible to turn the tensor dimension into a spatial dimension and
back, using \ref TensorToSpatial and \ref SpatialToTensor.
Turning the tensor dimension into a spatial dimension can be useful for example
to apply the same operation to all samples in a vector image: it is easier to loop
over a scalar image with a single \ref dip::ImageIterator loop than to loop over a
tensor image using a double loop over pixels and over tensor elements.

Similar in purpose and function to \ref TensorToSpatial are functions
that split the complex-valued samples into two floating-point--valued samples,
and present these as either a new spatial dimension, or the tensor dimension:
\ref SplitComplex, and \ref SplitComplexToTensor. The inverse
operation is accomplished with \ref MergeComplex and
\ref MergeTensorToComplex.


\comment --------------------------------------------------------------

\section protect The "protect" flag

An image carries a "protect" flag. When set, the \ref Strip function
throws an exception. That is, when the flag is set, the data segment cannot
be stripped (freed) or reforged (reallocated). Furthermore, when the protect
flag is set, the assignment operator will perform a deep copy. For example:

```cpp
dip::Image img1( dip::UnsignedArray{ 256, 256 }, 3, dip::DT_SFLOAT );
img1.Protect();
//img1.Strip();  // Throws!
img1 = img2;     // Equivalent to: `img1.Copy( img2 )`.
```

The protect flag has two purposes:

**First:** To prevent an image from being reforged, for example when the data segment
is allocated in a special way and one needs to ensure it stays that way. In this
case, it functions as a warning.

**Second:** To provide a simple means of specifying the data type for the output image
or a filter. Most filters and operations in
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
// `out` is forged with correct sizes to receive filter result, and as 16-bit integer.
```

This is especially suitable for in-place operations where we want to receive the
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

Note, however, that some functions require specific data types for their output image,
and will throw an error if the wrong output data type is requested.


\comment --------------------------------------------------------------

\section const_correctness Const correctness

When an image object is marked `const`, the compiler will prevent modifications
to it, it cannot be assigned to, and it cannot be used as the output argument
to a filter function. However, the \ref indexing "indexing operators", the copy
assignment operator, and \ref QuickCopy all allow the user to make
a non-const object that points to the same data, making it possible to
modify the pixel values of a const image (see \ref design_const_correctness
for our reasons to allow this). Because of that, it did not really make sense
either to have \ref Data, \ref Origin, and \ref Pointer
return const pointers when applied to a const image.

Thus, there is nothing prevent you from modifying the pixel values of a const image.
However, none of the functions in *DIPlib* will do so. A const image (usually
the input images to functions are marked const) will not be modified.


\comment --------------------------------------------------------------

\section arithmetic Arithmetic and comparison operators

Arithmetic operations, logical operations, and comparisons
can all be performed using operators, but there are also functions defined
that perform the same function with more flexibility. See \ref math_arithmetic
and \ref math_comparison for a full list of these functions.

For example, to add two images `a` and `b`, one can simply do:

```cpp
dip::Image c = a + b;
```

But it is also possible to control the output image by using the \ref dip::Add
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

```cpp
a += b;
```

All dyadic operations (arithmetic, logical, comparison) perform \ref singleton_expansion.
They also correctly handle tensor images of any shape. For example, it is possible
to add a vector image and a tensor image, but it is not possible to add two vector
images of different lengths. The multiplication operation always performs matrix
multiplication on the tensors, use the \ref dip::MultiplySampleWise function to do
sample-wise multiplication. Other operators are sample-wise by definition (including
the division, which is not really defined for tensors).


\comment --------------------------------------------------------------

\section color Color images

A color image is a vector image where each vector element represents a color
channel. \ref SetColorSpace assigns a string into the image object that
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

An object of type \ref dip::ColorSpaceManager is used to convert an image from one
known color space to another. This object is the only place in the library where
there is knowledge about color spaces. Create one of these objects for any
application that requires color space knowledge. It is possible to register new
color spaces and color space conversion functions with this object. Other functions
that use specific color spaces will have knowledge only of those specific color
spaces, and will expect their input to be in one of those color spaces.


\comment --------------------------------------------------------------

\section pixel_size Pixel size

Each image carries with it the size of its pixels as a series of physical
quantities (\ref dip::PhysicalQuantity), one for each image dimension. The advantage
of keeping the pixel size with the image rather than as a separate value taken
manually from the image file metadata is that it is automatically updated through
manipulations such as scaling (zoom), dimension permutations, etc. When the pixel
size in a particular dimension is not set, it is always presumed to be of size 1
(a dimensionless unit). See \ref dip::PixelSize for details.

There are three ways in which the pixel size can be used:

- The measurement function will return its measurements as physical quantities,
  using the pixel sizes, if known, to derive those from measurements in pixels.

- The `dip::Image::PhysicalToPixels` method converts a filter size in physical
  units to one in pixels, suitable to pass to a filtering function. For example,
  to apply a filter with a sigma of 1 micron to an image:

        :::cpp
        dip::PhysicalQuantityArray fsz_phys{ 1 * dip::PhysicalQuantity::Micrometer() };
        dip::Filter( img, img, img.PhysicalToPixels( fsz_phys ));

- The `dip::Image::PixelsToPhysical` method converts coordinates in pixels to
  coordinates in physical units. For example, to determine the position in
  physical units of a pixel w.r.t. the top left pixel:

        :::cpp
        dip::FloatArray pos_pix{ 40, 24 };
        dip::PhysicalQuantityArray pos_phys = img.PixelsToPhysical( pos_pix );

It is currently possible to add, subtract, multiply and divide two physical quantities,
and elevate a physical quantity to an integer power. Other operations should be
added as necessary.


\comment --------------------------------------------------------------

\section external_data_segment Controlling data segment allocation

It is possible to create `dip::Image` objects whose pixels are not allocated
by *DIPlib* using two different methods:

1. Create an image around an existing data segment. This is used in the case
when passing data from other imaging libraries, or from interpreted languages
that have their own array type. Just about any data can be encapsulated by a
a `dip::Image` without copy.

2. Define an image's allocator, so that when *DIPlib* forges the image, the
user's allocator is called instead of `std::malloc`. This is used in the
case when the result of *DIPlib* functions will be passed to another
imaging library, or to an interpreted language.

In both cases, \ref IsExternalData returns true.

Note that when the image needs to be reforged (sizes and/or data type do not
match what is required for output by some function), but the data segment is
of the correct size, \ref ReForge would typically re-use the data
segment if it is not shared with another image. However, when the data segment
is external, it is always reforged.

\subsection use_external_data Create an image around existing data

One of the `dip::Image` constructors takes a pointer to the first pixel of a
data segment (i.e. pixel buffer), and image sizes and strides, and creates a new
image that references that data. The only requirement is that all pixels are
aligned on boundaries given by the size of the sample data type. That is,
*DIPlib* strides are not in bytes but in samples. The resulting image is identical
to any other image in all respects, except the behavior of \ref ReForge,
as mentioned in the previous paragraph.

For example, in this bit of code we take the data of a `std::vector` and build an
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
img.Protect();          // Prevent `dip::Gauss` from reallocating its output image.
dip::Gauss( img, img ); // Apply Gaussian filter, output is written to input array.
```

The first argument to this constructor is a \ref dip::DataSegment object, which is just
a shared pointer to void. If you want the `dip::Image` object to own the resources,
pass a shared pointer with an appropriate deleter function. Otherwise, use the
function \ref dip::NonOwnedRefToDataSegment to create a shared pointer without a deleter
function (as in the example above), indicating that ownership is not to be transferred.
In the example below, we do the same as above, but we transfer ownership of the
`std::vector` to the image. When the image goes out of scope (or is reallocated) the
`std::vector` is deleted:

```cpp
auto src = new std::vector<unsigned char>( 256 * 256, 0 ); // existing data
dip::Image img(
   dip::DataSegment{ src }, // transfer ownership of the data
   src->data(),   // origin
   dip::DT_UINT8, // dataType
   { 256, 256 },  // sizes
   { 1, 256 }     // strides
);
dip::Gauss( img, img ); // Apply Gaussian filter, output is written to a different data segment.
```

After the call to \ref dip::Gauss, `*src` no longer exists. `dip::Gauss` has reforged
`img` to be of type \ref dip::DT_SFLOAT, triggering the deletion of object pointed to
by `src`.

Another form of the constructor simplifies the above in the case where ownership is not
transferred. It takes two or three input arguments: a pointer to the data and an array
with sizes, and optionally the number of tensor elements (channels). The strides are
assumed to be normal (see \ref normal_strides).
The data pointer must be of any of the allowed data types:

```cpp
std::vector<unsigned char> src( 256 * 256, 0 ); // existing data
dip::Image img(
   src.data(),    // origin
   { 256, 256 }   // sizes
);
```

\subsection external_interface Define an image's allocator

Int the previous sub-section we saw how to encapsulate external data. The
resulting image object can be used as input and output, but most *DIPlib*
functions will reforge their output images to be of appropriate size and data type.
Unless the allocated image is of suitable size and data type, and the image is
protected, a new data segment will be allocated. This means that the output of the
function call will not be written into the buffer we had provided.

Instead we can define an allocator class, derived from \ref dip::ExternalInterface,
and assign a pointer to an object of the allocator class to an image using
\ref SetExternalInterface. When that image is forged or reforged,
the \ref dip::ExternalInterface::AllocateData method is called. This method is
free to allocate whatever objects it needs, and sets the image's origin pointer
and strides (much like in the previous section). As before, it is possible to
set the \ref dip::DataSegment also, so that ownership of the allocated objects is
transferred to the image object. As can be seen in the *DIPlib--MATLAB* interface
(see \ref dml::MatlabInterface in \ref "dip_matlab_interface.h"), the
\ref dip::DataSegment can contain a custom deleter function that again calls the
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
indicating to the calling \ref Forge function that it cannot allocate
such an image. `Forge` will then allocate the data itself. If you prefer the
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
            return nullptr; // We do not want to handle such images.
         }
         auto data = new std::vector< unsigned char >( sizes[ 0 ] * sizes[ 1 ] * datatype.SizeOf(), 0 );
         origin = data.data();
         strides = dip::UnsignedArray{ 1, sizes[ 0 ] };
         tstride = 1;
         return dip::DataSegment{ data };
      }
}
```

See the source code to \ref dml::MatlabInterface for a more realistic example
of this feature. That class stores the data in such a way that ownership can
be retrieved from the shared pointer, so that the data array can be used by
*MATLAB* after the `dip::Image` object that originally owned it has been
destroyed.


\comment --------------------------------------------------------------

\section singleton_expansion Singleton expansion

When two images need to be of the same size, a process we refer to as
singleton expansion is performed. First, singleton dimensions (dimensions
with a size of 1) are added to the image with the fewer dimensions. This
process does not require a data copy. Next, all singleton dimensions
are expanded to match the size of the corresponding dimension in the other
image. This expansion simply repeats the value all along that dimension,
and is accomplished by setting the image's size in that dimension to
the expected value, and the corresponding stride to 0 (again, no data are
physically copied). This will cause an algorithm to read the same value,
no matter how many steps it takes along this dimension. For example:

```cpp
dip::Image img1( dip::UnsignedArray{ 50, 1, 60 } );
dip::Image img2( dip::UnsignedArray{ 50, 30 } );
dip::Image img3 = img1 + img2;
```

Here, the dimension array for `img2` will be extended to `{ 50, 30, 1 }`
in the first step. In the second step, the arrays for both images will
be changed to `{ 50, 30, 60 }`. `img1` gets its second dimension expanded,
whereas `img2` will get its new third dimension expanded. The output image
`img3` will thus have 50x30x60 pixels.
