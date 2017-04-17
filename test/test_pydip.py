import PyDIP as dip
import numpy as np
import matplotlib.pyplot as pp

data = np.random.random((256,256))
data[20:40,60:80] = 0
a = dip.Image(data)
pp.imshow(data);
pp.figure();
pp.imshow(a);
pp.show();

from scipy import misc
f = misc.face()
a = dip.Image(f)
a.SpatialToTensor();
pp.imshow(f);
pp.show(block=False)
a.Show()

b=dip.Uniform(a,15)
b.Convert(dip.DT_UINT8);
b.Show()

###

import PyDIP as dip
from scipy import misc
f = misc.face()
a = dip.Image(f)
a.SpatialToTensor();
dip.Show(a,'percentile')
