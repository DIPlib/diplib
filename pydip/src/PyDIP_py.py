# PyDIP 3.0, Python bindings for DIPlib 3.0
#
# (c)2017-2019, Flagship Biosciences, Inc., written by Cris Luengo.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
The portion of the PyDIP module that contains Python code.
"""

from .PyDIP_bin import ImageDisplay
import importlib.util

hasMatPlotLib = True
reportedPlotLib = False
if importlib.util.find_spec('matplotlib') is None:
    hasMatPlotLib = False
else:
    import matplotlib
    import matplotlib.pyplot as pp
    import numpy as np


# Label color map from the function of the same name in DIPimage:
def _label_colormap():
    if hasMatPlotLib:
        cm = np.array([
            [1.0000, 0.0000, 0.0000],
            [0.0000, 1.0000, 0.0000],
            [0.0000, 0.0000, 1.0000],
            [1.0000, 1.0000, 0.0000],
            [0.0000, 1.0000, 1.0000],
            [1.0000, 0.0000, 1.0000],
            [1.0000, 0.3333, 0.0000],
            [0.6667, 1.0000, 0.0000],
            [0.0000, 0.6667, 1.0000],
            [0.3333, 0.0000, 1.0000],
            [1.0000, 0.0000, 0.6667],
            [1.0000, 0.6667, 0.0000],
            [0.0000, 1.0000, 0.5000],
            [0.0000, 0.3333, 1.0000],
            [0.6667, 0.0000, 1.0000],
            [1.0000, 0.0000, 0.3333],
        ])
        n = len(cm)
        index = list(i % n for i in range(0, 255))
        cm = np.concatenate((np.array([[0, 0, 0]]), cm[index]))
        return matplotlib.colors.ListedColormap(cm)
    return None


def Show(img, range=(), complexMode='abs', projectionMode='mean', coordinates=(), dim1=0, dim2=1, colormap=''):
    """Show an image in the current pyplot window

    Keyword arguments:
    range -- a 2-tuple indicating the range of input values to map to the
        output range, or a string indicating how to compute the range and
        how to map. Valid strings are:
        - `'unit'`: use the `(0,1)` range.
        - `'8bit'` or `'normal'`: use the `(0,255)` range.
        - `'12bit'`: use the `(0,2**12)` range.
        - `'16bit'`: use the `(0,2**16)` range.
        - `'s8bit'`: use the `(-128,127)` range.
        - `'s12bit'`: use the `(-2**11,12**11-1)` range.
        - `'s16bit'`: use the `(-2**15,12**15-1)` range.
        - `'angle'`: use the `(0,2*pi)` range, with folding of out-of-
            range values by modulo operation. Additionally, it sets the
            color map such that 0 and 2*pi are shown in the same color.
        - `'orientation'`: use the `(0,pi)` range, with folding of out-of-
            range values by modulo operation. Additionally, it sets the
            color map such that 0 and pi are shown in the same color.
        - `'lin'` or `'all'`: use the range from lowest to highest value in
            `img`. This is the default.
        - `'percentile'`: use the range from 5th to 95th percentile value
            in `img`.
        - `'base'` or `'based'`: like 'lin', but setting the value of 0 to
            the middle of the output range. Additionally, it sets the color
            map to `'coolwarm'`, such that negative and positive values
            have blue and red colors, respectively, and 0 is a neutral
            grey.
        - `'log'`: use a logarithmic mapping.
        - `'modulo'` or `'labels'`: use the `(0,255)` range, with folding
            of out-of-range values by modulo operation. Additionally, it
            sets the color map such that nearby values get very different
            colors. This mode is suitable for labeled images.
    complexMode -- a string indicating how to convert complex values to
        real values for display. One of `'abs'` or `'magnitude'`,
        `'phase'`, `'real'`, `'imag'`. The default is `'abs'`.
    projectionMode -- a string indicating how to extract a 2D slice from a
        multi-dimensional image for display. One of `'slice'`, `'max'`,
        `'mean'`. The default is `'mean'`.
    coordinates -- Coordinates of a pixel to be shown, as a tuple with as
        many elements as image dimensions. Determines which slice is shown
        out of a multi-dimensional image.
    dim1 -- Image dimension to be shown along x-axis of display.
    dim2 -- Image dimension to be shown along y-axis of display.
    colormap -- Name of a color map to use for display.

    For images with more than 2 dimensions, a slice is extracted for
    display. The direction of the slice is determined using the `dim1` and
    `dim2` parameters, and the location using the `coordinates` parameter.
    If `projectionMode` is `'slice'`, then the single slice is shown. If
    `projectionMode` is `'max'` or `'mean'`, then a projection is computed
    across the full image volume along the non-displayed dimensions.

    For 1D images, or if `dim1==dim2`, a line is plotted. In this case, the
    `colormap` is ignored. Note that, if `dim1==dim2`, a 2D image is also
    projected as described above for higher-dimensional images.
    """
    global reportedPlotLib
    if hasMatPlotLib:
        out = ImageDisplay(img, range, complexMode=complexMode, projectionMode=projectionMode, coordinates=coordinates, dim1=dim1, dim2=dim2)
        if out.Dimensionality() == 1:
            axes = pp.gca()
            axes.clear()
            axes.plot(out)
            axes.set_ylim((0, 255))
            axes.set_xlim((0, out.Size(0) - 1))
        else:
            if colormap == '':
                if range == 'base' or range == 'based':
                    colormap = 'coolwarm'
                elif range == 'modulo' or range == 'labels':
                    colormap = 'labels'
                elif range == 'angle' or range == 'orientation':
                    colormap = 'hsv'
                else:
                    colormap = 'gray'
            if colormap == 'labels':
                cmap = _label_colormap()
            else:
                cmap = pp.get_cmap(colormap)
            pp.imshow(out, cmap=cmap, norm=matplotlib.colors.NoNorm(), interpolation='none')
        pp.draw()
        pp.pause(0.001)
    elif not reportedPlotLib:
        print("""
    PyDIP requires matplotlib for its display functionality. Matplotlib was not found
    on your system. Image display (diplib.Show and diplib.Image.Show) will not do anything.
    You can install matplotlib by typing on your Linux/MacOS command prompt:
        pip3 install matplotlib
    or under Windows:
        python3 -m pip install matplotlib
    Alternatively, use diplib.viewer.ShowModal or diplib.viewer.Show/diplib.viewer.Spin
    """)
        reportedPlotLib = True
