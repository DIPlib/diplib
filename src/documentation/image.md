\class dip::Image

Represents an image with all associated information.

An Image object is the core of the *DIPlib* library, as all functionality
revolves around images. Some image manipulation is provided as class
methods, but most image processing and analysis functionality is provided
in functions defined in the #dip namespace.

[//]: # (--------------------------------------------------------------)

\section image_representation Image representation

An Image object can have any number of dimensions (from 0 up to
billions), though 0D, 2D and 3D are the most often used dimensionalies.
Most functions in the library accept images with any number of
dimensions, for the functions that are limited in this respect there is
a note in the documentation. In the following, we refer to a dimension
with size 1 (a single pixel) as a singleton dimension.

An Image object has pixels represented by a tensor. We have limited the
tensors to have no more than two dimensions (a matrix), as there
doesn't seem to be much use for higher-dimensional tensors in image
analysis. However, there is no limit to the number of tensor elements
(other than available memory and the size of pointers in the underlying
system). If the tensor is 0D (a single value), the image is a standard
gray-value image. A 1D tensor (a vector) can be used to represent color
images (e.g. the three values of an RGB image), but also for example
the image gradient (TODO: add reference to the gradient function). With
a 2D tensor (a matrix) it is possible to represent concepts such as the
Hessian and the structure tensor. For more details, see the section on
\ref tensors.

An Image object can contain data of a wide variety of numeric types,
including binary, unsigned and signed integers, floating point, and
complex. For a complete list see the desciption of the DataType class.

All of these image properties are dynamic. That is, they can be
determined and changed at runtime, they are not fixed at compile time.
An Image object has two states: *raw* and *forged*. When an image is
**raw**, it has no associated data segment. In the raw state, all image
properties can be changed. The Image::Forge method allocates the data
segment (the memory block that holds the pixel values). Once the image
is **forged**, its properties are fixed. It is possible to call the
Image::Strip method to revert to the raw state. The reason behind this
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
and how the data is stored in memory, an Image object specifies a
`strides` array. This array indicates, for each dimension, how many
pixels to skip to get to the neighboring pixel. For example, to go from
a pixel at coordinates (`x`,`y`) to the neighbour at coordinates
(`x+1`,`y`), you would need to increment the data pointer with
`strides[0]`. In a 2D image, the pixel at coordinates (`x`,`y`) can be
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
which can be obtained through Image::TensorStride. Even for a 2D
tensor, all tensor elements can be visited using a single stride value.
See the section \ref tensors for more information on accessing tensor
elements.

[//]: # (--------------------------------------------------------------)

\section tensors Tensor images

For details on possible tensor representations, see dip::Tensor::Shape.

[//]: # (--------------------------------------------------------------)

\section assignment Assignment

The assignment operator has two distinct meanings, depending on whether
the Image object is raw or forged:

    dest = src;

If the Image object `dest` is raw, assigning an Image `src` to it causes the
Image object to be an identical copy of `src`, pointing at the same
data segment. The two images share the data segment using reference
counting, this memory will not be freed until the last of the Image
objects referring to is is destroyed or stripped (Image::Strip).

If the Image object `dest` is forged, assigning an Image `src` to it
causes the pixel values in `src` to be copied to `dest`. The sizes of
the two images are expected to be identical, but \ref
singleton_expansion "singleton expansion" is performed first. Alternatively,
`src` can be a 1D image with the same number of pixels as `dest`. Pixels
are then assigned into `dest` by iterating first over the first dimension,
then over the second, etc. That is, the order in which the pixels are
stored in memory is irrelevant when making this assignment.

[//]: # (--------------------------------------------------------------)

\section indexing Indexing


\subsection tensor_indexing Tensor dimensions


\subsection regular_indexing Regular indexing (windows, ROI processing, subsampling)

**Note** using linear index to get a pixel is slightly less efficient than accessing
a pixel by its coordinates, unless the image has only one dimension.


\subsection irregular_indexing Irregular indexing


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

In the case of assignment with forged images, the destination image will
not be adjusted, only the source image will be adjusted to match the
destination.
