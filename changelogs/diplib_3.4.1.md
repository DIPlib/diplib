---
layout: post
title: "Changes DIPlib 3.4.1"
---

## Changes to *DIPlib*

### Changed functionality

- `dip::Image::At( dip::Image mask )` will throw an exception if the mask has no pixels set. This ensures that a
  `dip::Image::View` always selects at least one pixel, it no longer is possible to create an empty `View` object.

- `dip::CopyFrom( dip::Image const& src, dip::Image& dest, dip::Image const& srcMask )` returns a raw image
  instead of trying to allocate space for 0 pixels if `srcMask` has no pixels set. Forging an image with 0 pixels
  is not possible.

- `dip::ImageRead()` produces a better error message if the file doesn't exist.

- Improved error messages when reading or writing JPEG files.

- `dip::DrawPolygon2D()` no longer requires that all vertices be within the image for the non-filled case (the filled
  case never required it).

### Bug fixes

- `dip::Image out = view` threw an exception with the message "Sizes must be non-zero and no larger than
  9223372036854775807" if the `view` was empty. This could happen if `view` was created using a mask with
  no pixels set. This issue has been corrected by making it impossible to create an empty `View` object.  
  Note that `out = img.At( img < 0 )` still throws an exception if `img` has no negative values, because
  the indexing operation is now illegal. The exception thrown now, "The mask image selects no pixels",
  is more useful.

- The `dip::FileInformation` data structure crated for a NPY file reported the file type was `"NYP"`.

- Third and fourth order cubic spline interpolation (methods `"3-cubic"` and `"4-cubic"`) now always use
  double-precision floats internally for computation. The improved precision is necessary to evaluate the third-order
  polynomials with sufficient precision, the errors could be seen in flat areas of the rescaled image.

- `dip::Sharpen()` and `dip::UnsharpMask()` didn't work correctly when the input and output images were the same.

- `dip::BesselJN( 0, 0 )` returned 0.0 instead of 1.0.

- `dip::BinaryPropagation` unwittingly changed behavior in 3.4.0: When `iterations` was 0, the seed pixels outside
  the mask made it into the output image, but shouldn't have. See [issue #135](https://github.com/DIPlib/diplib/issues/135).




## Changes to *DIPimage*

### Changed functionality

None, but see changes to *DIPlib*.

### Bug fixes

None, but see bugfixes to *DIPlib*.




## Changes to *PyDIP*

### Changed functionality

- Significantly faster load times ([PR #127](https://github.com/DIPlib/diplib/pull/127), [PR #128](https://github.com/DIPlib/diplib/pull/128)).
  The undocumented variable `dip.hasDIPjavaio` no longer exists. The *DIPjavaio* module is loaded when first used.

- Updated the version of Bio-Formats that is downloaded with `python -m diplib download_bioformats` to 7.0.0.

(See also changes to *DIPlib*.)

### Bug fixes

None, but see bugfixes to *DIPlib*.




## Changes to *DIPviewer*

None.




## Changes to *DIPjavaio*

### Bug fixes

- Images of more than 2 GB can now be read using Bio-Formats. But each single x-y plane still must be smaller than 2 GB.
  See [issue #133](https://github.com/DIPlib/diplib/issues/133).

- When reading through Bio-Formats, pixel sizes in reported in micron would be ignored with a warning.
  See [issue #133](https://github.com/DIPlib/diplib/issues/133).
