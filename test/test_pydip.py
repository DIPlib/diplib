import PyDIP as dip

a=dip.Image((10,20),1)
a.Fill(3)
b=a[0:4,4:-1]
b.Fill(55)
a[:3,:10]=100
a[5:7,10:15]=200
a.Show('normal')

m=a>=100
m.Show()

a[m].Show('normal')

a[m]=176
a.Show('normal')

###

import PyDIP as dip
from scipy import misc

f = misc.face()
a = dip.Image(f)
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
x = dip.Gradient(dip.Norm(img))
x.Show()

y = x*dip.Transpose(x)
y.Show()
