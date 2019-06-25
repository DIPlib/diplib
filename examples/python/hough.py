#!/usr/bin/env python3

# Tests some functions for finding circles using the Hough transform
import PyDIP as dip

# Prepare image
a = dip.Image((512, 512))

dip.DrawEllipsoid(a, (200, 200), (256, 256))
dip.DrawEllipsoid(a, (50, 50), (350, 350))

gv = dip.Gradient(a)
gm = dip.Norm(gv)
bin = dip.IsodataThreshold(gm)

# Find circles using low-level functions
h = dip.HoughTransformCircleCenters(bin, gv)
m = dip.FindHoughMaxima(h, 10)
d = dip.PointDistanceDistribution(bin, m)
r = d.MaximumLikelihood()
print(m, r)

# Find circles using integrated function
c = dip.FindHoughCircles(bin, gv, (), 10)
print(c)

# Show original and transform
dip.viewer.Show(bin, "Input")
dip.viewer.Show(h, "Hough transform")
dip.viewer.Spin()
