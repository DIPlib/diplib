# (c)2022-2024, Cris Luengo, Wouter Caarls.
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
The portion of the PyDIP module that contains the DIPviewer functionality.
"""

import importlib.util
import sys
import time

hasDIPviewer = False
if importlib.util.find_spec('.PyDIPviewer', __name__.rsplit('.', 1)[0]) is not None:
    try:
        from .PyDIPviewer import *
        hasDIPviewer = True
    except:
        # If that fails, maybe the user has no OpenGL?
        # TODO: do we want to bother the use with this?
        pass

if hasDIPviewer:
    def ShowModal(*args, **kwargs):
        """Show an image in a new SliceViewer window, blocking until it is closed.

           See diplib.viewer.Show for help on parameters."""
        Show(*args, **kwargs)
        Spin()

    # Below we're doing a name switcheroo. `Show()` is defined in .PyDIPviewer. We want to create a new
    # function `Show()` that calls the original one. So first we create an alias `_ViewerShow` for `Show()`,
    # then define a new function `Show()` that calls `_ViewerShow()`.
    _ViewerShow = Show
    def Show(*args, **kwargs):
        """Show an image in a new SliceViewer window.

        Parameters
        ----------
        in : dip.Image
            Image to show.
        title : str, optional
            Window title.
        link : diplib.viewer.SliceViewer, optional
            Window to link to.
        position : tuple of int, optional
            Window position (x and y).
        size : tuple of int, optional
            Window size (width and height).
        dims : tuple of int, optional
            Visualized dimensions: main X, main Y, left X, top Y.
        operating_point : tuple of float, optional
            Selected point.
        element : int, optional
            Selected tensor element.
        zoom : tuple of float, optional
            Zoom factor.
        origin : tuple of int, optional
            Pixel displayed at top left of window.
        mapping_range : tuple of float, optional
            Black and white levels.
        mapping : {"unit", "angle", "8bit", "lin", "base", "log"}, optional
            Automatic mapping range setting.
        lut : {"original", "ternary", "grey", "sequential", "divergent", "periodic", "labels"}, optional
            Color lookup table setting.

        Returns
        -------
        diplib.viewer.SliceViewer
            SliceViewer object for further interaction.
        """
        showargs = dict(filter(lambda elem: elem[0] == 'in' or elem[0] == 'title', kwargs.items()))
        v = _ViewerShow(*args, **showargs)

        for elem in kwargs.items():
            if elem[0] == 'in' or elem[0] == 'title':
                continue
            elif elem[0] == 'position':
                v.SetPosition(elem[1][0], elem[1][1])
            elif elem[0] == 'size':
                v.SetSize(elem[1][0], elem[1][1])
            elif elem[0] == 'link':
                v.Link(elem[1])
            else:
                setattr(v, elem[0], elem[1])

        return v

    # Allow IPython users to write `%gui dip` to enable interaction
    if 'IPython' in sys.modules:
        from IPython import terminal

        def _inputhook(context):
            while not context.input_is_ready():
                Draw()
                time.sleep(0.001)

        terminal.pt_inputhooks.register('dip', _inputhook)

    # Same for IPython kernels
    if 'ipykernel' in sys.modules:
        from ipykernel.eventloops import register_integration

        @register_integration('dip')
        def _loop_dip(kernel):
            """Start a kernel with the dipviewer event loop."""
            while not kernel.shell.exit_now:
                if kernel.shell_stream.flush(limit=1):
                    return
                Draw()
                time.sleep(0.001)
