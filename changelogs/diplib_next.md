---
layout: post
title: "Changes DIPlib 3.x.x"
---

## Changes to *DIPlib*

### New functionality

- Added `dip::OutputBuffer` as a virtual base class, and `dip::SimpleOutputBuffer` and `dip::FixedOutputBuffer`
  as specific instances. These are used to handle a buffer that a JPEG or PNG file can be written into by
  `dip::ImageWriteJPEG()` and `dip::ImageWritePNG()`.

### Changed functionality

- The `compressionLevel` argument to `dip::ImageWritePNG()` changed from `dip::uint` to `dip::sint`, allowing for
  -1 to configure the deflate algorithm to use RLE instead of its default strategy. For some images this option
  can lead to smaller files, and for some images to much faster compression.

- The versions of `dip::ImageWriteJPEG()` and `dip::ImageWritePNG()` that write to a memory buffer now take
  a version of a `dip::OutputBuffer` object to write to. Overloaded versions of these two functions maintain
  the previous syntax where the buffer is created internally and returned to the caller.

### Bug fixes

### Updated dependencies




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
