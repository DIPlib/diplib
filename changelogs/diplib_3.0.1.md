---
layout: post
title: "Changes DIPlib 3.0.1"
---

## Changes to DIPlib

### New functionality

- Added `dip::MarginalPercentile()` histogram-based statistics function.

- Added `dip::SplitRegions()`, a function to transform labeled images.

- Added `dip::FlushToZero()`, a function to remove denormal values from an image.

- Added `dip::maximum_gauss_truncation()`, returning the maximum truncation that is useful for Gaussian functions.

### Changed functionality

- `dip::Measurement::ObjectIdToIndexMap()` is much faster by changing `dip::ObjectIdToIndexMap`
  from a `std::map` to a `std::unordered_map`. This makes `dip::MeasurementTool::Measure()` is significantly
  faster.

- Added `dip::Measurement::SetObjectIDs()`, improved speed of `dip::Measurement::AddObjectIDs()` and
  `dip::Measurement::operator+()`.

- `dip::OptimalFourierTransformSize()` has a new option to return a smaller or equal size, rather than
  a larger or equal size, so we can crop an image for efficient FFT instead of padding.

- All the variants of the Gaussian filter (`dip::Gauss()` et al.) now limit the truncation value to avoid
  unnecessarily large filter kernels. `dip::DrawBandlimitedPoint()`, `dip::DrawBandlimitedBall()`,
  `dip::DrawBandlimitedBox()`, `dip::GaussianEdgeClip()` and `dip::GaussianLineClip()` also limit the
  truncation in the same way.

- `dip::NeighborList` allows accessing neighbors by index, through the new member functions `Coordinates`,
  `Distance` and `IsInImage`.

- `dip::Label` is slightly more efficient for 3D and higher-dimensional images; added tests. 

### Bug fixes

- `dip::DrawPolygon2D()` produced wrong results for filled polygons when vertices were very close together
  (distances smaller than a pixel).


## Changes to DIPimage

### New functionality

- Added `dip_image/ftz`, a function to remove denormal values from an image.

### Changed functionality

### Bug fixes


## Changes to PyDIP

### New functionality

- Added `dip.FlushToZero()`, a function to remove denormal values from an image.

### Changed functionality

### Bug fixes


## Changes to DIPviewer

### New functionality

### Changed functionality

### Bug fixes


## Changes to DIPjavaio

### New functionality

### Changed functionality

### Bug fixes

- Signed integer images could not be read.
