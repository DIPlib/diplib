import diplib as dip

# Playing around with the options of the seeded watershed

img = dip.ImageReadICS('../cermet')
grad = dip.GradientMagnitude(img, 5)
grad.ShowSlice("grad")

a = dip.Watershed(grad)
a.ShowSlice("watershed")

seeds = dip.CreatePoissonPointProcess(img.Sizes(), 0.001)
b = dip.SeededWatershed(grad, seeds, flags={"labels"})
b.ShowSlice("watershed seeded with Poisson point process", lut="labels")

smooth = dip.Gauss(img, 5)
seeds = dip.Minima(smooth)
b = dip.SeededWatershed(grad, seeds, flags={"labels"})
b.ShowSlice("watershed seeded with minima of smoothed image", lut="labels")

mask = img < 120
b = dip.SeededWatershed(smooth, seeds, mask, flags={"labels", "uphill only"})
b.ShowSlice("watershed, uphill only", lut="labels")

dip.viewer.Spin()
