import diplib as dip
import numpy as np


input = dip.ImageRead('../DIP.tif')
dip.viewer.Show(input)

# Compute histogram and cluster it
hist = dip.Histogram(input, configuration=dip.Histogram.Configuration(0.0, 255.0, 64))
centers = dip.MinimumVariancePartitioning(hist, out=hist, nClusters=3)

# Reverse lookup to create the segmented image
labels = hist.ReverseLookup(input)
dip.viewer.Show(labels, mapping="8bit", lut="labels")

# A color lookup table to paint each label with the cluster center RGB values
centers.insert(0, [0.0, 0.0, 0.0])  # label #0 doesn't have an entry in centers, this aligns the labels to the indices
lut = dip.LookupTable(dip.Image(np.array(centers), tensor_axis=1))
output = lut.Apply(labels)

output.SetColorSpace(input.ColorSpace())
dip.viewer.Show(output)
dip.viewer.Spin()

