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

import PyDIP.PyDIP_bin as dip

import matplotlib.pyplot as pp

def Show(img, mode=None):
   kwargs = {'coordinates': [0, 0],   # currently only defined for 2D images...
             'dimensions': [0, 1],
             'mode': 'lin',           # 'lin', 'log', 'based'
             'complex': 'abs',        # 'abs', 'phase', 'real', 'imag'
             'projection': 'slice',   # 'slice', 'max', 'mean'
             'bounds': [0,255]
   }
   if mode=='lin':
      kwargs['bounds'] = dip.GetMaximumAndMinimum(img)
   if mode=='percentile':
      kwargs['bounds'] = (dip.Percentile(img,[],5), dip.Percentile(img,[],95))
      print(kwargs['bounds'])
   elif mode=='based':
      kwargs['bounds'] = dip.GetMaximumAndMinimum(img)
      kwargs['mode'] = 'based'
   elif mode=='log':
      kwargs['bounds'] = dip.GetMaximumAndMinimum(img)
      kwargs['mode'] = 'log'
   #pp.imshow(dip.Display(img, **kwargs))
   #pp.show(block=False)
   # TODO: Fix dip.Display

dip.Image.Show = Show
