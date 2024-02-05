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

- DIPlib 3.4.1 changed the behavior of generating a view of an image using a mask when the mask was empty
  (i.e. selected no pixels). This fixed one issue, but created many more: `img.At(mask)=0` stopped working
  when the mask happened to be empty, which is unexpected and very inconvenient (as this syntax is quite common).
  Now an empty view can be generated again, but when cast to an image, a raw image is generated.



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

- Dragging `viewslice` windows between screens no longer causes MATLAB to crash.

(See also bugfixes to *DIPlib*.)




## Changes to *PyDIP*

### New functionality

### Changed functionality

- `dip.Show()` now uses the *DIPlib* color maps by default, taken from `dip.ApplyColorMap()`. The names recognized by
  that function can be used for the `colormap` parameter. Other names are still passed to Matplotlib.

(See also changes to *DIPlib*.)

### Bug fixes

(See also bugfixes to *DIPlib*.)




## Changes to *DIPviewer*

### New functionality

- Added `dipview` entry point.

### Changed functionality

- The "RGB" display will convert the colors in the image to sRGB, rather than linear RGB, to match
  the behavior in other parts of the DIPlib project.

### Bug fixes

- `SliceViewer` properties given as `DimensionArray`s are now properly converted to Python buffers
  again.
- Maintaining a `SliceViewer` handle after a window has been closed no longer
  causes a crash.



## Changes to *DIPjavaio*

### New functionality

### Changed functionality

### Bug fixes
