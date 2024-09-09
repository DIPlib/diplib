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


\page pum_dipjavaio *DIPjavaio*, or how to use *Bio-Formats*

When using an installation of *DIPlib* that has *DIPjavaio* (the installation from PyPI does), and *Bio-Formats*
has been downloaded according to \ref pum_installation, then one can load images from a file
in any of the 160 or so formats currently supported by *Bio-Formats*.

The function `dip.ImageRead()` will, in this case, use *Bio-Formats* if it doesn't recognize the file type
(currently this is for any file that is not ICS, TIFF, PNG, JPEG or NPY). Adding `format="bioformats"` as an
argument will cause *Bio-Formats* to be used even for these known file types.

`dip.ImageRead()` is a simple interface, it just reads the first image seen. *DIPlib* has specialized functions
for each file type, which allow for specifying how an image is to be read. For the *Bio-Formats* "format",
this function is `dip.javaio.ImageReadJavaIO()`. It has a parameter `interface`, which defaults to our
*Bio-Formats* interface. In principle other parameters are possible here, but no other interfaces currently exist.
The other parameter is `imageNumber`, which specifies which image from a multi-image file format to read.
This is what *Bio-Formats* refers to as the "series".

Note that *Bio-Formats* doesn't always have the same interpretation of a file as *DIPlib*.
For example, a multi-page TIFF file is always seen as multiple images by *DIPlib*.
`dip.ImageReadTIFF("file", imageNumbers=slice(0, 5))` will read the first 5 images (pages) and stack them
along the 3rd dimension (assuming they're of the same size). In contrast, *Bio-Formats* will read all the
pages as a single 3D image. Only if the images have different sizes will it see them as individual images.

To use the `dip.javaio.ImageReadJavaIO()` function, `dip.javaio` must first be imported. This doesn't happen
automatically when importing the *DIPlib* package because it takes a bit of time and it is not always needed.
`dip.ImageRead()` will import `dip.javaio` when first called. Otherwise, one can explicitly import it via
```python
import diplib as dip
import diplib.javaio
```
Note that we cannot `import dip.javaio`, as `dip` is an alias and the `import` statement does not resolve it.
But after importing we can refer to `dip.javaio`.
As an alternative, one can, for example,
```python
import diplib.javaio as djio
```
and then use `djio.ImageReadJavaIO()`.
