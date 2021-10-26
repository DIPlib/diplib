---
layout: post
title: "Changes DIPlib 3.2.0"
---

## Changes to *DIPlib*

### New functionality

- Added `dip::BlendBandlimitedMask()`.

- Added `dip::FreeTypeTool` to render text into images. It depends on the FreeType library to use any font file
  for rendering the text. This functionality is opt-in (that is, by default it is not enabled).

- Added `dip::DrawText()` to render text into images using a built-in, fixed-size font.

- `dip::Image::Pad()` can now pad with an arbitrary pixel value.

### Changed functionality

### Bug fixes

- When inserting a `dip::Measurement` object into a stream, the table it produced sometimes had feature value columns
  narrower than the feature name header.

- Fixed bug that prevented compilation with *MinGW*.



## Changes to *DIPimage*

### New functionality

### Changed functionality

(See also changes to *DIPlib*.)

### Bug fixes

- Fixed build script to allow building with newest version of *MATLAB*.

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
