---
layout: post
title: "Changes DIPlib 3.4.2"
date: 2024-03-16
---

## Changes to *DIPlib*

### New functionality

- Added `"Oklab"`, `"Oklch"`, `"Y'PbPr"` and `"Y'CbCr"` as color spaces.

- Added `dip::Distribution::ConstSample` as an unmutable version of `dip::Distribution::Sample`.

- Added functions `dip::ImageReadPNG()`, `dip::ImageReadPNGInfo()`, `dip::ImageIsPNG()` and `dip::ImageWritePNG()`.
  Previously, PNG files could only be read through *DIPjavaio* with Bio-Formats.

- The functions `dip::ImageRead()` and `dip::ImageWrite()` now recognize PNG files.

- Added overloads for `dip::ImageReadPNG()`, `dip::ImageReadPNGInfo()`, `dip::ImageWritePNG()`,
  `dip::ImageReadJPEG()`, `dip::ImageReadJPEGInfo()`, and `dip::ImageWriteJPEG()` that read from and write to
  a memory buffer rather than a file.

- Added function `dip::FileAppendExtension()`.

### Changed functionality

- The 'diverging' color map in `dip::ApplyColorMap()` switched from CET-D08 to CET-D07 (i.e. using yellow
  instead of red for positive values).

- `dip::Distribution::Iterator` is now a template class that takes either `dip::Distribution::Sample` or
  `dip::Distribution::ConstSample` as argument. `dip::Distribution::MutableIterator` and `dip::Distribution::ConstIterator`
  are two template specializations, the first one replaces the use of `dip::Distribution::Iterator`.

- `dip::FileAddExtension()` has been deprecated, the functionality is not needed in *DIPlib*.

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

- Equality comparison of `dip::PixelSize` objects was documented to use a tolerance, but it wasn't using one.

- `dip::ImageReadTIFF()`, `dip::ImageReadJPEG()` and `dip::ImageReadNPY()` would replace the extension of the given
  file name instead of simply appending an appropriate extension, as described in the documentation.

### Updated dependencies

- Updated included *zlib* to version 1.3.1. This version no longer uses K&R function declarations, which generated
  lots of warnings in newer compilers.




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

- Added functions `dip.ImageReadPNG()`, `dip.ImageReadPNGInfo()`, `dip.ImageIsPNG()` and `dip.ImageWritePNG()`.

- Added functions for reading and writing PNG and JPEG as a bytes object.

### Changed functionality

- `dip.Show()` now uses the *DIPlib* color maps by default, taken from `dip.ApplyColorMap()`. The names recognized by
  that function can be used for the `colormap` parameter. Other names are still passed to Matplotlib.

- All `dip.ImageRead...()` functions now have an overload with named `out` argument to read an image into a
  pre-allocated memory buffer.

(See also changes to *DIPlib*.)

### Bug fixes

None, but see bugfixes to *DIPlib*.




## Changes to *DIPviewer*

### New functionality

- Added `dipview` entry point.

### Changed functionality

- The "RGB" display will convert the colors in the image to sRGB, rather than linear RGB, to match
  the behavior in other parts of the DIPlib project.

### Bug fixes

- `SliceViewer` properties given as `DimensionArray`s are now properly converted to Python buffers
  again.

- Maintaining a `SliceViewer` handle after a window has been closed no longer causes a crash.



## Changes to *DIPjavaio*

None
