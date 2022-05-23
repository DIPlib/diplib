---
layout: post
title: "Changes DIPlib 3.x.x"
---

## Changes to *DIPlib*

### New functionality

- Added `dip::Kernel::IgnoreZeros()`, which causes subsequent calls to `dip::Kernel::PixelTable()` for that
  object to also exclude zero values in the gray-scale kernel image.

### Changed functionality

### Bug fixes

- `dip::ResampleAt(in, map)` didn't copy the color space information from the input image to the output image.

- `dip::GeneralConvolution()` skips zero pixels in the kernel image, as was described in the documentation.
  This makes the operation significantly faster if the kernel has many zero pixels.




## Changes to *DIPimage*

### New functionality

### Changed functionality

- Added a new parameter to `convolve`, which allows the selection of the computation method.

(See also changes to *DIPlib*.)

### Bug fixes

(See also bugfixes to *DIPlib*.)




## Changes to *PyDIP*

### New functionality

### Changed functionality

(See also changes to *DIPlib*.)

### Bug fixes

(See also bugfixes to *DIPlib*.)




## Changes to *DIPviewer*

### New functionality

### Changed functionality

### Bug fixes




## Changes to *DIPjavaio*

### New functionality

### Changed functionality

### Bug fixes
