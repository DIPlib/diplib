---
layout: post
title: "Changes DIPlib 3.x.x"
---

## Changes to *DIPlib*

### New functionality

- Added `"Oklab"`, `"Oklch"`, `"Y'PbPr"` and `"Y'CbCr"` as color spaces.

- Added `dip::Distribution::ConstSample` as an unmutable version of `dip::Distribution::Sample`.

### Changed functionality

- The 'diverging' color map in `dip::ApplyColorMap()` switched from CET-D08 to CET-D07 (i.e. using yellow
  instead of red for positive values).

- `dip::Distribution::Iterator` is now a template class that takes either `dip::Distribution::Sample` or
  `dip::Distribution::ConstSample` as argument. `dip::Distribution::MutableIterator` and `dip::Distribution::ConstIterator`
  are two template specializations, the first one replaces the use of `dip::Distribution::Iterator`.

### Bug fixes

- XYZ to Yxy conversion and its inverse were reversed, and Yxy to gray conversion was incorrect. This means 
  that the Yxy color space was all wrong and inconsistent.

- NPY file header written was correct according to the specification, but didn't exactly match what was written
  by NumPy itself. NumPy under Python 3.12 errored out reading these files. Now the header exactly matches NumPy's.

- `dip::SeededWatershed()` and `dip::CompactWatershed()` discarded the `seeds` input image when it was the same
  as the `out` image (when the caller intended for the algorithm to work in-place).

- Member functions of `dip::Distribution` return `ConstSample` or `ConstIterator` when the distribution object
  is const. See [issue #135](https://github.com/DIPlib/diplib/issues/138).




## Changes to *DIPimage*

### New functionality

- New overload `dip_image/linspace`.

### Changed functionality

- New function `apply_colormap` calls `dip::ApplyColorMap()`.

- The color maps used in the `dipshow` tool are now directly taken from DIPlib. This changes the 'periodic' and
  'diverging' color maps.

- The `dipshow` 'Mappings' menu has a new option 'Sequential', which applies the `"linear"` color map from
  `dip::ApplyColorMap()`. This also affects `dipmapping`, which has a new option `'sequential'`.

(See also changes to *DIPlib*.)

### Bug fixes

- `joinchannels` produced an error if the first input image was a MATLAB array rather than a `dip_image` object.

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

- The "RGB" display will convert the colors in the image to sRGB, rather than linear RGB, to match
  the behavior in other parts of the DIPlib project.

### Bug fixes




## Changes to *DIPjavaio*

### New functionality

### Changed functionality

### Bug fixes
