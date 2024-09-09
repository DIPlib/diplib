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


\page pum_display Displaying Images

\section pum_display_pyplot Image display using *matplotlib*

The class `dip.Image` has a method `Show()`. There is an identical function
`dip.Show()`. They display an image to the current *matplotlib* window, if
*matplotlib* is installed:
```python
import diplib as dip
img = dip.ImageRead('examples/trui.ics')
img.Show()
```

This function is useful to show a 1D or a 2D image. For higher-dimensional images,
a section or projection is shown.

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

Type `help(dip.Show)` in Python to learn about many more options, including setting
the color map, defining how complex data is mapped to grayscale, and how higher-dimensional
images are mapped to the 2D display.


\section pum_display_dipviewer Image display using *DIPviewer*

If \ref dipviewer is installed (which will always be the case for the official *PyDIP*
installation on PyPI), its functionality will be in the `dip.viewer` namespace.
`img.ShowSlice()` is the same as `dip.viewer.Show(img)`. This interactive tool can display
images of any type and with any number of dimensions.

Depending on the backend used, it  will be necessary to do `dip.viewer.Spin()` to interact
with the created window. `Spin()` interrupts the interactive session until all *DIPviewer*
windows have been closed. Even when `Spin()` is not needed to interact with the windows,
it should be run before closing the Python session.
Alternatively, periodically call `dip.viewer.Draw()`.

`dip.Image.ShowSlice()` and `dip.viewer.Show()` have additional parameters
that can be used to set viewing options. They also return an object that can be used for
further interaction:
```python
wdw = img.ShowSlice('Window title', mapping='unit', lut='sequential')
img2.ShowSlice('Second image', link=wdw)
dip.viewer.Spin()
```

Type `help(dip.viewer.Show)` for details.

The function `dip.viewer.ShowModal()` is a convenience function that calls `dip.viewer.Show()`
and `dip.viewer.Spin()`. Use it to display a single image.
