---
layout: post
title: "Changes DIPlib 3.x.x"
date: 2020-00-00
---

## Changes to *DIPlib*

### New functionality

- Added a `"antisym reflect"` (`dip::BoundaryCondition::ANTISYMMETRIC_REFLECT`) boundary condition. It is similar to
  `"asym mirror"`, but ensures the derivative is constant at the image boundary.
  See [Issue #214](https://github.com/DIPlib/diplib/issues/214).

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
  See [issue #211](https://github.com/DIPlib/diplib/issues/211).

- The low-level B-spline interpolation function had a bug that could sometimes cause the program to crash.
  See [issue #212](https://github.com/DIPlib/diplib/issues/212).

- `dip::Rotation()` has a bug if the `"periodic"` boundary condition is given. It now throws an exception if this
  boundary condition is used, rather than potentially crashing the program.

- The numeric function `dip::modulo()` didn't handle all cases correctly.

- `operator%()` and the unary `operator-()` for `dip::Image::Pixel` inputs threw a "data type not supported" exception
  for single-precision float pixels.

- `dip::ReadPixelWithBoundaryCondition()` didn't implement the boundary conditions exactly the same way as all other
  functions in this library, it now produces the exact same values for the modes that it supports.

- Projection functions that compute sums (`dip::Sum`, `dip::Mean`, `dip::SumAbs`, `dip::MeanAbs`, `dip::SumSquare`,
  `dip::MeanSquare`, `dip::SumSquareModulus`, `dip::MeanSquareModulus`) or products (`dip::Product`, `dip::GeometricMean`)
  now do the computation using double precision internally, before casting to the output data type (typically
  single-precision float). This should lead to better precision for large images.

### Updated dependencies

- Updated LibTIFF to version 4.7.1.

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

- `dip.Rotation2D()` and `dip.Rotation3D()` use the `"add zeros"` boundary condition by default, like the C++
  functions do.

(See also bugfixes to *DIPlib*.)

### Build and installation changes




## Changes to *DIPviewer*

### New functionality

### Changed functionality

### Bug fixes

- When choosing the labeled color map, a rounding error in the mapping from grayvalues to color map indices became
  apparent, noticeable in a few objects getting the same color as the object with the previous ID. 

- Offsets are no longer overwritten when updating the image being displayed.

### Build changes




## Changes to *DIPjavaio*

### New functionality

### Changed functionality

### Bug fixes

### Build changes
