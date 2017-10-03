# PyDIP 3.0, Python bindings for DIPlib 3.0
#
# (c)2017, Flagship Biosciences, Inc., written by Cris Luengo.
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
This module is PyDIP, the Python interface to DIPlib.

Currently, most functionality is directly mirrored from the DIPlib
library. That is, function names and signatures are mostly identical to
those in DIPlib. Please see the documentation for DIPlib to learn how
to use these functions, as we haven't ported the documentation yet.
   https://diplib.github.io/diplib-docs/

One addition specific to Python is the Show() function and the
Image.Show() method (these are identical). They display an image to the
current matplotlib window, if matplotlib is installed:
   import PyDIP as dip
   img = dip.ImageReadTIFF('cameraman')
   img.Show()

**What is written below is out-of-date** (TODO: update!)

Note that even indexing is as it is in C++, and quite different from
what you'd expect if you are a NumPy user. The [] indexing accesses
the various tensor elements in the image (e.g. color channels). For
spatial indexing, use the Image.At() method:
   img[0]
   img.At(slice(0,-1),slice(100,200))
   img[0].At(slice(0,-1),slice(100,200))
   img.At(slice(0,-1),slice(100,200))[0]
The first line above extracts the red channel. The second line extracts
a rectangular area covering the whole image left to right, and
vertically from index 100 to 200, both included (indexing starts at 0).
The third and fourth lines are equivalent, and combines the two
indexing operations, extracting the rectangular area for the red
channel only. The image returned by Image.At() and [] indexing are
always views of the original image. That is, they share the same data.
Writing to these views also changes the original image.

Irregular indexing is also supported, through the Image.CopyAt()
method. This method copies the pixels to a new image, so cannot be used
to change pixel values. Overloaded versions of Image.CopyAt() can be
used to write to those pixels instead:
   mask = img < 0
   tmp = img.CopyAt(mask)
   tmp.Copy(-tmp)          # does not affect img
   tmp.Fill(0)             # does not affect img
   img.CopyAt(-tmp,mask)   # copies values to pixels selected by mask
   img.FillAt(0,mask)      # writes 0 to the pixels selected by mask

Dimensions are ordered in reverse from how NumPy stores them (the first
dimension is horizontal, or x).
"""

# Here we import classes and functions from the binary and the python-code modules into
# the same namespace.
from PyDIP.PyDIP_bin import *
from PyDIP.PyDIP_py import *
import PyDIP.PyDIPviewer as viewer
