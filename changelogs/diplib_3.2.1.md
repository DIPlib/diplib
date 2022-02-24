---
layout: post
title: "Changes DIPlib 3.2.1"
---

## Changes to *DIPlib*

### New functionality

- Added `dip::MakeRegionsConvex2D()`, which works for both labelled and binary images.

- Added `Perimeter()` (as an alias for `Length()`), `Reverse()`, `Rotate()`, `Scale()`, and `Translate()`
  as member functions to `dip::Polygon`.

- `dip::MomentAccumulator` has a new method `PlainSecondOrder()`, which returns the plain old second
  order central moments, compared to the moment of inertia tensor returned by `SecondOrder()`.

- Added `dip::Image::Mask()`, to apply a mask to an image.

- Added `reverse()` to `dip::DimensionArray<>` and `Reverse()` to `dip::PixelSize`.

- Added `dip::Image::ReverseDimensions()`, which permutes dimensions to reverse their order.

### Changed functionality

### Bug fixes

- `dip::DrawPolygon2D()`, when drawing filled polygons, would skip the bottom row in the polygon. The
  algorithm is a bit more sophisticated now to properly handle these bottom rows. This also takes care
  of some rounding errors that could be seen for polygons with very short edges.

- `dip::ResampleAt` with a `map` input argument and using `"cubic"` interpolation could overflow,
  yielding unsightly artifacts. See [issue #107](https://github.com/DIPlib/diplib/issues/107).

- Better error messages for some forms of `dip::Image::View::At()`

- `dip::ImageDisplay()` didn't pay attention to the `dim1` and `dim2` parameters if the image was 2D.




## Changes to *DIPimage*

### New functionality

### Changed functionality

(See also changes to *DIPlib*.)

### Bug fixes

(See also bugfixes to *DIPlib*.)




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

### Changed functionality

- Operators overloaded for `dip.Image` objects can use lists of numbers as a second argument, which
  is interpreted as a 0D tensor image (column vector). This makes `img / img[0]` possible.

- When PyDIPjavaio fails to load, the error is no longer displayed immediately. Instead, it is
  shown when `dip.ImageRead()` fails. The error message is also a bit more helpful.
  See [issue #106](https://github.com/DIPlib/diplib/issues/106).

- The `__repr__` string for many classes has changed to be more consistent and informative.

(See also changes to *DIPlib*.)

### Bug fixes

- `__len__()` now properly returns 0 for an empty (raw) image. This makes `if img` fail for a raw image.

- Fixed a few issues with indexing into image, allowing using a list of coordinates, and allowing assigning
  a scalar value into a multi-channel image.

- `dip.SubpixelMaxima()` and `dip.SubpixelMinima()`, when a mask image was given, didn't work correctly.

(See also bugfixes to *DIPlib*.)




## Changes to *DIPviewer*

### New functionality

- Added option to change axis labels.

### Changed functionality

### Bug fixes




## Changes to *DIPjavaio*

### New functionality

### Changed functionality

### Bug fixes
