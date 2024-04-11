---
layout: post
title: "Changes DIPlib 3.x.x"
---

## Changes to *DIPlib*

### New functionality

- Added `dip::OutputBuffer` as a virtual base class, and `dip::SimpleOutputBuffer` and `dip::FixedOutputBuffer`
  as specific instances. These are used to handle a buffer that a JPEG or PNG file can be written into by
  `dip::ImageWriteJPEG()` and `dip::ImageWritePNG()`.

- Added `dip::Framework::Projection()` as a public function. Previously it was hidden in the library for internal
  use only. This framework function simplifies the creation of projection operations, and forms the basis of
  functions such as `dip::Mean()` and `dip::Maximum()`.

### Changed functionality

- The `compressionLevel` argument to `dip::ImageWritePNG()` changed from `dip::uint` to `dip::sint`, allowing for
  -1 to configure the deflate algorithm to use RLE instead of its default strategy. For some images this option
  can lead to smaller files, and for some images to much faster compression.

- The versions of `dip::ImageWriteJPEG()` and `dip::ImageWritePNG()` that write to a memory buffer now take
  a version of a `dip::OutputBuffer` object to write to. Overloaded versions of these two functions maintain
  the previous syntax where the buffer is created internally and returned to the caller.

- `dip::Framework::Projection()` is now parallelized. All projection functions (for example `dip::Mean()` and
  `dip::Maximum()`) now can use multithreading for faster computation if the output has at least one dimension.
  For example, the mean value of each image row can be computed in parallel, but the mean value over the whole
  image is still not computed in parallel.

### Bug fixes

### Updated dependencies




## Changes to *DIPimage*

### New functionality

- Added the CMake option `DIP_JAVA_VERSION`, which can be important when building *DIPimage* for older versions
  of MATLAB. The Java version must be equal or older than the version used by MATLAB. Running `version -java`
  in MATLAB will tell you what version of Java it is using.

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
