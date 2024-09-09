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


\page pum_differences Differences from *DIPlib*

This section has two functions: First, it helps understand how to translate the information
in the function documentation to use those functions in Python. Second, it brings
people up to speed that are familiar with *DIPlib* the C++ library.

To find a function in the documentation, type **T**{ .m-label .m-warning } here (or click the
search icon at the top of the page) to bring up a search dialog box. There you can search for
functions, modules and pages by name. For example, typing "watershed" in the search box
will point you to a series of functions related to the watershed.

A few classes and functions work differently than in C++, and are not explicitly named here.
For these cases, the Python `help()` function should give you sufficient information to understand
the differences. For all other cases, the `help()` function only gives an automatically-generated
function signature, use the on-line documentation to learn more about the function.

\section pum_differences_correspondences Type correspondences

Most classes defined in *DIPlib* and used as input arguments to functions have
a Python binding, with the following exceptions:

- `dip::DataType`: translated to/from a string, such as `'UINT8'` or `'SFLOAT'`.

- `dip::Sample`: a scalar (a regular Python number).

- `dip::Pixel`: a list of scalars.

- `dip::Range`: a slice (`slice(0, 3, 1)`). Note that the second argument,
  the end value, is interpreted differently by *DIPlib*: it is included in the range.
  A scalar can also be translated to a range.

- `dip::UnsignedArray`, `dip::FloatArray`, or similar: a Python list: `[5, 5]`.
  A scalar is accepted as a one-element list.

- `dip::StringArray`: a list of strings (`['foo', 'bar']`).

- `dip::StringSet`: a set of strings (`{'foo', 'bar'}`).

By using named arguments, it is quite simple to set only needed arguments, and
leave all others with their default values. All arguments that have a default
value in C++ have the same default value in Python.


\section pum_differences_indexing Indexing into images

Indexing into a `dip.Image` object uses the same syntax as other array types in
Python:
```python
img[0]
img[0, 10]
img[0:-1:2, 0:-1:2]
```
But the indexing follows *DIPlib*'s rules:
First, dimensions are ordered in reverse from how *NumPy* stores them
(the first dimension is horizontal, or x). Second, the end index is included in the
range. Third, all dimensions must be specified.

In C++ *DIPlib*, square brackets index into the tensor dimensions, not the spatial dimensions.
Use round brackets (parentheses) for this indexing in Python:
```python
img(0)
img(0, 2)
img(slice(0, 3))
```

Indexing operations do not return a special view object, they directly return a `dip.Image` object.
With regular indexing, this new object shares data with the original one.
Irregular indexing using a mask image is also supported. This indexing
returns a copy of the data, but an assignment form is also available:
```python
img2 = img[mask]  # this copy does not share data with img
img2.Fill(0)      # does not affect img
img[mask] = 0     # sets all pixels in mask to 0
```

See \ref pum_indexing for more details.


\section pum_differences_testing Testing image validity

You can use either `IsForged()` or `IsEmpty()` to test if an image is forged.
`IsEmpty()` is the opposite of `IsForged()`, and returns `True` if this image is not forged.

An image in a Boolean context, such as `if image`, has the value of `IsForged()`. This means
that the following two pieces of code are identical:
```python
if image.IsForged():
    print("The image has data")

if image:
    print("The image has data")
```

Functions that expect an image interpret `None` as an empty (non-forged) image.


\section pum_differences_operators Operators applied to images

Most operators have been overloaded to do what they do in C++:

- Arithmetic operators: `+`, `-`, `/`, `%`, and the unary `+` and `-`.
- Bit-wise logical operators: `&`, `|`, `^`.
- Comparison operators: `<`, `<=`, `==`, `>=`, `>` and `!=`.

Operators that work differently in Python and C++:

- Indexing operators: see \ref pum_indexing.
- Multiplication operators: Python has both `*` and `@`. `@` behaves like `*` in C++. `*` is always the
  element-wise multiplication.
- Exponentiation operator: Python has `**`, this doesn't exist in C++.

For operators that have an in-place version (e.g. `+` has a `+=`), we have always overloaded the in-place
version as well.

Not overloaded are the integer division `//`, `del`, and the shift operators `<<` and `>>`. Logical operators
`and`, `or` and `not` cannot be overloaded.

`len()` and `str()` have also been overloaded. `len(image)` returns the number of pixels (i.e. it is the same
as `image.NumberOfPixels()` if the image is forged, or 0 otherwise). `str(image)` returns a string containing
what you'd see if you do `print(image)`.

Note that operator chaining, where `x < y < z` is interpreted as `x < y and y < z`, does not work as expected
with *DIPlib* images (in the same way it doesn't work with *NumPy* arrays). This expression, where `x` or `y`
is an image, will evaluate to `y < z`. This is because `and` here first evaluates its left-hand-side argument
as a boolean expression. If `x` or `y` is an image, this will always evaluate as true, as described
in \ref pum_differences_testing. `and` will then evaluate to its right-hand-side argument, `y < z`.


\section pum_differences_display Displaying images

The class `dip.Image` has a method `Show()`. There is an identical function
`dip.Show()`. They display an image to the current *matplotlib* window, if
*matplotlib* is installed:
```python
import diplib as dip
img = dip.ImageRead('examples/trui.ics')
img.Show()
```

If \ref dipviewer is installed, its functionality will be in the `dip.viewer`
namespace. Use `img.ShowSlice()` for convenience, it calls `dip.viewer.Show()`,
meaning that `dip.viewer.Spin()` might be needed to interact with the created
window.

`dip.Image.ShowSlice()` and `dip.viewer.Show()` have additional parameters
that can be used to set viewing options. They also return an object that can
be used for further interaction:
```python
wdw = img.ShowSlice('Window title', mapping='unit', lut='sequential')
dip.viewer.Spin()
```

See \ref pum_display to learn more.
