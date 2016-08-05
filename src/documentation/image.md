\ingroup GrpDummyPages

\class dip::Image

\brief Represents an image with all associated information.

An Image object is the core of the *DIPlib* library, as all functionality
revolves around images. Some image manipulation is provided as class
methods, but most image processing and analysis functionality is provided
in functions defined in the #dip namespace.

[//]: # (--------------------------------------------------------------)

\section image_representation Image representation

A dip::Image object can have any number of dimensions (limited by the integer
representation used), though 0D, 2D and 3D are the most often used dimensionalies.
Most functions in the library accept images with any number of
dimensions, for the functions that are limited in this respect there is
a note in the documentation. In the following, we refer to a dimension
with size 1 (a single pixel) as a singleton dimension.

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

If the tensor is 0D (a single sample), the image is a standard
gray-value image. A 1D tensor (a vector) can be used to represent color
images (e.g. an RGB image has three samples per pixel), but also for example
the image gradient (TODO: add reference to the gradient function). With
a 2D tensor (a matrix) it is possible to represent concepts such as the
Hessian and the structure tensor. For example, the Hessian of a 3D image
has 9 samples per pixel. For more details on how tensor elements are stored,
see the section on \ref tensors.

An Image object can contain samples of a wide variety of numeric types,
including binary, unsigned and signed integers, floating point, and
complex. For a complete list see the desciption of the dip::DataType class.
All the image's samples must have the same type.

All of these image properties are dynamic. That is, they can be
determined and changed at runtime, they are not fixed at compile time.
An Image object has two states: **raw** and **forged**. When an image is
**raw**, it has no associated data segment. In the raw state, all image
properties can be changed. The dip::Image::Forge method allocates the data
segment (the memory block that holds the pixel values). Once the image
is **forged**, its properties are fixed. It is possible to call the
dip::Image::Strip method to revert to the raw state. The reason behind this
dynamic image structure is that it allows flexibility: one can read the
data in a file without knowing what the file's data type is going to
be; the file reading function can adjust the Image object's data type
(and dimensionality) at run time to accomodate any data that the file
might contain. Another advantage is that the programmer does not need
to think about, for example, what data type is appropriate as output of
a specific function. However, when desired, it is possible to control
the data types of images.

[//]: # (--------------------------------------------------------------)

\section strides Strides

For maximum flexibility in the relationship between image coordinates
and how the samples are stored in memory, an Image object specifies a
**stride** array (dip::Image::Strides). This array indicates, for each dimension, how many
samples to skip to get to the neighboring pixel in the given dimension.
For example, to go from a pixel at coordinates (`x`,`y`) to the neighbour
at coordinates (`x+1`,`y`), you would need to increment the data pointer
with `strides[0]`. In a 2D image, the pixel at coordinates (`x`,`y`) can be
reached by (assuming dip::DT_UINT8 data type):

    dip::uint8* origin = img.Data();
    dip::IntegerArray strides = img.Strides();
    dip::uint8 value = *( origin + x * strides[0] + y * strudes[1] );

This concept naturally scales with image dimensionality. Strides can be
negative, and need not be ordered in any particular way. This allows an
Image object to contain a regular subset of the pixels of another
image, but still point to the same physical memory block. Here are some
examples:

  * An Image object can contain a region of interest (ROI), a smaller
  region within a larger image. In this case, the strides are identical
  to those of the larger image, but the origin and the image size
  differs.

  * An Image object can contain a subsampled image, where the strides
  are a multiple of the strides of the original image.

  * An Image object can contain a single slice of a 3D image, where the
  strides are identical to the 3D image's strides, but the origin and
  the image size and dimensionality are different.

  * An Image object can contain a mirrored image, where the stride in
  the mirrored dimension is negative, and the origin is different.

  * An Image object can contain a rotated image. For example, rotating
  over 90 degrees involves swapping the two dimensions (i.e. swapping the
  sizes and strides associated to these dimensions), and inverting one
  of the dimensions (as in the case of the mirrored image).

All routines in the library support images with arbitrary strides.

The various elements of a tensor are also accessed through a stride,
which can be obtained through dip::Image::TensorStride. Even for a 2D
tensor, all tensor elements can be visited using a single stride value.
See the section \ref tensors for more information on accessing tensor
elements. And see the section \ref pointers for more information about
accessing samples.

[//]: # (--------------------------------------------------------------)

\section tensors Tensor images

A tensor image (generalization of the vector and matrix image) has a tensor
for each pixel. A tensor collects all samples corresponding to the same spatial
location into a specific shape. dip::Image::TensorElements indicates how
many samples per pixel the image has.

A tensor image is stored into a single memory block. In the same way that
strides indicate how to skip from one pixel to another (as described in the
section \ref strides), the dip::Image::TensorStride indicates how to skip
from one tensor element to another.

All tensor elements are stored as if they composed a single spatial dimension.
Therefore, it is possible to change the image such that the tensor elements
form a new spatial dimension (dip::Image::TensorToSpatial), or such that one
spatial dimension is converted to a tensor (dip::Image::SpatialToTensor),
without moving the samples. It is also possible to change the shape of the
tensor without moving data (dip::Image::ReshapeTensorAsVector, dip::Image::Transpose).

The shape of the tensor is represented by the enumerator dip::Tensor::Shape
(obtained through dip::Image::Shape).
The chosen way of storing tensor elements allows us, for example, to store
a symmetric 2D tensor such as the Hessian matrix without repeating the
repeating the duplicated values. We also have a specific shape for diagonal
matrices and triangular matrices.

[//]: # (--------------------------------------------------------------)

\section pointers On pixel coordinates, indices, offsets and data pointers.

Given

    dip::Image img( dip::uint16, { 10, 12, 20, 8, 18 } );
    img.Forge();

Then `img.Origin()` (dip::Image::Origin) is a `void*` pointer to the first pixel
(or rather the first sample of in the image).
This pointer needs to be cast to the type given by `img.DataType()`
(dip::Image::DataType) to be used, as in:

    (dip::uint16*)img.Origin() = 0;

A pixel's **offset** is the number of samples to move away from the origin
to access that pixel:

    dip::uint16* ptr = (dip::uint16*)img.Origin();
    ptr + offset = 1;

Alternatively, it is possible to compute the pixel's pointer without casting
to the right data type (this leads to a more generic algorithm) by using the
dip::DataType::SizeOf operator:

    (dip::uint8*)img.Origin() + offset * img.DataType().SizeOf();

This computation is performed by `img.Pointer( offset )` (dip::Image::Pointer).

Note that the offset is a signed integer, and can be negative, because strides
can be negative also.
The offset is computed from coordinates using the image's strides:

    dip::UnsignedArray coords { 1, 2, 3, 4, 5 };
    dip::sint offset = 0;
    for( dip::uint ii = 0; ii < img.Dimensionality(); ++ii ) {
      offset += coords[ii] * img.Stride( ii );
    }

This computation is performed by `img.Offset( coords )` (dip::Image::Offset).
`img.Pointer( coords )` (dip::Image::Pointer) simply chains this operation
with the previous one. The inverse operation is performed by
`img.OffsetToCoordinates( offset )` (dip::Image::OffsetToCoordinates).
Two images of the same size do not necessarily share offset values.
Both the dimensions and the strides must be identical for the offset to be
compatible between the images.

The coordinates to a pixel simply indicate the number of pixels to skip along
each dimension. The first dimension (dimension 0) is typically `x`, but this
is not evident anywhere in the library, so it is the application using the
library that would make this decision. Coordinates start at 0, and should be
smaller than the `img.Dimensions()` value for that dimension.

The index to a pixel (a.k.a. "linear index") is a value that increases
monotonically as one moves from one pixel to the next, first along dimension 0,
then along dimension 1, etc. The index computed from a pixel's coordinates is
as follows:

    dip::UnsignedArray coords { 1, 2, 3, 4, 5 };
    dip::uint dd = img.Dimensionality();
    dip::uint index = 0;
    while( dd > 0 ) {
      --dd;
      index *= img.Dimension( dd );
      index += coords[dd];
    }

This computation is performed by `img.Index( coords )` (dip::Image::Index).
It is the *n*D equivalent to `x + y * width`. An index, as opposed to an
offset, is always non-negative, and therefore stored in an unsigned integer. The
index is shared among any images with the same dimensions.

It is not efficient to use indices to access many pixels, as the relationship
between the index and the offset is non-trivial. One can determine the
coordinates corresponding to an index through `img.IndexToCoordinates( index )`
(dip::Image::IndexToCoordinates), which then leads to an offset or a pointer.
The function dip::Image::At with a scalar argument uses linear indices, and
consequently is not efficient for images with dimensionality of 2 or more.

Oftentimes it is possible to determine a simple stride that will allow you to
access every pixel in an image. When an image is a view into another image,
this is not necessarily possible, but any default image (i.e. with normal strides)
has this possibility. This simple stride allows one to view the image as a
1D image. The function dip::Image::Flatten will create this 1D image (without
copying any data, if there exists such a simple stride). Walking along this
one dimension will, however, not necessarily access the pixels in the same order
as given by the linear index. This order is only consistent if the image has
normal strides. See dip::Image::HasNormalStrides,
dip::Image::HasSimpleStride, dip::Image::GetSimpleStrideAndOrigin.

To walk along all pixels in an arbitrary image (i.e. arbitrary dimensionality
and strides) in the order given by the linear index, use the functions in the
dip::NDLoop namespace:

    dip::sint offset;
    dip::UnsignedArray pos = dip::NDLoop::Init( offset, img.Dimensions() );
    dip::uint16* ptr = (dip::uint16*)img.Origin();
    dip::uint ii = 0;
    do {
      *(ptr + offset) = ii++;
    } while( dip::NDLoop::Next( img, pos, offset );

The functionality in the dip::Framework namespace is the recommended way of
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
a pixel, and therefore dip::Image::Offset always gives the offset to the
first sample of a pixel. The other samples can be accessed by adding
`n * img.TensorStride()` to the offset, where `n` is the tensor element. Tensor
elements are then accessed in a specific order, depending on the shape of the
tensor. See dip::Tensor::Shape for a description of the order of the tensor
elements in memory.

[//]: # (--------------------------------------------------------------)

\section indexing Indexing


\subsection tensor_indexing Tensor dimensions


\subsection regular_indexing Regular indexing (windows, ROI processing, subsampling)


\subsection irregular_indexing Irregular indexing

[//]: # (--------------------------------------------------------------)

\section assignment Assignment


[//]: # (--------------------------------------------------------------)

\section arithmetic Arithmetic and comparison operators

... \ref singleton_expansion "singleton expansion" is always performed
on the two operands ...


[//]: # (--------------------------------------------------------------)

\section color Color images


[//]: # (--------------------------------------------------------------)

\section phys_dims Physical dimensions


[//]: # (--------------------------------------------------------------)

\section external_interface Controlling data segment allocation


[//]: # (--------------------------------------------------------------)

\section singleton_expansion Singleton expansion

When two images need to be of the same size, a process we refer to as
singleton expansion is performed. First, singleton dimensions are added
to the image with the fewer dimensions. Next, all singleton dimensions
are expanded to match the size of the corresponding dimension in the other
image. This expansion simply repeats the value all along that dimension,
and is accomplished by setting the image's size in that dimension to
the expected value, and the corresponding stride to 0. This will cause
an algorithm to read the same value, no matter how many steps it takes
along this dimension. For example:

    dip::Image img1( UnsignedArray{ 50, 1, 60 } );
    dip::Image img2( UnsignedArray{ 50, 30 } );
    dip::Image img3 = img1 + img2;

Here, the dimension array for `img2` will be extended to `{ 50, 30, 1}`
in the first step. In the second step, the arrays for both images will
be changed to `{ 50, 30, 60 }`. `img1` gets its second dimension expanded,
wereas `img2` will get its new third dimension expanded. The output image
`img3` will thus have 50x30x60 pixels.

