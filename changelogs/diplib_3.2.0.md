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

- `dip::Histogram` now has a stream output operator (`<<`), which shows basic information about the histogram.

### Changed functionality

- The output stream operator for `dip::Image` writes the same information in a slightly different way.

### Bug fixes

- When inserting a `dip::Measurement` object into a stream, the table it produced sometimes had feature value columns
  narrower than the feature name header.

- Fixed bug that prevented compilation with *MinGW*.

- The documentation was missing a lot of functions in the arithmetic, comparison and trigonometric operators modules,
  as well as the macros in the *DIPlib*-*MATLAB* interface.




## Changes to *DIPimage*

### New functionality

### Changed functionality

(See also changes to *DIPlib*.)

### Bug fixes

- Fixed build script to allow building with newest version of *MATLAB*.

- Updated `dipmex` to work properly with the *MinGW* compiler on Windows, and with older versions of *MATLAB*.

(See also bugfixes to *DIPlib*.)




## Changes to *PyDIP*

### New functionality

- Added Python 3.10 to the list of deployed wheels.

### Changed functionality

(See also changes to *DIPlib*.)

### Bug fixes

- Fixed memory corruption bug when releasing data from a different thread.

(See also bugfixes to *DIPlib*.)




## Changes to *DIPviewer*

### New functionality

### Changed functionality

### Bug fixes

- Fixed an issue that could cause a segmentation violation after all the *DIPviewer* windows were closed.

- Fixed an issue that prevented Visual Studio from building *PyDIPviewer*.




## Changes to *DIPjavaio*

### New functionality

### Changed functionality

### Bug fixes
