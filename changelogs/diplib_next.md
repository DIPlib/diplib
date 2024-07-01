---
layout: post
title: "Changes DIPlib 3.x.x"
date: 2020-00-00
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

### Updated dependencies

### Build changes

- Fixed some assorted issues introduced in 3.5.0 for 32-bit builds.



## Changes to *DIPimage*

### New functionality

### Changed functionality

(See also changes to *DIPlib*.)

### Bug fixes

None, but see bugfixes to *DIPlib*.
(See also bugfixes to *DIPlib*.)

### Build and installation changes




## Changes to *PyDIP*

### New functionality

### Changed functionality

(See also changes to *DIPlib*.)

### Bug fixes

None, but see bugfixes to *DIPlib*.
(See also bugfixes to *DIPlib*.)

### Build and installation changes




## Changes to *DIPviewer*

### New functionality

### Changed functionality

- The `dip.viewer.SliceViewer` object now also maps the `complex` and `projection` properties
  (matching the third and fourth columns of the "control panel").

### Bug fixes

### Build changes




## Changes to *DIPjavaio*

### New functionality

### Changed functionality

### Bug fixes

### Build changes
