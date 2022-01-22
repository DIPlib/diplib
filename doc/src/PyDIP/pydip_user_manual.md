\comment (c)2017-2021, Cris Luengo.

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


\page pydip_user_manual PyDIP User Manual

Currently, most functionality in the *PyDIP* module is directly mirrored from the
*DIPlib* library. That is, function names and signatures are mostly identical to
those in *DIPlib*. Please see the documentation for *DIPlib* to learn how to use
these functions. Type **T**{ .m-label .m-warning } here to bring up a search
dialog box where you can find functions by name.

To install the package from PyPI, use
```bash
pip install diplib
```
To read images through the Bio-Formats library, you will need to download it separately:
```bash
python -m diplib download_bioformats
```

This user manual discusses the differences with the *DIPlib* library.

\section pum_correspondences Type correspondences

Most classes defined in *DIPlib* and used as input arguments to functions have
a Python binding, with the following exceptions:

- `dip::DataType`: pass a string such as `'UINT8'` or `'SFLOAT'`.

- `dip::Sample`: pass a scalar (a regular Python number).

- `dip::Pixel`: pass a list of scalars.

- `dip::Range`: pass a slice (`slice(0, 3, 1)`). Note that the second argument,
  the end value, is interpreted differently by *DIPlib*: it is included in the range.
  You can also pass a scalar here.

- `dip::UnsignedArray`, `dip::FloatArray`, or similar: pass a Python list: `[5, 5]`.
  A scalar is accepted as a one-element list.

- `dip::StringArray`: pass a list of strings (`['foo','bar']`).

- `dip::StringSet`: pass a dictionary (`{'foo','bar'}`).

By using named arguments, it is quite simple to set only needed arguments, and
leave all others with their default values. All arguments that have a default
value in C++ also have a default value in Python.

\section pum_display Displaying images

The class `dip.Image` has a method `Show()`. There is an identical function
`dip.Show()`. They display an image to the current *matplotlib* window, if
*matplotlib* is installed:

```python
import diplib as dip
img = dip.ImageReadTIFF('cameraman')
img.Show()
```

By default, the image intensities are mapped to the full display range
(i.e. the minimum image intensity is black and the maximum is white). This
can be changed for example as follows:

```python
img.Show('unit')  # maps [0,1] to the display range
img.Show('8bit')  # maps [0,255] to the display range
img.Show('orientation')  # maps [0,pi] to the display range
img.Show('base')  # keeps 0 to the middle grey level, and uses a divergent color map
img.Show('log')  # uses logarithmic mapping
```

Type `help(dip.Show)` in Python to learn about many more options.

If \ref dipviewer is installed, its functionality will be in the `diplib.viewer`
namespace. Use `dip.viewer.Show(img)`. Depending on the backend used, it
will be necessary to do `dip.viewer.Spin()` to interact with the created
window. `Spin()` interrupts the interactive session until all *DIPviewer*
windows have been closed. Even when `Spin()` is not needed to interact
with the windows, it should be run before closing the Python session to
avoid a series of error messages. Alternatively, periodically call
`dip.viewer.Draw()`.
`dip.viewer.Show()` has additional parameters
that can be used to set viewing options; type `help(dip.viewer.Show)` for details.
It also returns an object that can be used for further interaction.

\section pum_indexing Indexing into images

Indexing into a `dip.Image` object works as it does for other array types in
Python:

```python
img[0]
img[0:10]
img[0:-1:2, 0:-1:2]
```

Note that dimensions are ordered in reverse from how *NumPy* stores them
(the first dimension is horizontal, or x).

It is possible to assign to a subset of the image pixels using indexing:

```python
img[0] = 0
img[0:10] = img[20:30]
img[0:-1:2, 0:-1:2] = 255
```

Unlike in *DIPlib*, the square brackets index into spatial dimensions.
To index into tensor dimensions, use round brackets (parenthesis):

```python
img(0)
img(0, 2)
img(slice(0, 3))
```

The output of any of these indexing operations shares data with the original
image, so writing to that output also changes the original image:

```python
img2 = img(0)        # this copy shares data with img
img2.Fill(100)       # same as img(0).Fill(100)
img(1).Copy(img(0))
img(2)[:,:] = img(0)

img2 = img(0).Copy() # this copy does not share data with img
img2.Fill(100)       # does not affect img
```

Irregular indexing using a mask image is also supported. This indexing
returns a copy of the data, but an assignment form is also available:

```python
img2 = img[mask]  # this copy does not share data with img
img2.Fill(0)      # does not affect img
img[mask] = 0     # sets all pixels in mask to 0
```

\section pum_testing Testing image validity

You can use either `IsForged()` or `IsEmpty()` to test if an image is forged.
`IsEmpty()` is the opposite of `IsForged()`, and returns `True` if this image is not forged.

Functions that expect an image interpret `None` as an empty (non-forged) image.

\section pum_numpy Mixing *NumPy* arrays and *DIPlib* images

A NumPy array can be passed instead of an image to any *DIPlib* function. In fact, any Python object
that uses the buffer interface implicitly casts to an image. The reverse is also true: NumPy treats
*DIPlib* images as an array, you can call any *NumPy* function on an image. However, some code that
accepts a *NumPy* array calls methods of the array, which would not be defined for a *DIPlib* image.
For example,

```python
array = np.zeros((10,11))
dip.Gauss(array)          # OK
img = dip.Image((11,10))
np.amax(img)              # OK
img.max()                 # error! np.array method not defined for dip.Image
img.shape                 # error! np.array property not defined for dip.Image
```

One can "cast" from a *NumPy* array to a *DIPlib* image and back:

```python
x = np.array(img)
y = dip.Image(array)
```

The image and the array point to the same memory in these two cases: modifying values in the one cause
the other to see the modified values as well

The mapping from images to arrays causes the indexes to be reversed: The first array index corresponds
to the last image index, and vice versa. If an image is indexed as `img[x,y,z]`, the corresponding
array is indexed as `array[z,y,x]`. 2D *NumPy* arrays are typically interpreted with the first dimension
being vertical (y) and the second horizontal (x). This is how they are printed to the console, and how
`pyplot.imshow` displays them as images. Preserving the indexing order between *DIPlib* and *NumPy* would
therefore cause 2D images to be shown transposed by other Python tools. Furthermore, by reversing the
indexing, we map an image with normal strides to an array in *NumPy*'s standard C-ordering.
The following evaluates to `True`:

```python
dip.Image( np.zeros((10,11,5,7)) ).HasNormalStrides()
```
