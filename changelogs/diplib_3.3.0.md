---
layout: post
title: "Changes DIPlib 3.3.0"
date: 2022-05-21
---

## Changes to *DIPlib*

### New functionality

- Added `dip::MakeRegionsConvex2D()`, which works for both labelled and binary images.

- Added `Perimeter()` (as an alias for `Length()`), `Reverse()`, `Rotate()`, `Scale()`, and `Translate()`
  as member functions to `dip::Polygon`.

- `dip::MomentAccumulator` has a new method `PlainSecondOrder()`, which returns the plain old second
  order central moments, compared to the moment of inertia tensor returned by `SecondOrder()`.

- Added `dip::Image::Mask()`, to apply a mask to an image.

- Added `dip::DimensionArray<>::reverse()` and `dip::PixelSize::Reverse()` to reverse the elements of an array.

- Added `dip::Image::ReverseDimensions()`, which permutes dimensions to reverse their order.

- `dip::Label` has a new string argument, `mode`, which can be set to `"largest"` to return a labeled
  image that only contains the largest connected component in the input image.

- Added `dip::PixelSize::UnitLength()`, `dip::PixelSize::UnitSize()`, `dip::PixelSize::ForcePhysical()`,
  and `dip::PixelSize::SameUnits()` to simplify finding the right units for measurement results.

- Added operators to multiply and divide two `dip::Vertex` objects. They apply an element-wise product.

- `dip::ChainCode::Length` has an optional parameter to include the image boundary pixels in the measurement.

- The `"Perimeter"` feature can be configured: setting the `"include boundary pixels"` parameter to a non-zero
  value causes the perimeter measurement to include pixels at the image boundary.

- `dip::TriangleThreshold()` and `dip::BackgroundThreshold()` have a new parameter `sigma`, which defaults to
  4 to preserve the old behavior.

### Changed functionality

- The deterministic initialization for `dip::GaussianMixtureModel()` is more robust, making the
  initial Gaussians overlap instead of setting their sigma to 1.

- `dip::ConvexHull` no longer holds a `dip::Polygon` as a private member, instead it uses `dip::Polygon`
  as a base class. This makes all of the `dip::Polygon` functionality directly available on the convex
  hull. `dip::ConvexHull::Polygon()` now just returns a base class reference.

- `dip::Units` (and by extension `dip::PhysicalQuantity` and `dip::PixelSize`) now uses the Greek letter
  μ (U+03BC), instead of the legacy symbol µ (U+00B5) for micron (when *DIPlib* is built with Unicode
  enabled). Both symbols are recognized when parsing a string, but the strings generated are now different.

- `dip::BackgroundThreshold()` now determines the half width at half height with sub-sample precision,
  and takes the smoothing of the histogram into account, such that the amount of smoothing should have
  little influence in the computed threshold.

- The "Mu" and "GreyMu" features no longer use the pixel sizes if the units for different image
  dimensions differ. No uses of the tensor make sense if the units for the various dimensions
  don't match, in this case it is more useful to report the tensor in pixel units.

### Bug fixes

- `dip::DrawPolygon2D()`, when drawing filled polygons, would skip the bottom row in the polygon. The
  algorithm is a bit more sophisticated now to properly handle these bottom rows. This also takes care
  of some rounding errors that could be seen for polygons with very short edges.

- `dip::ResampleAt` with a `map` input argument and using `"cubic"` interpolation could overflow,
  yielding unsightly artifacts. See [issue #107](https://github.com/DIPlib/diplib/issues/107).

- Better error messages for some forms of `dip::Image::View::At()`

- `dip::ImageDisplay()` didn't pay attention to the `dim1` and `dim2` parameters if the image was 2D.

- `dip::DefineROI()` was incorrect if the input and output images were the same object.

- `dip::GaussianMixtureModel()` could produce NaN for amplitude, those components now have a zero amplitude.

- The "SolidArea" feature didn't take the pixel size into account. This also caused the "Roundness" feature
  to report wrong values. The "SurfaceArea" feature didn't properly scale the result by the pixel size.
  Several composed features (computed from other features) didn't produce correct values for anisotropic
  pixels ("P2A", "Roundness", and "PodczeckShapes").

- `dip::div_round()` was incorrect, which caused `dip::DirectedPathOpening()` to not use certain
  directions.

- `dip::DirectionalStatisticsAccumulator::StandardDeviation()` returned NaN if the mean was identical to zero.




## Changes to *DIPimage*

### Changed functionality

- `threshold(.., 'triangle')` now accepts a parameter to control how much smoothing to apply.

(See also changes to *DIPlib*.)

### Bug fixes

None, but see bugfixes to *DIPlib*.




## Changes to *PyDIP*

### New functionality

- Added `dip.MakeRegionsConvex2D()`.

- Added `Perimeter()` (as an alias for `Length()`), `Reverse()`, `Rotate()`, `Scale()`, and `Translate()`
  as methods to `dip.Polygon`.

- `dip.Polygon` and `dip.ChainCode` now both have `__len__`, `__getitem__` and `__iter__` methods,
  meaning that they can be treated like lists or other iterables. Previously, the values of `dip.Polygon`
  had to be extracted by conversion to a NumPy array, and the values of `dip.ChainCode` by copying to
  a list when accessing its `codes` property.

- The structure returned by `dip.Moments()` has a new component `plainSecondOrder`, which contains
  the plain old second order central moments, compared to the moment of inertia tensor contained
  in `secondOrder`.

- Added `Mask()` and `ReverseDimensions()` as methods to `dip.Image`.

- Added `dip.ReverseDimensions()`, which reverses the indexing order for images for the remainder of the
  session. Has repercussion on how the `dip.Image` buffer is exposed, buffer protocol objects are
  converted to a `dip.Image`, files are read and written, and how *DIPviewer* displays images.
  It is intended to make indexing into a `dip.Image` match the indexing into the corresponding
  *NumPy* array, which should make it easier to mix calls to *DIPlib* and *scikit-image* in the same
  program. Note that this also causes positive angles to be counter-clockwise instead of clockwise.

- Added the `@` and `@=` operators for `dip.Image` objects. These apply matrix multiplication if both
  operands are non-scalar images. That is, the vector or matrix at each pixel is multiplied by the
  vector or matrix at the corresponding pixel in the other operand. The two image's tensor dimensions
  must be compatible. If one of the operands is a scalar image, the normal element-wise multiplication
  is applied. One of the operands can be a single pixel (a list of numbers).

- Added `dip.MeasurementTool.Configure` to allow measurement features to be configured.

- Added `AspectRatio()`, `SameUnits()`, `UnitLength()`, `UnitSize()`, `ForcePhysical()`,
  and `ApproximatelyEquals()` as methods to `dip.PixelSize`.

### Changed functionality

- Operators overloaded for `dip.Image` objects can use lists of numbers as a second argument, which
  is interpreted as a 0D tensor image (column vector). This makes `img / img[0]` possible.

- When *PyDIPjavaio* fails to load, the error is no longer displayed immediately. Instead, it is
  shown when `dip.ImageRead()` fails. The error message is also a bit more helpful.
  See [issue #106](https://github.com/DIPlib/diplib/issues/106).

- The `__repr__` string for many classes has changed to be more consistent and informative.

- The `*` and `*=` operators have changed meaning, they now always apply element-wise multiplication,
  their previous behavior is now obtained with the new `@` and `@=` operators.
  **NOTE! This breaks backwards compatibility.** To keep old code working that depends on image matrix
  multiplications, you can do
  ```python
  import diplib as dip
  dip.Image.__mul__ = dip.Image.__matmul__
  dip.Image.__rmul__ = dip.Image.__rmatmul__
  dip.Image.__imul__ = dip.Image.__imatmul__
  ```
  But we recommend instead that you update the code to use the right operators.

(See also changes to *DIPlib*.)

### Bug fixes

- `__len__()` now properly returns 0 for an empty (raw) image. This makes `if img` fail for a raw image.

- Fixed a few issues with indexing into image, allowing using a list of coordinates, and allowing assigning
  a scalar value into a multi-channel image.

- `dip.SubpixelMaxima()` and `dip.SubpixelMinima()`, when a mask image was given, didn't work correctly.

- The `**=` operator did the computations correctly, but then assigned `None` to the variable instead of the
  result of the operation. The `**` operator didn't work with a single pixel (a number or a list) on the
  left-hand side.

(See also bugfixes to *DIPlib*.)




## Changes to *DIPviewer*

### New functionality

- Added option to change axis labels.

### Bug fixes

- Automatically close all *PyDIPviewer* windows at program exit to avoid segfault.




## Changes to *DIPjavaio*

None.
