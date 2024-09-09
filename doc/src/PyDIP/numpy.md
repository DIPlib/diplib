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


\page pum_numpy Combining *PyDIP* with *NumPy*

The `dip.Image` object uses the Python buffer protocol, which makes it very easy to mix
*PyDIP* with *NumPy* and other image processing packages such as *scikit-image* or *OpenCV*,
which use *NumPy* arrays to represent their images.


\section pum_numpy_numpy Images and *NumPy* arrays

A *NumPy* array can be passed instead of an image to any *DIPlib* function. In fact, any Python object
that uses the buffer interface implicitly casts to an image. The reverse is also true: *NumPy* treats
*DIPlib* images as an array, you can call any *NumPy* function on an image. However, some code that
accepts a *NumPy* array calls methods of the array, which would not be defined for a *DIPlib* image.
For example,
```python
arr = np.zeros((10, 11))
dip.Gauss(arr)          # OK
img = dip.Image((11, 10))
np.amax(img)              # OK
img.max()                 # error! np.array method not defined for dip.Image
img.shape                 # error! np.array property not defined for dip.Image
```

One can "cast" from a *NumPy* array to a *DIPlib* image and back, without copying the data:
```python
x = np.asarray(img)
y = dip.Image(arr)
```
The image and the array point to the same memory in these two cases: modifying values in the one cause
the other to see the modified values as well.

Note that we used `np.asarray()`, not `np.array()`. The latter would have copied the data, unless the
`copy` argument is set to `False`: `np.array(img, copy=False)` is the same as `np.asarray(img)`.


\section pum_numpy_dimension_ordering Dimension ordering

But do note the different interpretation of dimensions and ranges in *NumPy* and *PyDIP*, see \ref pum_indexing:

1. *DIPlib* reverses the dimensions from how they are treated by *NumPy*, with the first index being the *x*
    coordinate, the second *y*, etc. In *NumPy*, and consequently in libraries such as *scikit-image* or *OpenCV*,
    the *x* coordinate is always the last index.
2. Ranges are always inclusive: `img[0:10]` represents 11 samples in *DIPlib*, not 10 as in *NumPy*,
   because `img[10]` is part of the range.

Thus, the following indexing operations are identical:
```python
arr = np.zeros((10, 11, 12))
img = dip.Image(arr)
arr[1, 2, 3] == img[3, 2, 1]
```

By reversing the indexing, we map an image with normal strides to an array in *NumPy*'s standard C-ordering.
The following evaluates to `True`:
```python
dip.Image( np.zeros((10,11,5,7)) ).HasNormalStrides()
```

When using a *NumPy* array as an image in a *DIPlib* function, it is implicitly cast to a `dip.Image`
object as above, and passed to the *DIPlib* function. This means that, whether the input is a *NumPy*
array or a *DIPlib* image, other function parameters that identify dimensions are always interpreted
in the same way. For example, the filter sizes are ordered (x, y, z), not (z, y, x) as they would be
ordered in *scikit-image* or other Python imaging libraries.

By calling `dip.ReverseDimensions()` (which one should do only directly after loading the `diplib`
module to avoid confusing results), *PyDIP* is configured to reverse dimensions of all *DIPlib* images.
This means that the *NumPy* indexing order will be preserved, images will be indexed as `img[z,y,x]`.
This has several surprising results, for example the direction of all angles is reversed, with
positive angles being counter-clockwise instead of clockwise. This option is intended to make it
easier to mix *DIPlib* functions into code that also uses e.g. *scikit-image*.


\section pum_numpy_tensor_dimension The tensor dimension

When casting a tensor image to a *NumPy* array, the tensor dimension will become the last array dimension.
When casting a *NumPy* array to a *DIPlib* image, there is no information about which dimension, if
any, is the tensor dimension. By default, the following heuristic is used: if the array has more than
two dimensions, and if the smaller of the last or the first array dimension has no more than 4 elements,
then that dimension will be the tensor dimension. The tensor will have a column vector shape (this is the
default tensor shape in *DIPlib*). The threshold of 4 was picked because it will handle correctly all color
images. This threshold can be adjusted using `dip.SetTensorConversionThreshold()`. If set to 0, all arrays
will be converted to a scalar image.

For example, here `img` is a 2D image of 11x10 pixels, and three samples per pixel:
```python
arr = np.zeros((10, 11, 3))
img = dip.Image(arr)
```

The `dip.Image()` constructor with a *NumPy* array as input takes an optional argument that determines
which axis, if any, is the tensor dimension. Add `None` to force a scalar output image:
```python
img2 = dip.Image(arr, None)
img3 = dip.Image(arr, tensor_axis=0)
```
Here, `img2` is a 3D scalar image, and `img3` is a 3D image with 3x11 pixels, and 10 samples per pixel.

If the *NumPy* array represents a color image, you will have to explicitly set this information in
the new `dip.Image` object:
```python
img.SetColorSpace('sRGB')
```


\section pum_numpy_function  Calling PyDIP functions with an array as output

The `out` keyword argument to *PyDIP* functions is easier to use with a `dip.Image` object than with a `np.array`
object. When passing in an image object, the image can be reforged (meaning its data segment can be reallocated,
changing the size, number of tensor elements, and/or data type). The same is not true for an input of a different
type, which will be converted to a protected `dip.Image` object (see \ref protect).
This means that it must have the right sizes to receive the output of the function.
```python
arr = np.zeros(10)
dip.Gauss(img, out=arr)  # error! the output has the wrong sizes, and cannot be reforged
```
This will raise a `dip.ParameterError`: "Image is protected". We need to make the array have the right sizes
for this to work:
```python
arr = np.zeros((img.Size(1), img.Size(0), img.TensorElements()))
dip.Gauss(img, out=arr)
```
Note that the filter is computed in the output type, 64-bit float in this case.

This is most useful to work in-place, where the feature is easy to use:
```python
dip.Gauss(arr, out=arr)
```
