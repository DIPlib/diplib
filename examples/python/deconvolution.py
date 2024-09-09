import diplib as dip
import numpy as np

# Demonstration of the deconvolution functions
#
# The regularization parameters

# Input image
blur = dip.ImageRead("../blur.tif") + 10
dip.viewer.Show(blur, "Input")

# This is the approximate size of the disk PSF
diameter = 12

# Create PSF image
sz = (diameter // 2) * 2 + 5
psf = dip.Image((sz, sz))
psf.Fill(0)
dip.DrawBandlimitedBall(psf, diameter=diameter, origin=(sz//2, sz//2), value=1)
psf /= dip.Sum(psf)
dip.viewer.Show(psf, "PSF")

# Method 1: Wiener deconvolution
wiener = dip.WienerDeconvolution(blur, psf, regularization=0.04)
dip.viewer.Show(wiener, "Wiener deconvolution")

# Method 2: Tikhonov-Miller deconvolution
tm = dip.TikhonovMiller(blur, psf, regularization=2.0)
dip.viewer.Show(tm, "Tikhonov-Miller deconvolution")

# Method 3: Iterative Tikhonov-Miller deconvolution
itm = dip.IterativeConstrainedTikhonovMiller(blur, psf, regularization=0.3)
dip.viewer.Show(itm, "Iterative Tikhonov-Miller deconvolution")

# Method 4: Richardson-Lucy deconvolution
# Fewer iterations means more regularization. The `regularization` parameter we leave to the default 0.
rl = dip.RichardsonLucy(blur, psf, nIterations=15)
dip.viewer.Show(rl, "Richardson-Lucy deconvolution")

# Method 5: Richardson-Lucy deconvolution with Total Variation regularization
rltv = dip.RichardsonLucy(blur, psf, regularization=0.01, nIterations=50)
dip.viewer.Show(rltv, "Richardson-Lucy w/TV deconvolution")

# Method 6: FISTA deconvolution
fista = dip.FastIterativeShrinkageThresholding(blur, psf, regularization=1.0)
dip.viewer.Show(fista, "FISTA deconvolution")

dip.viewer.Spin()
