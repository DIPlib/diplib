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

- The stream insertion operator (`<<`) for `dip::Image` now also notes if the image is protected, if the data is external,
  if it has an `ExternalInterface` attached, and if the data is contiguous.

- `dip::Image::HasContiguousData()` and `dip::Image::HasNormalStrides()` no longer check if the image is forged or not.
  These functions only check the sizes and strides of the image, which can be set before the image is forged.

- `dip::ExternalInterface` has a new virtual member function `Name()` that derived classes can overload to give
  themselves a name.

### Bug fixes

- `dip::Image::Mask` used multiplication for masking, which doesn't work to mask out NaN or Infinity values.

- `dip::NormalizedConvolution` and `dip::NormalizedDifferentialConvolution` didn't handle NaN or infinity input values
  correctly if they were masked out. This now works correctly for the case of binary mask image. For non-binary masks,
  the user is now warned by the documentation to remove such values from the image before the convolution.

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

- Added bindings for three functions that manipulate the image's data segment: `dip.Image.ForceNormalStrides()`,
  `dip.Image.ForceContiguousData()` and `dip.Image.Separate()`.

### Changed functionality

- Regular indexing (such as `img[10:40:2, :]`), which creates a new image that shares data with the original image,
  now has the output image protected. This allows the user to write into the sub-image with confidence.
  See [issue #204](https://github.com/DIPlib/diplib/issues/204).

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
