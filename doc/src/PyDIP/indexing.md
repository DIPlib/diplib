\comment (c)2017-2024, Cris Luengo.

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


\page pum_indexing Indexing

Indexing into a `dip.Image` object uses the same syntax as other array types in
Python:
```python
img[0]
img[0, 10]
img[0:-1:2, 0:-1:2]
```
But the indexing follows *DIPlib*'s rules:

1. Dimensions are ordered in reverse from how *NumPy* stores them (the first dimension is horizontal, or x).
2. The end index is included in the range.
3. You must provide one range (slice) per dimension, you cannot index only into a subset of dimensions.

The only exception to the 3rd point is that you can index a single pixel using linear indexing (i.e. a single
index), which will be interpreted as counting along rows. For example, for a 3D image, coordinates `[x, y, z]`
cooresponds to linear index `(z * img.Size(1) + y) * img.Size(0) + x`.

It is possible to assign to a subset of the image pixels using indexing:
```python
img[0] = 0
img[0, 10] = img[20, 30]
img[0:-1:2, 0:-1:2] = 255
```

Unlike in *DIPlib*, the square brackets index into spatial dimensions.
To index into tensor dimensions, use round brackets (parentheses):
```python
img(0)
img(0, 2)
img(slice(0, 3))
```

When the indexing operation uses ranges, or indexes into the tensor dimension, then the output is
a new `dip.Image` object that shares data with the original image object. This means that writing
to that output also changes the original image:
```python
img2 = img(0)        # this copy shares data with img
img2.Fill(100)       # same as img(0).Fill(100)
img(1).Copy(img(0))
img(2)[:,:] = img(0) # note that img(2) = img(0) is not allowed by Python

img2 = img(0).Copy() # this copy does not share data with img
img2.Fill(100)       # does not affect img
```

Indexing operations that use only single indices along each dimension return a pixel. This is represented
as a list with the pixel values (even for scalar images). This list does not share data with the image.


\section pum_indexing_irregular Irregular indexing

There are also indexing operations that select an arbitrary (irregular) subset of pixels. These indexing
operations return a new `dip.Image` object, which is always 1D.
This new object does not share data with the original image, the pixel values are copied over.
But of course there is also an assignment version of these indexing operations.

There are two irregular indexing methods: a mask image and a list of coordinates.

```python
# Mask image indexing
mask = img > 100
img2 = img[mask]  # this copy does not share data with img
img2.Fill(0)      # does not affect img
img[mask] = 0     # sets pixels selected by mask to 0

# Coordinate list indexing
list = [
    (0, 0),
    (0, 1),
    (0, 5),
    (10, 3),
]
img3 = img[list]  # also does not share data with img
img[list] = 0     # sets selected pixels to 0
```

For the mask indexing, the order of the pixels in the 1D output image depends on the internal linear pixel
storage order, which is not consistent with any specific (row-major or column-major) order due to operations
such as `Rotation90()` and `Mirror()`. For the coordinate list indexing, the output order always matches
the coordinate list order.
