\comment DIPlib 3.0

\comment (c)2017-2020, Cris Luengo.
\comment Based on original DIPimage user manual: (c)1999-2014, Delft University of Technology.

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


\page sec_dum_dip_image The `dip_image` Object

Images used by this toolbox are encapsulated in an object called
`dip_image`. Objects of this type are unlike regular *MATLAB* arrays in
some ways, but behave similarly most of the time. This chapter explains
the usage of these objects.

For more information on the functions mentioned in this chapter (and
elsewhere) use the *MATLAB* help system. At the *MATLAB* command prompt, type
`help <function_name>`.

\section sec_dum_dip_image_creating Creating a `dip_image` object

To create a `dip_image` object, the function `dip_image` must be used.
It converts any numeric array into an image object. The optional second
argument indicates the desired data type for the image. The pixel data
will be converted to this type if possible, or else an error will be
generated (for example, it is illegal to convert complex data to a real
type, since there are many ways this can be accomplished; it is
necessary to do this explicitly). The valid data types are listed in
the table below. This table also lists some alternative names
that are mapped to the names on the left; these are just to make
specifying the data type easier.

Data type  | Description               | Other allowed names
---------- | ------------------------- | -------------------
`bin`      | binary (in 8-bit integer) |
`uint8`    | 8-bit unsigned integer    |
`uint16`   | 16-bit unsigned integer   |
`uint32`   | 32-bit unsigned integer   |
`uint64`   | 64-bit unsigned integer   |
`sint8`    | 8-bit signed integer      | `int8`
`sint16`   | 16-bit signed integer     | `int16`
`sint32`   | 32-bit signed integer     | `int32`
`sint64`   | 64-bit signed integer     | `int64`
`sfloat`   | single precision float    | `single`
`dfloat`   | double precision float    | `double`
`scomplex` | single precision complex  |
`dcomplex` | double precision complex  |

For example,

```matlab
a = dip_image(a,'sfloat');
```

will convert the data in `a` to `single` (4-byte) floats before creating
the `dip_image` object. The variable `a` now behaves somewhat
differently than you might be used to. The following sections explain
its behavior.

To convert a `dip_image` object back to a *MATLAB* array use the function
`dip_array`. It simply returns the data array stored inside the
`dip_image` object. The functions `double`, `single`, `uint8`, etc.
convert the `dip_image` object to a *MATLAB* array of the specified class.

There are also some commands to create an image from scratch. `newim` is
equivalent to the `zeros` function, but returns a `dip_image` object.

```matlab
a = newim(256,256);
```

creates an image with 256x256 pixels set to zero. An additional
parameter (as in the table above) can be used to specify the
data type of the new image. The default is `'sfloat'`. If `b` is an
object of type `dip_image`, then

```matlab
a = newim(b);
```

creates an image of the same size (this is the same as
`newim(imsize(b))`). The functions `xx`, `yy`, `zz`, `rr` and `phiphi` all
create an image containing the coordinates of its pixels, and can be
used in formulas that need them. For example, `rr([256,256])<64` creates a
binary image with a disk of radius 64. The expression

```matlab
a = (yy('corner'))*sin((xx('corner'))^2/300)
```

generates a nice test pattern with increasing frequency along the
x-axis, and increasing amplitude along the y-axis. All these functions
have 256x256 pixels as the default output size, and allow as a parameter
either the size of an image, or an image whose size is to be copied. For
example, `a*xx(a)` is an image multiplied by its x-coordinates.

\section sec_dum_dip_image_displaying Displaying `dip_image` objects

When a *MATLAB* command does not end with a semicolon, the display method
is called for the resulting values, if any. This method defaults to
calling the `disp` method, which displays all the values in matrices.
For the `dip_image` objects, the display method has been overloaded to
call `dipshow` instead. `dipshow` displays the image in a figure window
(see \ref sec_dum_functions_dipshow for more information on this
function). Before display, `dipshow` first calls `squeeze` (see
\ref sec_dum_dip_image_dimensions and \ref sec_dum_dip_image_indexing),
meaning that a 4x1x6 image will be displayed as if it were a 4x6 image.

The `disp` method shows only the image size and data type instead. If
you want `display` to call `disp` instead of `dipshow`, you can change
the `'DisplayToFigure'` preference using `dipsetpref` (see
\ref sec_dum_functions_dippref and \ref sec_dum_customizing_dippref).

For images that cannot be displayed by `dipshow`, (e.g. zero-dimensional
and empty images, tensor images, etc.), `display` always calls `disp`.

There exist overloaded methods to query image properties, such as `size`
and `ndims`, and some methods that are specific to `dip_image` objects,
such as `imsize`, `ntensordims` and `datatype`.

\section sec_dum_dip_image_operations Operations on `dip_image` objects

All mathematical operations have been overloaded for the `dip_image`
object. The matrix multiplication (`*`)
does a pixel-by-pixel multiplication, just as the array multiplication
(`.*`) (the difference between these two operators becomes relevant
when we introduce tensor images, see \ref sec_dum_dip_image_tensors).
The same applies to the other matrix operations. Relational
operations return binary images. Binary operations on non-binary images
treat any non-zero value in those images as true and zero as false. For
example, to do a threshold we do not need a special function, since we
have the relational operators:

```matlab
b = a > 100;
```

A double threshold would be (note *MATLAB*'s operator precedence):

```matlab
b = a > 50 & a < 200;
```

When the two images in the operation do not have the same number of
dimensions, images are expanded to match each other. This is called
*singleton expansion*. For example, if image `a` is 10x12x15, and
image `b` is 10x12, then image `b` is expanded along the third dimension
by replication to compute `a+b`, resulting in an image the same size as
`a`. If image `a` is 10x1, and image `b` is 1x12, the result of `a+b` is
10x12. Only dimensions of size one (and non-existing dimensions) will be
expanded. If image `a` is 10x12, and image `b` is 1x6, `a+b` will
produce an error.

A note is required on the data types of the resulting images. The
"higher" data type always determines this result, but we have chosen
never to return an integer type after any arithmetic operation. Thus,
adding two integer images will result in a 4-byte floating-point image;
an 8-byte floating-point (`double`) image is returned only if any of the
two inputs is `double`.

Many of the arithmetic functions have also been defined for objects of
type `dip_image` (see the two tables below for a complete listing).
The basic difference between these and their *MATLAB* counterpart is that by
default they work on the image as a whole, instead of on a per-column basis.
For example, the function `sum` returns a row
vector with the sum over the columns when applied to a numeric matrix,
but returns a single number when applied to an image. An additional argument
can be provided to compute a sum projection. Besides these,
there are some other functions that are only defined for objects of type
`dip_image`. See \ref sec_dum_dip_image_overloaded to learn about these
functions. That section also lists some functions that behave
differently than usual when applied to images.

| Arithmetic functions defined for objects of type `dip_image` (image in, image out) { colspan="6" } ||||||
| ------- | --------- | ---------- | --------- | ---------- | ---------- |
| `abs`   | `acos`    | `and`, `&` | `angle`   | `asin`     | `atan`     |
| `atan2` | `besselj` | `ceil`     | `complex` | `conj`     | `cos`      |
| `erf`   | `exp`     | `fix`      | `floor`   | `hypot`    | `imag`     |
| `log`   | `log10`   | `log2`     | `mod`     | `not`, `~` |  `or`, `|` |
| `phase` | `pow10`   | `pow2`     | `real`    | `round`    | `sign`     |
| `sin`   | `sqrt`    | `tan`      | `xor`     | `-`        | `+`        |
| `*`     | `.*`      | `./`       | `/`       | `\^`       | `.^`       |
| `==`    | `~=`      | `>`        | `>=`      | `<`        | `<=`       |

| Arithmetic functions defined for objects of type `dip_image` (image in, scalar out) { colspan="6" } ||||||
| ------------ | ------ | ----- | ------ | -------- | --------- |
| `all`        | `any`  | `max` | `mean` | `median` | `min`     |
| `percentile` | `prod` | `std` | `sum`  | `var`    |           |

\section sec_dum_dip_image_dimensions Dimensions

*MATLAB* arrays have at least 2 dimensions. This is not true for an image
in a `dip_image` object, which can also have 0 or 1 dimension. That is,
for images there is an explicit distinction between a 2D image of size
256 by 1 pixels, and a 1D image of size 256. Even though both images
have the same number of pixels and their *MATLAB* array representation is
identical, these two images behave differently in many aspects. For
example, the `imsize` method (which is specific to `dip_image` objects,
and returns the image size) will return two numbers for the first image,
but only one for the second; similarly, it will return an empty array for
a 0D image (whereas the corresponding *MATLAB* matrix has a size of 1-by-1).
The method `size` (which is overloaded from the common *MATLAB* function)
behaves more similarly to what you're used to, as it always returns at
least two values. This method is implemented because *MATLAB* requires
it for displaying information with `whos`, we recommend you always use
`imsize` with `dip_image` objects.

Use the function `ndims` to obtain the number of dimensions in an image.

The 2D image in the example above has a singleton dimension. A singleton
dimension is any dimension of size 1. In *MATLAB* arrays, trailing
singleton dimensions are removed if the array has more than two
dimensions. That is, an array of size 4x1x6x1 is silently converted to
an array of size 4x1x6. This never happens with `dip_image` objects.

As in *MATLAB*, operations between two images require that both images
have compatible sizes. Singleton expansion is applied to make the two
images equal in size. Singleton trailing dimensions can be applied to
one of the images before singleton expansion, meaning that the images
do not need to have the same dimensionality.

\section sec_dum_dip_image_indexing Indexing pixels

In image processing, it is conventional to index images starting at
(0,0) in the upper-right corner, and have the first index (usually x),
index into the image horizontally. Unfortunately, *MATLAB* is based on
matrices, which are indexed starting at one, and indicating the row
number first. By encapsulating images in an object, we were allowed to
redefine the indexing. We chose not to follow *MATLAB*'s default indexing
method. This might be confusing at first, and special care must be taken
to check the class of a variable before indexing.

`dip_image` objects are indexed from 0 to `end` in each dimension, the
first being the horizontal. The `imsize` function also returns the image
width as the first number in the array. Any portion of a `dip_image`
object, when extracted, is still a `dip_image` object, and of the same
dimensionality, even if it is just a single pixel. Thus, if `a` is a 3D
`dip_image` object, `a(0,0,0)` is also a 3D `dip_image` object, even
though it only has a single pixel. To get a pixel value as a *MATLAB*
array, use `double(a(0,0,0))`. To remove these singleton dimensions use
`squeeze`. For example, `a(:,:,2)` is a 3D image with a singleton
dimensions, whereas `squeeze(a(:,:,2))` is a 2D image.

Any numeric type can be assigned into a `dip_image` object, without
changing the image data type (that is, the element assigned into the
image is converted to the image data type). For example,

```matlab
b(:,0) = 0;
```

sets the top row of the image in `b` to 0. Note that indexing
expressions can become as complicated as you like. For example, to
sub-sample the image by a factor 3, we could write

```matlab
b = b(1:3:end,1:3:end);
```

Instead of using full indexing (indexing each dimension separately), it
is also possible to index using a single (linear) index. As with standard
*MATLAB* arrays, the indices increase in the vertical direction, which is
how pixels are stored in memory. However they start at 0 for `dip_image`
objects ($i = y + x \cdot \textrm{height}$). The output is always a 1D image.

Finally, it is also possible to index using a mask image. Any binary
image (or logical array) can be used as mask, but it must be of the same
size as the image into which is being indexed. For example,

```matlab
a(m) = 0;
```

sets all pixels in `a`, where `m` is one, to zero. A very common
expression is of the form

```matlab
a(a<0) = 0;
```

(which sets all negative pixels to zero).

Note that the expression `a(m)` above returns a one-dimensional image,
with all pixels selected by the mask. It is equivalent to `a(find(m))`,
where `find` returns an array of indices where `m` is one. This array is
then used as a linear index into `a`.

\section sec_dum_dip_image_tensors Tensor images

Some image data benefits from assigning multiple values to each pixel.
The most common example is a multi-channel image, such as an RGB
color image. The multi-channel image is a form of vector image, where
each pixel is a vector of values. Similarly, you could think of each
pixel being a matrix. This concept can be generalized using tensors.
A tensor can be a scalar value (0-rank tensor), a vector (1-rank tensor),
a matrix (2-rank tensor), etc. In *DIPimage* (and *DIPlib*) we currently
limit the tensor rank to 2, simply because we never came across a use
for higher-rank tensor images.

The function `newtensorim` creates a new tensor image filled with zeros:

```matlab
A = newtensorim([2,2],[256,256])
```

creates a 2-by-2 tensor image of 256 by 256 pixels.

Note that a scalar image (with one component) is also a tensor
image (`istensor` returns true). The function `isscalar` returns true
when there is only one tensor component. Additionally, the function
`isvector` returns true if the tensor has rank 1. Relevant similar
functions are `iscolumn`, `isrow`.

Some arithmetic operations behave differently for non-scalar images than
for scalar images. For example, the `*` operator, which behaves
identically to the `.*` operator for scalar images, actually applies
a matrix multiplication between each corresponding pair of pixels. For
example, the following code applies a matrix multiplication to the
2-vector image `b`, yielding a 2-by-2 matrix image `c`:

```matlab
a = readim('trui');
b = gradient(a)      % yields a 2-vector
c = b * b'           % yields a 2-by-2 matrix
```

The pixels of a tensor image can be indexed like a normal image,
returning a new tensor image. It can also be indexed using curly
braces (`{}`) to select one or more tensor elements (channels). Indexing
into the tensor dimension is identical to indexing into a *MATLAB*
matrix: the first index goes down, and indices start at 0. For
example, `c{1,1}` is a scalar image with the first tensor element of
each matrix in the image `c`.
A single index uses linear indexing: `c{1}` also is the first tensor element.

It is possible to combine spatial and tensor indexing, but the curly
braces have to come first (this is a limitation of the *MATLAB* parser).
Thus, write `c{1}(0,0)`, not `c(0,0){1}`.

!!! warning
    Note that the function `end` only works correctly for spatial indexing
    (within `()`), not for tensor images (within `{}`).

Because of limitations in the *MATLAB* language, it is impossible to know,
for the overloaded `end` method, if it is being used inside curly or
round braces (i.e. whether the last element of the image array is
requested, or the last pixel of the image is requested). The solution we
have adopted is to always assume round braces (`()`). Never use `end`
within curly braces (`{}`). You can use `tensorsize` or `numtensorel`
to compute indices from the end for tensor indexing:

```matlab
a{end};             % doesn't work!
a{numtensorel(a)};  % returns the last tensor component
```

Note here that the image `c` above is a special type of matrix image:
it is symmetric. That is because `c{1,2}` and `c{2,1}` are the result of
the same operation: `b{1}*b{2}`. The `*` operator recognizes that the two
inputs are transposed versions of each other (because they point at the
same data block), and thus knows not to compute the same thing twice.
The image `c` actually only contains 3 tensor elements, even though it
represents a 2-by-2 matrix. Therefore, `c{4}` is an out-of-bounds error,
whereas `c{2,2}` returns the result of `b{2}.^2`, and is identical to
`c{3}`. There are special representations for the column-major matrix
(the default), row-major matrix (obtained by transposing a matrix, which
therefore doesn't need to copy any data), symmetric matrix, and upper
and lower triangular matrices. How the elements are stored is described
in the *DIPlib* API documentation, but if you always access matrix
elements using the two-index form then there is no need to know how
these are stored.

The method `numtensorel` returns the number of tensor elements, and
the method `tensorsize` returns the size of the tensor.

!!! todo
    Document how to set and change the tensor shape.

To get the array at a single pixel, use the `double` function:
`c(0,0)` is a tensor image with a single pixel, and `double(c(0,0))` is
a *MATLAB* array with the tensor values at the first pixel.

Functions defined specifically for tensor images are summarized in the
following table. See \ref sec_dum_dip_image_overloaded.

| Functions defined for tensor images  { colspan="6" }    ||||||
| ------- | ------- | ----- | ------ | ------------ | -------- |
| `cross` | `curl`  | `det` | `diag` | `divergence` | `dot`    |
| `eig`   | `eye`   | `inv` | `norm` | `pinv`       | `rotate` |
| `svd`   | `trace` | `*`   | `.'`   | `'`          |          |

\section sec_dum_dip_image_color Color images

A color image is represented in a `dip_image` object by a tensor image
with some extra information on the color space in which the pixel values
are to be interpreted. A color image must have more than one channel, so
the tensor image that represents it should have at least two components.
Use the `colorspace` function to add this color space information to a
tensor image:

```matlab
C = colorspace(A,'RGB')
```

A color space is any string recognized by the system. See `help colorspace`
for currently known color spaces.
Images with a color space will be displayed by `dipshow`, which will
convert them to RGB for a correct representation.

To convert an image from one color space to another, use the
`colorspace` function. Converting to a color-space-less tensor image is
done by specifying the empty string as a color space. This action only
changes the color space information, and does not change any pixel
values. Thus, to change from one color space to another without
converting the pixel values themselves, change first to a
color-space-less tensor image, and then to the final color space.

The function `joinchannels` combines two or more images into a color
image using the specified color space:

```matlab
C = joinchannels('RGB',a,b,c)
```

The function `newcolorim` creates a new color image of the given
color space, filled with zeros:

```matlab
C = newcolorim([256,256],'RGB');
```

All operations that are defined for tensor images can be applied to
color images. These operations simply ignore the color space. Thus,
adding two images with different color spaces does not cause one to
be converted to the other color space. Typically, the output image
will have the color space of the first input image with a color space
and whose number of tensor elements matches that of the output image.

\section sec_dum_dip_image_shape Manipulating the image shape

Functions used in *MATLAB* to manipulate array dimensions have been
overloaded to do the same thing with images. They are listed in
the table below.

| Dimension manipulation functions defined for objects of type `dip_image` { colspan="6" } ||||||
| --------- | ----------------- | ----------------- | --------- | ---------- | --------- |
| `cat`     | `circshift`       | `expanddim`       | `flipdim` | `fliplr`   | `flipud`  |
| `permute` | `repmat`          | `reshape`         | `rot90`   | `shiftdim` | `squeeze` |
| `swapdim` | `tensortospatial` | `spatialtotensor` |           |            |           |

A few of these functions are unique to `dip_image` objects.
The function `expanddim` adds trailing singleton dimensions, and `swapdim`
is a simpler interface to the more general `permute`, and allows to
swap two image dimensions.

`spatialtotensor` takes a spatial dimension of a scalar image and converts
it to the tensor dimension, returning a vector image. `tensortospatial` does
the reverse, returning a scalar image.
When not specifying which spatial dimension to use, both these functions pick
the dimension that requires no copying of data: 2. If you specify any other
dimension, the data must be copied (and reordered). Thus, by using dimension 2,
it is possible to exchange image shapes very efficiently, for example to
apply functions such as `max` or `sum` along the tensor dimension.

Note that `reshape` and `squeeze` never copy image data (but see
\ref sec_dum_dip_image_reshape), and thus preserve the
linear indexing order (the linear indexing order is related to storage in memory).
Because linear indexing order matches the *MATLAB* storage order, dimension 2
is the most rapidly changing dimension. This means that squeezing an image of
size `[1,10,20,30]` leads to an image of size `[20,10,30]`, not `[10,20,30]`,
as one would expect.

\subsection sec_dum_dip_image_reshape A note on the reshape and squeeze methods

`reshape` and `squeeze` have a different behavior in *DIPimage 3* than they
had in earlier versions of the toolbox. The behavior was changed for consistency,
though the new behavior can be surprising at times.

In older versions of the toolbox, `reshape` and `squeeze` often reordered the
data (i.e. incurred the cost of a data copy), whereas the methods applied to a
normal array never do so, these methods are supposed to be essentially free.
`reshape` was implemented to fill the output image row-wise with pixels taken
row-wise from the input image. But because MATLAB stores matrices column-wise,
the data copy was necessary. However, this behavior was inconsistent with
linear indexing, which wasn't translated to use that same ordering. That is,
linear indexing used the memory order of the pixels to translate an index to
pixel coordinates, in the same way that it works for normal array. Thus,
applying `reshape` (or `squeeze`, which applies a `reshape` to remove singleton
dimensions) would change the pixels accessed at a given linear index, which is
counter-intuitive. For example, in the following program `a` and `b` are
different values:

```matlab
img = dip_image(rand(10,11,8));
a = img(200);
img = reshape(orig,[11,8,10]);
b = img(200);
```

*DIPimage 3* changed this behavior, such that `reshape` and `squeeze` are
essentially free like they are for normal MATLAB arrays. Reshaping or squeezing
an image is consistent with linear indexing (i.e. `a` and `b` above have the
same value). However, this causes a different surprising behavior: `squeeze`
reorders dimensions!

MATLAB's array memory layout is such that a `dip_image`'s dimensions are ordered
in memory like so: [2, 1, 3, 4, ...]. If `squeeze` were to remove dimension number
1, subsequent dimensions would move left, meaning that dimension number 3 ends up
in the location of dimension 1, but 2 stays where it was. An image of size 1x20x30,
when squeezed, becomes an image of size 30x20, not 20x30 as one would expect.
Similarly, removing dimension 2 would move dimension 1 to its place, and dimension
3 to the place of 1. Thus, an image of size 20x1x30 becomes a image of size 30x20,
not 20x30 as one would expect.

Setting the preference \ref sec_dum_customizing_dippref_cheapsqueeze
to `'off'` changes the behavior of `squeeze` to match its old behavior,
possibly incurring a data copy. In essence, when the setting is `'off'`,
then `squeeze` is implemented through `permute`, whereas when it is `'on'`
(the default), it is implemented through `reshape`.

\section sec_dum_dip_image_overloaded Overloaded methods with different behavior

Most overloaded methods behave in a consistent manner with the built-in
*MATLAB* functions that they overload. However, due to differences of
the `dip_image` object, some behave somewhat differently. We summarize
these functions here.

\subsection sec_dum_dip_image_find `find`, `findcoord`

`find` works similarly to the base version, except it is not possible
to obtain `[I,J]` indices as output. The indices returned are always
linear indices. An optional second output argument receives the non-zero
values. To obtain the coordinates of non-zero values, use `findcoord`
instead. It returns the coordinates of the pixels with non-zero values
as a single array, with as many columns as dimensions in the input
image, and one row for every non-zero pixel. Note that this matrix
cannot be used directly to index an image.

\subsection sec_dum_dip_image_gradient `gradient`

The overloaded version of `gradient` returns a vector image, instead of
multiple outputs. The derivatives are computed using Gaussian
derivatives by default.

\subsection sec_dum_dip_image_ind2sub `ind2sub`, `sub2ind`

These functions have the same function as their base counterparts, but
instead of using subscripts specified with one array for each dimension,
they take and return a single coordinate array, compatible to that
returned by `findcoord`. Also, instead of a size array, they take an
image as input.

\subsection sec_dum_dip_image_isscalar `isscalar`, `isvector`, `isrow`, `iscolumn`, `ismatrix`

These functions examine the tensor shape, not the image shape. A scalar
image (it has a single channel) tests true with `isscalar`, no matter
how many spatial dimensions it has.

\subsection sec_dum_dip_image_max `max`, `min`, `mean`, `median`, `std`, `var`, `prod`, `sum`, `all`, `any`

The built-in *MATLAB* versions of these always operate along matrix columns,
yielding a row vector where each element is the max/min/mean/etc. of the
corresponding column. This is a max/min/mean/etc. projection. It is possible
to have them work along a different array dimension, and only since R2018b
along multiple dimensions.

The overloaded versions of these functions that operate on `dip_image`
objects can work along any number of dimensions simultaneously. By default,
they operate on all dimensions, such that `max(a)` returns the maximum
value of the image `a`. And `max(a,[],[2,3])`, if `a` is a 3D image, returns
a 3D image with two singleton dimensions, where each pixel `i` contains the
maximum over `a(i,:,:)`.

Note there is a second argument to `max` that we didn't use above. The
projection functions all take a mask image as an optional second argument.
The projection is taken only over those pixels selected by the mask. For
example,

```matlab
mean(a,a>0,1)
```

computes the mean projection along the first dimension (x axis), but only
computes the mean over the positive pixels.

The function `percentile` is also projection function, but does not have a
counterpart for *MATLAB* arrays (unless you have the statistics toolbox).

\subsection sec_dum_dip_image_ndims `ndims`

This method can return 0 or 1 (for 0D and 1D images, respectively).
For normal *MATLAB* arrays it always returns at least 2. Note that
`ndims(a)` is not necessarily equal to `length(size(a))`, but it is
equal to `length(imsize(a))`.

\subsection sec_dum_dip_image_numel `numel`

The overloaded `numel` is the number of samples in the image. Note that
`prod(size(a))` is not equal to `numel(a)`, as it is for regular arrays.
Instead, the following relations hold:

- `prod(size(a)) == numpixels(a)`

- `prod(tensorsize(a)) == numtensorel(a)`

- `numpixels(a) * numtensorel(a) == numel(a)`

\subsection sec_dum_dip_image_rotate `rotate`

The overloaded method `rotate` has nothing to with *MATLAB*'s `rotate`
(a handle graphics function).
Applied to a 3-vector image, it rotates the vectors around an axis
given by a second vector image or vector.

\section sec_dum_dip_image_review Review of the differences between a `dip_image` and a *MATLAB* array

As we have seen, objects of type `dip_image` have some differences with
respect of regular *MATLAB* arrays. The main difference is in indexing. We
start counting pixels from 0, and the first index counts from left to
right. This ordering is also used by functions such as `imsize`, in which
the first number is the image width and the second one the height.
Finally, `ndims` can return 0 or 1, which it never does for *MATLAB*
arrays. The reason is that zero-dimensional and one-dimensional images are
allowed, and are not seen as a special case of two-dimensional images.
Furthermore, singleton dimensions at the end are not ignored.

Another major difference is that one of the dimensions is not a spatial
dimension, and is not included in the result of `imsize` and `ndims`, or
accessible using normal indexing. This dimension represents a tensor at
each image pixel. `tensorsize` and `numtensorel` are relevant here, as
is indexing using curly braces (`{}`) as in cell array indexing.
Color images are images with multiple tensor elements (i.e. channels),
and color space information.

When a *MATLAB* command results in an object of type `dip_image`, and it
is not ended with a semicolon, the image is displayed to a figure
window, instead of having its pixel values shown in the command window.
This is the default behavior, but can be overridden.

All operators work on a pixel-by-pixel basis. For example, the transpose
operators `'` and `.'` transpose the vector or matrix at each pixel,
not the image itself, and the multiplication operator `*` applies matrix
multiplication to each of the corresponding pixel pairs.
All functions that work on the columns of numeric arrays (such as `sum`
and `max`) work on the image as a whole when applied to a `dip_image` object.

Objects of type `dip_image` cannot be used in functions of the
MathWorks' Image Processing Toolbox. Although most of *MATLAB*'s functions
work on `dip_image` objects, not every function will work as expected.
Use the functions `dip_array`, `double` or `uint8` to convert the image
to a format recognizable by these functions.
