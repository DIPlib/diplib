---
layout: post
title: "Changes DIPlib 3.x.x"
---

## Changes to *DIPlib*

### New functionality

### Changed functionality

- The 'diverging' color map in `dip::ApplyColorMap()` switched from CET-D08 to CET-D07 (i.e. using yellow
  instead of red for positive values).

### Bug fixes

- XYZ to Yxy conversion and its inverse were reversed, and Yxy to gray conversion was incorrect. This means 
  that the Yxy color space was all wrong and inconsistent.



## Changes to *DIPimage*

### New functionality

### Changed functionality

- New function `apply_colormap` calls `dip::ApplyColorMap()`.

- The color maps used in the `dipshow` tool are now directly taken from DIPlib. This changes the 'periodic' and
  'diverging' color maps.

- The `dipshow` 'Mappings' menu has a new option 'Sequential', which applies the `"linear"` color map from
  `dip::ApplyColorMap()`. This also affects `dipmapping`, which has a new option `'sequential'`.

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
