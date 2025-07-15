import diplib as dip

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
dip.BlendBandlimitedMask(out2, dip.Create0D(0.3), image2)
out2.ShowSlice("out2")

out3 = image1.Copy()
dip.BlendBandlimitedMask(out3, mask, dip.Create0D([255, 0, 0]), [195 - 126, 195 - 91])
out3.ShowSlice("out3")

dip.viewer.Spin()
