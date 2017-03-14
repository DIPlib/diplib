\page concepts Assorted concepts used in DIPlib 3.0.

[//]: # (DIPlib 3.0)

[//]: # ([c]2016-2017, Cris Luengo.)
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

This page describes assorted concepts used in *DIPlib* 3.0.

\tableofcontents

[//]: # (--------------------------------------------------------------)

\section connectivity Connectivity

Traditionally, neighborhood connectivity is given as 4 or 8 in a 2D image, 6, 18 or 26
in a 3D image, etc. These numbers indicate the number of neighbors one obtains when
using the given connectivity. Since this way of indicating connectivity does not naturally
lead to dimensionality-independent code, *DIPlib* uses the distance to the neighbors in
city-block distance instead (the L1 norm). Thus, the connectivity is a number between
1 and *N*, where *N* is the image dimensionality. For example, in a 2D image,
a connectivity of 1 leads to 4 nearest neighbors (the edge neighbors), and a connectivity
of 2 leads to 8 nearest neighbors (the edge and vertex neighbors).

We use negative values for connectivity in some algorithms. These indicate alternating
connectivities, which leads to more isotropic shapes in e.g. the binary dilation than
using the same connectivity for all iterations.

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

[//]: # (--------------------------------------------------------------)

\section structuringelement Structuring elements

Many functions in the Mathematical Morphology module require a structuring element definition.
These functions come in two flavors: one where the user can specify the shape name and the size
of a structuring element, and one where the user can pass an image containing the structuring
element.

With the first function signature, the relevant parameters are `filterShape` and `filterParam`.
These are the valid shapes for `filterParam`, and the corresponding meaning of `filterShape`:

 -  "elliptic", "rectangular", "diamond": these are unit circles in different metrics. The
    `filterParam` corresponds to the diameter in that metric. "elliptic" is the default shape,
    because it is isotropic, but also the slowest to compute. Both "elliptic" and "diamond"
    structuring elements always are symmetric. That is, their origin is centered on a pixel.
    The pixels included in the shape are those at most half of `filterParam` away from the origin.
    For the "rectangular" structuring element, a box with integer sizes is always generated,
    but the box can be even in size also, meaning that the origin is in between pixels.
    Any `filterParam` that is smaller or equal to 1 causes that dimension to not be processed.

 -  "parabolic": the parabolic structuring element is the morphological equivalent to the Gaussian
    kernel in linear filtering. It is separable and perfectly isotropic. `filterParam` corresponds
    to the scaling of the parabola (i.e. the *a* in *a<sup>-2</sup> x<sup>2</sup>*). A value equal
    or smaller to 0 causes that dimension to not be processed. The boundary condition is ignored
    for this mode of operation. The output image is always floating-point type.

 -  "interpolated line", "discrete line": these are straight lines, using different implementations.
    The `filterParam` corresponds to the size of the bounding box of the line, with signs indicating
    the direction. Thus, if `filterParam` is `[2,2]`, the line goes right and down two pixels,
    meaning that the line is formed by two pixels at an angle of 45 degrees down. If `filterParam`
    is `[-2,2]`, then the line is again two pixels, but at an angle of 125 degrees. (Note that
    in images, angles increase clockwise from the x-axis, as the y-axis is inverted).

With the second function signature, an input image `se` gives the structuring element. If `se`
is binary, the set pixels form the structuring element. If `se` is a grey-value image, those
grey values are directly used as structuring element values. Set pixels to negative infinity to
exclude them from the structuring element (the result would be the same by setting them to
a value lower than the range of the input image, but the algorithm should be more efficient if
those pixels are excluded).

As elsewhere, the origin of `se` is in the middle of the image, on the pixel to the right of
the center in case of an even-sized image.
