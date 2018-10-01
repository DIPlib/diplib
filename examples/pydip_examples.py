import PyDIP as dip

a = dip.Image((10,20),1)
a.Fill(3)
b = a[0:4,4:-1]
b.Fill(55)
a[:3,:10] = 100
a[5:7,10:15] = 200
a.Show('normal')

m = a>=100
m.Show()

a[m].Show('normal')

a[m] = 176
a.Show('normal')

###

from scipy import misc
f = misc.face()
a = dip.Image(f)
a.Show()

b = dip.Uniform(a,15)
b.Convert("UINT8")
b.Show()

###

img = dip.ImageReadICS('cermet')
grad = dip.GradientMagnitude(img,[5]) # TODO: fix syntax: allow scalar as sigma
grad.Show()

a = dip.Watershed(grad)
a.Show()

import numpy as np
seeds = dip.Image(np.random.random(list(reversed(img.Sizes()))))>0.999
b = dip.SeededWatershed(grad,seeds,flags={"labels"})
b.Show()

smooth = dip.Gauss(img,[5])
seeds = dip.Minima(smooth)
b = dip.SeededWatershed(grad,seeds,flags={"labels"})
b.Show()

mask = img < 120
b = dip.SeededWatershed(smooth,seeds,mask,flags={"labels","uphill only"})
b.Show('labels')


###

img = dip.ImageReadTIFF('erika')
h,b = dip.Histogram(img)
import matplotlib.pyplot as pp
pp.clf()
pp.plot(b[0],h)
pp.show(block=False)

###

img = dip.ImageReadTIFF('erika')
x = dip.Gradient(dip.Norm(img))
x.Show()

y = x*dip.Transpose(x)
y.Show()

###

a = dip.Image((20,10),3)
dip.FillXCoordinate(a.TensorElement(0))
dip.FillYCoordinate(a.TensorElement(1))
dip.FillColoredNoise(a.TensorElement(2))
a.Show()

a.SetColorSpace('rgb')
b = dip.ColorSpaceManager.Convert(a,'Lab')
b.Show()

b.TensorElement(1).Show()

b,c = dip.Histogram(a)
print(len(c))
print(b.Dimensionality() == len(c))

a = dip.Image((250,260),3)
a.Fill(0)
import random
color = list([1.0,1.5,0.5])
for ii in range(0,1000):
   random.shuffle(color)
   dip.DrawBandlimitedPoint(a,[random.uniform(-2,252),random.uniform(-2,262)],color,[random.uniform(1,3),random.uniform(1,3)])

a.Show()

###

a = dip.ImageReadICS('cermet')
a.SetPixelSize(dip.PixelSize(dip.PhysicalQuantity(1,"um")))
b = dip.Label(a < 120)
dip.MeasurementTool.Features()
m = dip.MeasurementTool.Measure(b,a,['Size','Feret','Solidity','Statistics'])
print(m)
print(m['Feret'][50][2])
dip.WriteCSV(m,'test.csv')
dip.WriteCSV(m,'test2.csv',{'unicode','simple'})

b = a < 120
b = dip.EdgeObjectsRemove(b)
b = dip.Label(b)
m = dip.MeasurementTool.Measure(b,a,features=['EllipseVariance','P2A','Roundness','Circularity','Solidity','Convexity'])
print(m)
dip.Show(dip.ObjectToMeasurement(b,m['EllipseVariance']))
dip.Show(dip.ObjectToMeasurement(b,m['Roundness']))
dip.Show(dip.ObjectToMeasurement(b,m['Circularity']))
dip.Show(dip.ObjectToMeasurement(b,m['Solidity']))
dip.Show(dip.ObjectToMeasurement(b,m['Convexity']))

b = dip.EuclideanSkeleton(a > 120)
dip.GetEndPixels(b).Show()
dip.GetLinkPixels(b).Show()
dip.GetBranchPixels(b).Show()

###

a = dip.ImageReadICS('cermet')
b = dip.Label(a < 120)
b.Show('labels')
dip.GetObjectLabels(b)
c = dip.SmallObjectsRemove(b, 150)
c.Show('labels')
dip.GetObjectLabels(c)
d = dip.Relabel(c)
d.Show('labels')
dip.GetObjectLabels(d)

###

mask = dip.Image([70,70],1,'BIN')
mask.Fill(0)
dip.DrawEllipsoid(mask,[40,50],[30,40])
dip.DrawEllipsoid(mask,[4,5],[40,40],[0])

seed = dip.Image([70,70],1,'BIN')
seed.Fill(0)
dip.DrawBox(seed,[5,70],[20,35])
dip.DrawLine(seed,[14,28],[35,28])
dip.DrawLine(seed,[14,30],[35,30])
dip.DrawLine(seed,[14,40],[35,40])

dip.BinaryPropagation(seed,mask,1,25).Show()
dip.ConditionalThickening2D(seed,mask,25).Show()

import timeit
a = dip.BinaryPropagation(seed,mask,1,0)
b = dip.MorphologicalReconstruction(seed,mask,1)
print(dip.All(a==b)[0][0])
timeit.timeit("dip.BinaryPropagation(seed,mask,1,0)", number=1000, globals=globals())
timeit.timeit("dip.MorphologicalReconstruction(seed,mask,1)", number=1000, globals=globals())

###

a = dip.ImageReadICS('cermet')
b = dip.Label(dip.EuclideanSkeleton(a < 120, 0, 'loose ends away'))
dip.MorphologicalReconstruction(b*1000,a).Show()
dip.LimitedMorphologicalReconstruction(b*1000,a,6).Show()

###

a = dip.Image([10,11],1,'UINT8')
a.Fill(0)
a[1,1] = 255
a[-2,-2] = 200
a[1,-2] = 150
a[-2,1] = 100
a[0,5] = 50
a[-1,5] = 50
a[5,0] = 40
a[5,-1] = 40
Show(a)

Show(dip.ExtendImage(a,[45,45],['mirror']))
Show(dip.ExtendImage(a,[45,45],['asym mirror']))
Show(dip.ExtendImage(a,[45,45],['periodic']))
Show(dip.ExtendImage(a,[45,45],['asym periodic']))

###

a = dip.ImageReadICS('trui')
b = a.Similar('UINT8')
b.Fill(0)
b[50,120] = 1
b[225,120] = 2
b[161,61] = 3
c = b==0
m = a.Similar('BIN')
m.Fill(1)
dip.DrawEllipsoid(m,[60,30],[70,180],[0])
dip.DrawEllipsoid(m,[20,80],[85,120],[0])
d1 = dip.GreyWeightedDistanceTransform(a,c)
d2 = dip.GreyWeightedDistanceTransform(a,c,m)
Show(dip.Overlay(dip.Overlay(a,~m),~c,[0,200,0]))
Show(d1)
Show(d2)
r1 = dip.GrowRegionsWeighted(b,a)
r2 = dip.GrowRegionsWeighted(b,a,m)
Show(r1)
Show(r2)
r3 = dip.GrowRegions(b)
r4 = dip.GrowRegions(b,m)
Show(r3)
Show(r4)

###

import matplotlib.pyplot as pp
import numpy as np

img1 = dip.ImageReadTIFF('../../../examples/erika')
img2 = dip.ImageReadICS('../../../examples/trui')

h,b = dip.Histogram(img2)
img3 = dip.HistogramMatching(img1,h)
img3.Convert('UINT8')

h1,b1 = dip.Histogram(img1,64)
h2,b2 = dip.Histogram(img2,64)
h3,b3 = dip.Histogram(img3,64)

pp.clf()
pp.subplot(3,1,1)
pp.bar(b1[0],np.array(h1),align='center')
pp.subplot(3,1,2)
pp.bar(b2[0],np.array(h2),align='center')
pp.subplot(3,1,3)
pp.bar(b3[0],np.array(h3),align='center')
pp.show(block=False)

###

a = dip.ImageReadICS('cermet')<120

import numpy as np
intv = dip.Interval(np.array([
   [ 2,2,1,0,2 ],
   [ 2,1,1,0,2 ],
   [ 1,1,0,0,2 ],
   [ 0,0,0,2,2 ],
   [ 2,2,2,2,2 ]],'uint8'))

Show(a)
Show(dip.UnionSupGenerating2D(a, intv, 90 ))

ht = dip.HomotopicThinningInterval2D()
skel8 = dip.Thinning(a, dip.Image(), ht)
Show(skel8)

ht = dip.BranchPixelInterval2D()
Show(dip.UnionSupGenerating(skel8, ht))

ht = dip.HomotopicEndPixelInterval2D()
skel8 = dip.Thinning(skel8, dip.Image(), ht)
Show(skel8)

ht = dip.HomotopicThinningInterval2D(1)
skel4 = dip.Thinning(a, dip.Image(), ht)
Show(skel4)
ht = dip.HomotopicEndPixelInterval2D(1)
skel4 = dip.Thinning(skel4, dip.Image(), ht)
Show(skel4)


ht = dip.HomotopicThickeningInterval2D(2)
skiz = dip.Thickening(a, dip.Image(), ht)
Show(skiz)
ht = dip.HomotopicInverseEndPixelInterval2D(2)
skiz = dip.Thickening(skiz, dip.Image(), ht)
Show(skiz)

ht = dip.BoundaryPixelInterval2D()
Show(dip.UnionSupGenerating2D(a, ht, 90))

ht = dip.ConvexHullInterval2D()
Show(dip.Thickening(a, dip.Image(), ht))

###

import numpy as np
import matplotlib.pyplot as pp

a = dip.Label(dip.ImageReadICS('cermet')<120)
ccs = dip.GetImageChainCodes(a)
Show(a==31)
cc = ccs[30] # indexing starts at 0, but label IDs start at 1
print(cc.objectID) # returns 31
print(cc.Length()) # best approximation of perimeter
print(cc.Polygon().Length()) # poor approximation of perimeter
print(cc.Feret()) # poor approximation of Feret diameters
print(cc.Polygon().Feret()) # best approximation of Feret diameters
print(cc.BoundingBox())
print(cc.Polygon().BoundingBox())
pol = cc.Polygon()
print(pol.Area() + 0.5)
print(pol.Centroid())
print(pol.RadiusStatistics())
print(pol.EllipseParameters())
print(pol.EllipseVariance())
np.array(pol.ConvexHull())

pol = np.array(pol)
pp.clf()
a.Show('labels')
pp.plot(pol[:,0],pol[:,1],'w-')
pp.show(block=False)

###

import math

a = dip.ImageReadICS('cermet')
b = dip.ImageReadTIFF('erika')
a = a / dip.Maximum(a)
b = b / dip.Maximum(b)

dice = dip.DiceCoefficient(a, b)
jaccard = dip.JaccardIndex(a, b)
sensitivity = dip.Sensitivity(a, b)
specificity = dip.Specificity(a, b)
accuracy = dip.Accuracy(a, b)
precision = dip.Precision(a, b)
overlap = dip.SpatialOverlap(a, b)
assert math.isclose(overlap.diceCoefficient, dice, abs_tol=1e-4)
assert math.isclose(overlap.jaccardIndex, jaccard, abs_tol=1e-4)
assert math.isclose(overlap.sensitivity, sensitivity, abs_tol=1e-4)
assert math.isclose(overlap.specificity, specificity, abs_tol=1e-4)
assert math.isclose(overlap.accuracy, accuracy, abs_tol=1e-4)
assert math.isclose(overlap.precision, precision, abs_tol=1e-4)

a = a>0.5
b = b>0.5

dice = dip.DiceCoefficient(a, b)
jaccard = dip.JaccardIndex(a, b)
sensitivity = dip.Sensitivity(a, b)
specificity = dip.Specificity(a, b)
accuracy = dip.Accuracy(a, b)
precision = dip.Precision(a, b)
overlap = dip.SpatialOverlap(a, b)
assert math.isclose(overlap.diceCoefficient, dice)
assert math.isclose(overlap.jaccardIndex, jaccard)
assert math.isclose(overlap.sensitivity, sensitivity)
assert math.isclose(overlap.specificity, specificity)
assert math.isclose(overlap.accuracy, accuracy)
assert math.isclose(overlap.precision, precision)
