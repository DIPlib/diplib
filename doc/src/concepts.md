\comment DIPlib 3.0

\comment (c)2016-2020, Cris Luengo.
\comment Based on original DIPlib code: (c)1995-2014, Delft University of Technology.

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


\page concepts Assorted concepts used in *DIPlib 3*

This page describes assorted concepts used in *DIPlib 3*.


\comment --------------------------------------------------------------

\section connectivity Connectivity

Traditionally, neighborhood connectivity is given as 4 or 8 in a 2D image, 6, 18 or 26
in a 3D image, etc. These numbers indicate the number of neighbors one obtains when
using the given connectivity. Since this way of indicating connectivity does not naturally
lead to dimensionality-independent code, *DIPlib* uses the distance to the neighbors in
city-block distance instead (the L1 norm). Thus, the connectivity is a number between
1 and *N*, where *N* is the image dimensionality. For example, in a 2D image,
a connectivity of 1 leads to 4 nearest neighbors (the edge neighbors), and a connectivity
of 2 leads to 8 nearest neighbors (the edge and vertex neighbors).

We use negative values for connectivity in some algorithms, in e.g. the binary dilation.
These indicate alternating connectivities, which leads to more isotropic shapes than
using the same connectivity for all iterations. These alternating connectivities are
available only if the function takes a \ref dip::sint as connectivity parameter.

In terms of the classical connectivity denominations we have, in 2D:

Connectivity | Classical denominations | Structuring element shape
------------ | ----------------------- | -------------------------
1            | 4 connectivity          | diamond
2            | 8 connectivity          | square
-1           | 4-8 connectivity        | octagon
-2           | 8-4 connectivity        | octagon

And in 3D:

Connectivity | Classical denominations | Structuring element shape
------------ | ----------------------- | -------------------------
1            | 6 connectivity          | octahedron
2            | 18 connectivity         | cuboctahedron
3            | 26 connectivity         | cube
-1           | 6-26 connectivity       | small rhombicuboctahedron
-3           | 26-6 connectivity       | small rhombicuboctahedron

Some functions will interpret a connectivity of 0 to mean the maximum connectivity
(i.e. equal to the image dimensionality). This is an easy way to define a default
value that changes depending on the image dimensionality.


\comment --------------------------------------------------------------

\section aliasing Handling input and output images that alias each other

Many of the old *DIPlib 2* functions (the ones that cannot work
in-place) used a function `dip_ImagesSeparate()` to create temporary images
when output images are also input images. The resource handler takes
care of moving the data blocks from the temporary images to the output
images when the function ends. With the current design of shared pointers
to the data, this is no longer necessary. Say a function is called with

```cpp
dip::Image A;
dip::Filter( A, A, params );
```

Then the function `dip::Filter()` does this:

```cpp
void dip::Filter( const dip::Image &in_c, dip::Image &out, ... ) {
   Image in = in_c.QuickCopy();
   out.Strip();
   // do more processing ...
}
```

What happens here is that the new image `in` is a copy of the input image, `A`,
pointing at the same data segment. The image `out` is a reference to image `A`.
When we strip `A`, the new image `in` still points at the original data segment,
which will not be freed until `in` goes out of scope. Thus, the copy `in`
preserves the original image data, leaving the output image, actually the
image `A` in the caller's space, available for modifying.

However, if `out` is not stripped, and data is written into it, then `in` is
changed during processing. So if the function cannot work in place, it should
always test for aliasing of image data, and strip/forge the output image if
necessary:

```cpp
void dip::Filter( const dip::Image &in_c, dip::Image &out, ... ) {
   Image in = in_c.QuickCopy();
   if( in.Aliases( out )) {
      out.Strip();       // Force out to not point at data we still need
   }
   out.ReForge( ... );   // create new data segment for output
   // do more processing ...
}
```

Note that the \ref dip::Framework functions take care of this.


\comment --------------------------------------------------------------

\section coordinates_origin Coordinate system origin

Some functions, such as \ref dip::FourierTransform, \ref dip::Rotation and
\ref dip::AffineTransform, use a coordinate system where the origin is a pixel
in the middle of the image. The indices of this pixel are given by
`index[ ii ] = img.Size( ii ) / 2`. This pixel is exactly in the middle of the
image for odd-sized images, and to the right of the exact middle for
even-sized images.

The function \ref dip::FillCoordinates and related functions can be used to
obtain the coordinates for each pixel. These all have a `mode` parameter
that determines which coordinate system to use. The value `"right"` (the
default) places the origin in the same location as \ref dip::FourierTransform,
\ref dip::Rotation, etc.

The function \ref dip::Image::GetCenter (using the default value for its input
argument) returns the coordinates of the central pixel as a floating-point array.


\comment --------------------------------------------------------------

\section normal_strides Normal strides

As discussed in \ref strides, images in *DIPlib* can be stored in different orders.
The stride array specifies how many samples to skip to find the neighboring pixel
along each image dimension. Likewise, the tensor stride indicates how many samples
to skip to find the value for the next channel of the current pixel. When an image
is first forged, the sample ordering is *normal*, unless an external interface
is set (see \ref external_interface). Normal strides are defined as follows:

1. The tensor stride is set to 1. That is, image channels are interleaved.
2. The first dimension's stride is set to the number of tensor elements. That is,
   pixels are stored consecutively in memory along the first dimension (x).
3. Other image dimension's strides are set to the previous dimension's
   stride times the previous dimension's size. That is, image rows are stored
   consecutively without padding, as are image planes, etc.
