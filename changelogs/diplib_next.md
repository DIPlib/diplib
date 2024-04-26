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

- Added macros `DIP_PARALLEL_ERROR_DECLARE`, `DIP_PARALLEL_ERROR_START` and `DIP_PARALLEL_ERROR_END`
  to simplify writing parallel code with OpenMP that properly handles exceptions.

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

- `dip::PairCorrelation()`, `dip::ProbabilisticPairCorrelation()`, `dip::Semivariogram()`, and `dip::ChordLength()`
  Have been parallelized for `"random"` sampling.

- `dip::GrowRegionsWeighted()` with a `dip::Metric` parameter has been deprecated. The new version of the function
  uses the fast marching algorithm in the grey-weighted distance transform. If both `grey` and `mask` are not
  forged, `dip::EuclideanDistanceTransform()` is called instead of `dip::GreyWeightedDistanceTransform()`.
  This makes the function efficient for when isotropic growing is required. And the function now has a new float
  parameter, `distance` that determines how far the regions are grown.

### Bug fixes

- `dip::PairCorrelation`, `dip::ProbabilisticPairCorrelation`, `dip::Semivariogram`, and `dip::ChordLength`
  did not properly compute step sizes for `"grid"` sampling, effectively ignoring the value of `probes`
  and using all pixels (as if `probes` were set to 0).

- Tensor indexing no longer removed color space information since the fix to the move constructor in 3.4.3.

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

- The *DIPlib* exception classes are now properly bound. This changes the type of the exceptions raised by
  the library in Python: `dip.Error` is the new base class, raised exceptions are either `dip.ParameterError`,
  `dip.RunTimeError` or `dip.AssertionError`. `dip.Error` is derived from `Exception`.

(See also changes to *DIPlib*.)

### Bug fixes

- `dip.viewer.Show()` would cause Python to crash if given a raw image.

(See also bugfixes to *DIPlib*.)




## Changes to *DIPviewer*

### New functionality

### Changed functionality

### Bug fixes

- Trying to display an image with wrong color space information would throw an exception. The image is now seen
  as a generic tensor image.




## Changes to *DIPjavaio*

### New functionality

### Changed functionality

### Bug fixes
