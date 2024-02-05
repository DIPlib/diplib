# (c)2017-2019, Flagship Biosciences, Inc., written by Cris Luengo.
# (c)2022, Cris Luengo.
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

from .PyDIP_bin import Image, ImageDisplay, ApplyColorMap
import importlib.util
import warnings

hasMatPlotLib = importlib.util.find_spec('matplotlib')
_reportedPlotLib = False


def Show(img, range=(), complexMode='abs', projectionMode='mean', coordinates=(),
         dim1=0, dim2=1, colormap='', extent=None):
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
            range values by modulo operation. Additionally, it applies the
            `'cyclic'` color map.
        - `'orientation'`: use the `(0,pi)` range, with folding of out-of-
            range values by modulo operation. Additionally, it applies the
            `'cyclic'` color map.
        - `'lin'` or `'all'`: use the range from lowest to highest value in
            `img`. This is the default.
        - `'percentile'`: use the range from 5th to 95th percentile value
            in `img`.
        - `'base'` or `'based'`: like 'lin', but setting the value of 0 to
            the middle of the output range. Additionally, it applies the
            `'diverging'` color map.
        - `'log'`: use a logarithmic mapping.
        - `'modulo'` or `'labels'`: use the `(0,255)` range, with folding
            of out-of-range values by modulo operation. Additionally, it
            applies the `'label'` color map.
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
    colormap -- Name of a color map to use for display. If it is one of
        `'grey'`, `'saturation'`, `'linear'`, `'diverging'`, `'cyclic'` or
        `'label'`, then the DIPlib color map of that name is retrieved
        through `dip.ApplyColorMap`. Otherwise, it is the name of a color
        map from Matplotlib.
    extent -- Tuple of floats, (left, right, top, bottom), indicating the
        centroids of the top-left and bottom-right pixel centers. The first
        two values correspond to `dim1`, the last two correspond to `dim2`.
        If `None`, assumes pixel centers at integer coordinates starting
        at 0. For a 1D image, this should be just (left, right).

    For images with more than 2 dimensions, a slice is extracted for
    display. The direction of the slice is determined using the `dim1` and
    `dim2` parameters, and the location using the `coordinates` parameter.
    If `projectionMode` is `'slice'`, then the single slice is shown. If
    `projectionMode` is `'max'` or `'mean'`, then a projection is computed
    across the full image volume along the non-displayed dimensions.

    For 1D images, a line is plotted. In this case, the `colormap` is
    ignored.

    `dim1` and `dim2`, if given, must be distinct.
    If `diplib.ReverseDimensions()` has been used, then `dim1` refers to
    the y axis and `dim2` refers to the x axis, but the meaning of `extent`
    doesn't change, meaning that the first two values continue referring
    to the x axis.

    Note that the 2D display, the value shown for the pixel under the
    cursor corresponds to the value after mapping to the 0-255 display
    range, and not to the actual pixel value. Use
    `diplib.viewer.ShowModal()`, or `diplib.Image.ShowSlice()` and
    `diplib.viewer.Spin()`, for a more useful interactive image display.
    """
    global _reportedPlotLib
    if not hasMatPlotLib:
        if not _reportedPlotLib:
            warnings.warn(
    """PyDIP requires matplotlib for its display functionality. Matplotlib was not found
    on your system. Image display (`diplib.Show()` and `diplib.Image.Show()`) will not do
    anything. You can install matplotlib by typing on your Linux/MacOS command prompt:
        pip3 install matplotlib
    or under Windows:
        python3 -m pip install matplotlib
    Alternatively, use `diplib.viewer.ShowModal()`, or `diplib.Image.ShowSlice()` and
    `diplib.viewer.Spin()`.
    """, RuntimeWarning)
            _reportedPlotLib = True
        return

    if dim1 == dim2:
        # Note that we could handle this case, but we choose not to, it complicates things a bit
        raise RuntimeError("dim1 and dim2 should be distinct")

    import matplotlib
    import matplotlib.pyplot as pp
    import numpy as np

    data = np.asarray(img)
    if data.size <= 1:
        warnings.warn("Nothing to display", SyntaxWarning)
        return
    sizes = [x for x in data.shape if x > 1]
    if len(sizes) == 1:
        data = np.squeeze(data)
        length = sizes[0]
        if np.iscomplexobj(data):
            if complexMode == 'abs' or complexMode == 'magnitude':
                data = np.abs(data)
            elif complexMode == 'phase':
                data = np.angle(data)
            elif complexMode == 'real':
                data = data.real
            elif complexMode == 'imag':
                data = data.imag
        x = np.arange(0.0, length)
        if extent:
            if len(extent) == 2:
                d = (extent[1] - extent[0]) / (length - 1)
                x *= d
                x += extent[0]
            else:
                warnings.warn("Parameter 'extent' has the wrong number of values, ignoring", SyntaxWarning)
        axes = pp.gca()
        axes.clear()
        axes.set_aspect('auto')
        axes.plot(x, data)
        axes.set_xlim((x[0], x[-1]))
        axes.set_ylim((np.amin(data), np.amax(data)))
    else:
        out = ImageDisplay(img, range, complexMode=complexMode, projectionMode=projectionMode, coordinates=coordinates, dim1=dim1, dim2=dim2)
        out = np.asarray(out)
        if colormap == '':
            if range == 'base' or range == 'based':
                colormap = 'diverging'
            elif range == 'modulo' or range == 'labels':
                colormap = 'label'
            elif range == 'angle' or range == 'orientation':
                colormap = 'cyclic'
            else:
                colormap = 'grey'
        if colormap in {'grey', 'saturation', 'linear', 'diverging', 'cyclic', 'label'}:
            cmap = np.asarray(ApplyColorMap(np.arange(256), colormap)) / 255
            cmap = matplotlib.colors.ListedColormap(cmap)
        else:
            cmap = pp.get_cmap(colormap)
        if extent:
            if len(extent) == 4:
                dx = (extent[1] - extent[0]) / (out.shape[1] - 1) / 2
                dy = (extent[3] - extent[2]) / (out.shape[0] - 1) / 2
                extent = (extent[0] - dx, extent[1] + dx, extent[3] + dy, extent[2] - dy)
            else:
                warnings.warn("Parameter 'extent' has the wrong number of values, ignoring", SyntaxWarning)
                extent = None
        axes = pp.gca()
        axes.clear()
        axes.imshow(out, cmap=cmap, norm=matplotlib.colors.NoNorm(), interpolation='none', extent=extent)
        if extent:
            axes.set_aspect(dx / dy)
    pp.draw()
    pp.pause(0.001)


def HistogramShow(hist, range=(), complexMode='abs', projectionMode='mean', coordinates=(), dim1=0, dim2=1, colormap=''):
    if hist.Dimensionality() == 1:
        extent = (hist.BinCenter(0), hist.BinCenter(hist.Bins() - 1))
    else:
        extent = (hist.BinCenter(0, dim1), hist.BinCenter(hist.Bins() - 1, dim1),
                  hist.BinCenter(0, dim2), hist.BinCenter(hist.Bins() - 1, dim2))
    Show(hist.GetImage(), range, complexMode, projectionMode, coordinates, dim1, dim2, colormap, extent)
