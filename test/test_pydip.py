import PyDIP as dip

a=dip.Image((10,20),1)
a.Fill(3)
b=a[0:4,4:-1]
b.Fill(100)
a.Show('normal')

###

import PyDIP as dip
from scipy import misc

f = misc.face()
a = dip.Image(f)
a.SpatialToTensor()
a.Show()

b = dip.Uniform(a,15)
b.Convert(dip.DT_UINT8)
b.Show()

###

import PyDIP as dip
import numpy as np
from scipy import misc
img = dip.Image(misc.face()[:,:,0])
grad = dip.GradientMagnitude(img,5)
grad.Show()
a = dip.Watershed(grad)
a.Show()
seeds = dip.Image(np.random.random(img.Sizes()))>0.99
b = dip.SeededWatershed(grad,seeds)
b.Show()

###

import PyDIP as dip
from scipy import misc
img = dip.Image(misc.face())
img.SpatialToTensor()
x = dip.Gradient(dip.Norm(img))
x.Show()
y = x*dip.Transpose(x)
y.Show()
