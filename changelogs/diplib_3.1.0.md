---
layout: post
title: "Changes DIPlib 3.1.0"
---

## Changes to *DIPlib*

### New functionality

- Added `dip::MarginalPercentile()` histogram-based statistics function.

- Added `dip::SplitRegions()`, a function to transform labeled images.

- Added `dip::FlushToZero()`, a function to remove denormal values from an image.

- Added `dip::maximum_gauss_truncation()`, returning the maximum truncation that is useful for Gaussian functions.

- `dip::NeighborList` allows accessing neighbors by index, through the new member functions `Coordinates`,
  `Distance` and `IsInImage`.

- Added `dip::Measurement::SetObjectIDs()`.

- Added `dip::Image::SwapBytesInSample()` to convert from little endian to big endian representation.

- Added `dip::ImageReadNPY()`, `dip::ImageReadNPYInfo()`, `dip::ImageIsNPY()` and `dip::ImageWriteNPY()`, to
  work with NumPy's NPY files, expanding interoperability with Python.

### Changed functionality

- Replaced the use of `std::map` and `std::set` in various functions to `tsl::robin_map` and
  `tsl::robin_set`, which are significantly faster. This affects `dip::Measurement` and functions
  using it (including `dip::MeasurementTool::Measure()`), `dip::GetImageChainCodes()`, `dip::GetObjectLabels()`,
  `dip::Relabel()`, `dip::ChordLength()` and `dip::PairCorrelation()`.

- Improved speed of `dip::Measurement::AddObjectIDs()` and
  `dip::Measurement::operator+()`.

- `dip::Label` is slightly more efficient for 3D and higher-dimensional images; added tests.

- Sped up `dip::MomentAccumulator`.

- `dip::OptimalFourierTransformSize()` has a new option to return a smaller or equal size, rather than
  a larger or equal size, so we can crop an image for efficient FFT instead of padding.

- All the variants of the Gaussian filter (`dip::Gauss()` et al.) now limit the truncation value to avoid
  unnecessarily large filter kernels. `dip::DrawBandlimitedPoint()`, `dip::DrawBandlimitedBall()`,
  `dip::DrawBandlimitedBox()`, `dip::GaussianEdgeClip()` and `dip::GaussianLineClip()` also limit the
  truncation in the same way.

- `dip::ColorSpaceConverter` has a `SetWhitePoint` member function. `dip::ColorSpaceManager::SetWhitePoint`
  calls the `SetWhitePoint` member function for all registered converters. This allows custom converters
  to use the white point as well.

- `dip::ColorSpaceManager::XYZ` is deprecated, use `dip::XYZ` instead.

- `dip::SetNumberOfThreads()` and `dip::GetNumberOfThreads()` set a thread-local value now. Each
  thread should set its own limit for how many threads *DIPlib* can use.

- All functions that used randomness internally but didn't have a `dip::Random` input parameter now do
  have such a parameter. Overloaded functions with the old signature create a default-initialized
  `dip::Random` object and call the function with the new signature. This affects the following functions:
  `dip::ChordLength`, `dip::PairCorrelation`, `dip::ProbabilisticPairCorrelation`, `dip::Semivariogram`,
  `dip::CostesSignificanceTest`, `dip::StochasticWatershed`, `dip::KMeansClustering`, and `dip::Superpixels`.

### Bug fixes

- `dip::DrawPolygon2D()` produced wrong results for filled polygons when vertices were very close together
  (distances smaller than a pixel).

- `dip::ColorSpaceManager` didn't register the ICH and ISH color spaces.

- `dip::Image::ResetNonDataProperties()` incorrectly set the number of tensor elements to 1.

- `dip::Image::Copy()` would incorrectly copy the external interface if the destination image didn't have one defined.




## Changes to *DIPimage*

### New functionality

- Added `dip_image/ftz`, a function to remove denormal values from an image.

### Changed functionality

(See also changes to *DIPlib*.)

### Bug fixes

- `readim` and `writeim`, when using *MATLAB*'s functionality, would work in linear RGB space, instead of sRGB
  like the *DIPlib* functions do.

- `readics` tried to read the file as a TIFF file instead of an ICS file.

- Code like `img * [1,2,3]` converted the second argument into a 0D column vector image. The same code
  with e.g. a 3x3 matrix would convert the matrix to a scalar 3x3 image, rather than a 0D matrix image.
  These now behave more similarly to how they behaved in *DIPimage* 2.9.

(See also bugfixes to *DIPlib*.)




## Changes to *PyDIP*

### New functionality

- Added `dip.FlushToZero()`, `dip.SplitRegions()`.

- Added `dip.ImageReadNPY()`, `dip.ImageReadNPYInfo()`, `dip.ImageIsNPY()` and `dip.ImageWriteNPY()`.

### Changed functionality

- `dip.ChordLength`, `dip.PairCorrelation`, `dip.ProbabilisticPairCorrelation`, `dip.Semivariogram`,
  `dip.CostesSignificanceTest`, `dip.StochasticWatershed`, `dip.KMeansClustering`, and `dip.Superpixels`
  now use the global random number generator instead of creating a default-initialized one.

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

- Signed integer images could not be read.
