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

