---
layout: post
title: "Changes DIPlib 3.x.x"
date: 2020-00-00
---

## Changes to *DIPlib*

### New functionality

### Changed functionality

- The `"label"` color map produced by `dip::ColorMapLut()` and used by `dip::ApplyColorMap()` now has 60 unique colors,
  up from 19. Only the first 6 colors are the same as before. This also affects the labeled image display in DIPviewer,
  and in the Python and MATLAB packages.

### Bug fixes

- `dip::Image::Mask` used multiplication for masking, which doesn't work to mask out NaN or Infinity values.

### Updated dependencies

### Build changes




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

### Bug fixes

- When choosing the labeled color map, a rounding error in the mapping from grayvalues to color map indices became
  apparent, noticeable in a few objects getting the same color as the object with the previous ID. 

### Build changes




## Changes to *DIPjavaio*

### New functionality

### Changed functionality

### Bug fixes

### Build changes
