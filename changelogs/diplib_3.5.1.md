---
layout: post
title: "Changes DIPlib 3.5.1"
date: 2024-07-05
---

## Changes to *DIPlib*

### New functionality

- Added `dip::SplitChannels()` to `diplib/geometry.h`.

### Changed functionality

- `dip::HoughTransformCircleCenters()` now draws lines in the parameter space with an intensity proportional to the
  magnitude of the input gradient. Low gradients are likely less precise in orientation, so down-weighing them
  improves precision.

### Bug fixes

- `dip::saturated_mul()` for `dip::sint64` inputs was broken on systems that don't have 128-bit arithmetic.

- Improved `dip::PhysicalQuantity::Normalize()` to avoid floating-point rounding errors.

- `dip::DrawEllipsoid()` and `dip::DrawDiamond()` could produce wonky shapes for even diameters,
  due to a floating-point rounding error.

- JPEG encoding and decoding had a small memory leak.

- `dip::GetOptimalDFTSize()` could loop indefinitely for inputs close to the maximum `dip::uint` value, because
  of an integer overflow in arithmetic.

- The classes `dip::StatisticsAccumulator` and `dip::VarianceAccumulator` divided by 0 when adding two empty
  accumulators together, producing NaN values as output. This caused the function `dip::SampleStatistics()`
  to sometimes produce NaN statistics when called with a mask image.

### Build changes

- Fixed some assorted issues introduced in 3.5.0 for 32-bit builds.
  
- Added a 32-bit build to the CI on GitHub.




## Changes to *DIPimage*

### Bug fixes

None, but see bugfixes to *DIPlib*.




## Changes to *PyDIP*

### Bug fixes

- `dip.ChainCode.Length()` had a parameter `boundaryPixels` added in *DIPlib* 3.3.0, but the Python bindings
  did not know about its default value.

(See also bugfixes to *DIPlib*.)




## Changes to *DIPviewer*

### Changed functionality

- The `dip.viewer.SliceViewer` object now also maps the `complex` and `projection` properties
  (matching the third and fourth columns of the "control panel").




## Changes to *DIPjavaio*

None
