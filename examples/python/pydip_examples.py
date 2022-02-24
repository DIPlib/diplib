import diplib as dip
import numpy as np
import matplotlib.pyplot as plt
import random
import math

###

img = dip.ImageReadICS('../cermet')
grad = dip.GradientMagnitude(img, 5)
grad.Show()

a = dip.Watershed(grad)
a.Show()

seeds = dip.CreatePoissonPointProcess(img.Sizes(), 0.001)
b = dip.SeededWatershed(grad, seeds, flags={"labels"})
b.Show()

smooth = dip.Gauss(img, 5)
seeds = dip.Minima(smooth)
b = dip.SeededWatershed(grad, seeds, flags={"labels"})
b.Show()

mask = img < 120
b = dip.SeededWatershed(smooth, seeds, mask, flags={"labels", "uphill only"})
b.Show('labels')

###

img = dip.ImageReadTIFF('../erika')
h = dip.Histogram(img)
h.Show()

###

a = dip.Image((20, 10), 3)
dip.FillXCoordinate(a(0))
dip.FillYCoordinate(a(1))
dip.FillColoredNoise(a(2))
a.Show()

a.SetColorSpace('rgb')
b = dip.ColorSpaceManager.Convert(a, 'Lab')
b.Show()

b(1).Show()

b = dip.Histogram(a)
print(b.Bins())

a = dip.Image((250, 260), 3)
a.Fill(0)
color = list([1.0, 1.5, 0.5])
for ii in range(0, 1000):
   random.shuffle(color)
   dip.DrawBandlimitedPoint(a, [random.uniform(-2, 252), random.uniform(-2, 262)], color,
                               [random.uniform(1, 3), random.uniform(1, 3)])

a.Show()

###

a = dip.ImageReadICS('../cermet')
a.SetPixelSize(1, "um")
b = dip.Label(a < 120)
dip.MeasurementTool.Features()
m = dip.MeasurementTool.Measure(b, a, ['Size', 'Feret', 'Solidity', 'Statistics'])
print(m)
print(m['Feret'][50][2])
dip.MeasurementWriteCSV(m, 'test.csv')
dip.MeasurementWriteCSV(m, 'test2.csv', {'unicode', 'simple'})

np.asarray(m)
np.asarray(m['Feret'])
np.asarray(m[50])

b = a < 120
b = dip.EdgeObjectsRemove(b)
b = dip.Label(b)
m = dip.MeasurementTool.Measure(b, a, features=['EllipseVariance', 'P2A', 'Roundness', 'Circularity', 'Solidity', 'Convexity'])
print(m)
dip.Show(dip.ObjectToMeasurement(b, m['EllipseVariance']))
dip.Show(dip.ObjectToMeasurement(b, m['Roundness']))
dip.Show(dip.ObjectToMeasurement(b, m['Circularity']))
dip.Show(dip.ObjectToMeasurement(b, m['Solidity']))
dip.Show(dip.ObjectToMeasurement(b, m['Convexity']))

b = dip.EuclideanSkeleton(a > 120)
dip.GetEndPixels(b).Show()
dip.GetLinkPixels(b).Show()
dip.GetBranchPixels(b).Show()

###

a = dip.ImageReadICS('../trui')
b = a.Similar('UINT8')
b.Fill(0)
b[50, 120] = 1
b[225, 120] = 2
b[161, 61] = 3
c = b == 0
m = a.Similar('BIN')
m.Fill(1)
dip.DrawEllipsoid(m, [60, 30], [70, 180], [0])
dip.DrawEllipsoid(m, [20, 80], [85, 120], [0])
d1 = dip.GreyWeightedDistanceTransform(a, c)
d2 = dip.GreyWeightedDistanceTransform(a, c, m)
dip.Overlay(dip.Overlay(a, ~m), ~c, [0, 200, 0]).ShowSlice("overlay")
d1.ShowSlice("d1")
d2.ShowSlice("d2")
r1 = dip.GrowRegionsWeighted(b, a)
r2 = dip.GrowRegionsWeighted(b, a, m)
r1.ShowSlice("r1", lut="labels", mapping="8bit")
r2.ShowSlice("r2", lut="labels", mapping="8bit")
r3 = dip.GrowRegions(b)
r4 = dip.GrowRegions(b, m)
r3.ShowSlice("r3", lut="labels", mapping="8bit")
r4.ShowSlice("r4", lut="labels", mapping="8bit")
dip.viewer.Spin()

###

img1 = dip.ImageReadTIFF('../erika')
img2 = dip.ImageReadICS('../trui')

h = dip.Histogram(img2)
img3 = dip.HistogramMatching(img1, h)
img3.Convert('UINT8')

conf = dip.Histogram.Configuration(0, 255, nBins=64)
h1 = dip.Histogram(img1, configuration=conf)
h2 = dip.Histogram(img2, configuration=conf)
h3 = dip.Histogram(img3, configuration=conf)

plt.close()
plt.subplot(3, 1, 1)
plt.bar(h1.BinCenters(), np.asarray(h1.GetImage()), width=3, align='center')
plt.subplot(3, 1, 2)
plt.bar(h2.BinCenters(), np.asarray(h2.GetImage()), width=3, align='center')
plt.subplot(3, 1, 3)
plt.bar(h3.BinCenters(), np.asarray(h3.GetImage()), width=3, align='center')
plt.show(block=False)

###

a = dip.Label(dip.ImageReadICS('../cermet') < 120)
ccs = dip.GetImageChainCodes(a)
dip.Show(a == 31)
cc = ccs[30]  # indexing starts at 0, but label IDs start at 1
print(cc.objectID)  # returns 31
print(cc.Length())  # best approximation of perimeter
print(cc.Polygon().Length())  # poor approximation of perimeter
print(cc.Feret())  # poor approximation of Feret diameters
print(cc.Polygon().Feret())  # best approximation of Feret diameters
print(cc.BoundingBox())
print(cc.Polygon().BoundingBox())
pol = cc.Polygon()
print(pol.Area() + 0.5)
print(pol.Centroid())
print(pol.RadiusStatistics())
print(pol.EllipseParameters())
print(pol.EllipseVariance())
np.asarray(pol.ConvexHull())

pol = np.asarray(pol)
plt.close()
plt.imshow(a)
plt.plot(pol[:, 0], pol[:, 1], 'w-')
plt.show(block=False)

###

a = dip.ImageReadICS('../cermet')
b = dip.ImageReadTIFF('../erika')
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

a = a > 0.5
b = b > 0.5

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

###

weights = dip.Image([1050,1000], 1, 'UINT8')
weights.Fill(1)
mask = dip.CreateRamp(weights.Sizes(), 0) > 0
weights[mask] = 2
mask = dip.Abs(dip.CreateRadiusCoordinate(weights.Sizes()) - 350) >= 10
mask |= dip.CreateRamp(weights.Sizes(), 1) > 0
bin = weights.Similar('BIN')
bin.Fill(1)
bin[100,100] = 0

out = dip.GreyWeightedDistanceTransform(weights, bin, mask)
out = dip.Modulo(out, 50)

weights.SetPixelSize([0.8, 1.2])

out2 = dip.GreyWeightedDistanceTransform(weights, bin, mask)
out2 = dip.Modulo(out2, 50)

out.ShowSlice()
out2.ShowSlice()
dip.viewer.Spin()

###

a = dip.ImageReadICS('../trui')
b = dip.Dilation(a, dip.StructuringElement(30, 'rectangular'))
c = dip.Dilation(a, dip.StructuringElement([30, 10], 'rectangular'))
d = dip.Dilation(a, dip.StructuringElement((30, 10), 'rectangular'))
e = dip.Dilation(a, dip.SE(30, 'rectangular'))
f = dip.Dilation(a, 30)
g = dip.Dilation(a, [30, 10])
h = dip.Dilation(a, (30, 10))
i = dip.Dilation(a, 'rectangular')

###

# Replicate what ../cpp/blend_images.cpp does
image1 = dip.ImageRead("../DIP.tif")
image2 = dip.ImageRead("../trui.ics")
image1.ShowSlice("image1")
image2.ShowSlice("image2")

mask = image2.Similar("SFLOAT")
mask.Fill(0)
dip.DrawBandlimitedBall(mask, 110, [126, 91], 1, "filled", 10)
mask.ShowSlice("mask")

out1 = image1.Copy()
dip.BlendBandlimitedMask(out1, mask, image2, [195 - 126, 195 - 91])
out1.ShowSlice("out1")

out2 = image1.Copy()
dip.BlendBandlimitedMask(out2, dip.Image(0.3), image2)
out2.ShowSlice("out2")

out3 = image1.Copy()
dip.BlendBandlimitedMask(out3, mask, dip.Create0D([255, 0, 0]), [195 - 126, 195 - 91])
out3.ShowSlice("out3")

dip.viewer.Spin()

###

# Replicate what ../cpp/oversegmentation.cpp does
input = dip.ImageReadICS("../trui.ics")
superpixels = dip.Superpixels(input, 0.01, 1.0, "CW", {"no gaps"})
msr = dip.MeasurementTool.Measure(superpixels, input, ["Mean"])
graph = dip.RegionAdjacencyGraph(superpixels, msr["Mean"], "touching")
graph = graph.MinimumSpanningForest([1])
graph.RemoveLargestEdges(80 - 1)  # Find 80 regions
output = dip.Relabel(superpixels, graph)

superpixels = dip.ObjectToMeasurement(superpixels, msr["Mean"])
msr = dip.MeasurementTool.Measure(output, input, ["Mean"])
output = dip.ObjectToMeasurement(output, msr["Mean"])

win1 = input.ShowSlice("input")
win2 = superpixels.ShowSlice("superpixels", link=win1)
win3 = output.ShowSlice("output", link=win2)
dip.viewer.Spin()
