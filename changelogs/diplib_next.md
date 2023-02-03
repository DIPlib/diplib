---
layout: post
title: "Changes DIPlib 3.x.x"
---

## Changes to *DIPlib*

### New functionality

### Changed functionality

- `dip::Image::At( dip::Image mask )` will throw an exception if the mask has no pixels set. This ensures that a
  `dip::Image::View` always selects at least one pixel, it no longer is possible to create an empty `View` object. 

- `dip::CopyFrom( dip::Image const& src, dip::Image& dest, dip::Image const& srcMask )` returns a raw image
  instead of trying to allocate space for 0 pixels if `srcMask` has no pixels set. Forging an image with 0 pixels
  is not possible.

### Bug fixes

- `dip::Image out = view` threw an exception with the message "Sizes must be non-zero and no larger than
  9223372036854775807" if the `view` was empty. This could happen if `view` was created using a mask with
  no pixels set. This issue has been corrected by making it impossible to create an empty `View` object.  
  Note that `out = img.At( img < 0 )` still throws an exception if `img` has no negative values, because
  the indexing operation is now illegal. The exception thrown now, "The mask image selects no pixels",
  is more useful. 

- The `dip::FileInformation` data structure crated for a NPY file reported the file type was `"NYP"`.



## Changes to *DIPimage*

### New functionality

### Changed functionality

(See also changes to *DIPlib*.)

### Bug fixes

None, but see bugfixes to *DIPlib*.
(See also bugfixes to *DIPlib*.)




## Changes to *PyDIP*

### New functionality

### Changed functionality

- Significantly faster load times ([PR #127](https://github.com/DIPlib/diplib/pull/127), [PR #128](https://github.com/DIPlib/diplib/pull/128)).
  The undocumented variable `dip.hasDIPjavaio` no longer exists. The *DIPjavaio* module is loaded when first used.

- Updated the version of Bio-Formats that is downloaded with `python -m diplib download_bioformats` to 6.11.1

(See also changes to *DIPlib*.)

### Bug fixes

None, but see bugfixes to *DIPlib*.
(See also bugfixes to *DIPlib*.)




## Changes to *DIPviewer*

### New functionality

### Changed functionality

### Bug fixes




## Changes to *DIPjavaio*

### New functionality

### Changed functionality

### Bug fixes
