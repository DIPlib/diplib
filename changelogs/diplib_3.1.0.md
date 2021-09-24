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

- Compound assignment operators defined for image views (`img[ 0 ] += ...` now works).

- Added `dip::GetLabelBoundingBox()`.

- Added `dip::IsotropicDilation()`, `dip::IsotropicErosion()`, `dip::IsotropicClosing()` and `dip::IsotropicOpening()`
  for binary images.

- Added `dip::Image::Sample::Maximum()` and `dip::Image::Sample::Minimum()`.

- Added `dip::ImposeMinima()`.

- Added `dip::Toggle()`, another variant to `dip::Select()`.

- Added `dip::MorphologicalSharpening()`, based on `dip::Toggle()`.

- It is now possible to construct a default-initialized `dip::Histogram`, which can only be assigned to.
  The new function `dip::Histogram::IsInitialized()` returns false for such an object.

### Changed functionality

- Replaced the use of `std::map` and `std::set` in various functions to `tsl::robin_map` and
  `tsl::robin_set`, which are significantly faster. This affects `dip::Measurement` and functions
  using it (including `dip::MeasurementTool::Measure()`), `dip::GetImageChainCodes()`, `dip::GetObjectLabels()`,
  `dip::Relabel()`, `dip::ChordLength()` and `dip::PairCorrelation()`.

- The speed of the following functions has been improved:
    - `dip::Measurement::AddObjectIDs()` and `dip::Measurement::operator+()`.
    - `dip::Label()` (slightly more efficient for 3D and higher-dimensional images).
    - `dip::MomentAccumulator`.
    - `dip::Dilation()` and the other basic morphological operators with a 2D diamond or octagonal structuring element,
      or with a small 2D disk.
    - `dip::MorphologicalReconstruction()` and functions that depend on it.
    - `dip::SeededWatershed()` and `dip::CompactWatershed()`, and functions that depend on them.
    - `dip::Thickening()` and `dip::Thinning()`.

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
  `dip::ChordLength()`, `dip::PairCorrelation()`, `dip::ProbabilisticPairCorrelation()`, `dip::Semivariogram()`,
  `dip::CostesSignificanceTest()`, `dip::StochasticWatershed()`, `dip::KMeansClustering()`, and `dip::Superpixels()`.

- `dip::Image::Rotation90()` can now be called without any arguments at all.

### Bug fixes

- `dip::Image::ResetNonDataProperties()` incorrectly set the number of tensor elements to 1.

- `dip::Image::Copy()` would incorrectly copy the external interface if the destination image didn't have one defined.

- `dip::Image::HasSameDimensionOrder()` didn't properly ignore singleton dimensions.

- It was possible to forge a new image with normal strides for which `dip::Image::HasNormalStrides()` was false.

- `dip::DrawPolygon2D()` produced wrong results for filled polygons when vertices were very close together
  (distances smaller than a pixel).

- `dip::ColorSpaceManager` didn't register the ICH and ISH color spaces.

- `dip::MaximumPixel()` and `dip::MinimumPixel()` produced an empty output array when the input was all NaN.

- `dip::GaussianMixtureModelThreshold()` passed wrong parameter values to `dip::GaussianMixtureModel()`.

- `dip::ResampleAt(in, map)` didn't use the pixels at the right or bottom edges of the input image.

- Fixed various issues in underlying code for `dip::AreaOpening()`, `dip::AreaClosing()`, `dip::VolumeOpening()` and `dip::VolumeClosing()`.
  These functions now behave as described in the documentation.

- There was a strange rounding error when creating disk-shaped filter kernels and structuring elements,
  for some even integer sizes, which caused these kernels to be not symmetric over 90 degree rotations.

- `dip::MeanAbs()` and `dip::SumAbs()` could produce wrong results for complex inputs.

- libics had a typo that caused out-of-bounds read (#81). 

- Fixed error handling in `dip::ImageReadJPEG()` and `dip::ImageWriteJPEG()`, which previously would crash when libjpeg produced an error (#80).

- Improved error messages in `dip::ImageRead()`.

- `dip::Closing()` and `dip::Opening()` for elliptic and custom structuring elements now have a better behavior at the
  image border.

- Morphological operations with the `"fast line"` structuring element triggered an assertion for some lines in 3D and
  higher dimensions.

- Morphological operations with the isotropic `"diamond"` structuring element in 3D and higher dimensions was implemented
  using a set of line structuring elements, but their composition didn't lead to a ball in city-block distance.

- `dip::MorphologicalThreshold()`, `dip::MorphologicalGist()`, `dip::MorphologicalRange()`, `dip::MorphologicalSmoothing()`,
  `dip::MultiScaleMorphologicalGradient()` and `dip::MorphologicalLaplace()` all produced wrong output for some combinations
  of input image data types and option settings.

- Avoid the C++ Standard Library generating an assertion error in debug mode when generating a Poisson-distributed random
  value with a non-positive mean.




## Changes to *DIPimage*

### New functionality

- Added `dip_image/ftz`, a function to remove denormal values from an image.

### Changed functionality

- Most functions now also accept a string array instead of char vector or a cell array of char vectors
  (that is, `"foo"` is now interpreted the same as `'foo'`, and `["foo","bar"]` is now interpreted the
  same as `{'foo','bar'}`).

- `mat2im` also accepts -1 as second argument, taking the last array dimension as the tensor dimension.

(See also changes to *DIPlib*.)

### Bug fixes

- `readim` and `writeim`, when using *MATLAB*'s functionality, would work in linear RGB space, instead of sRGB
  like the *DIPlib* functions do.

- `readics` tried to read the file as a TIFF file instead of an ICS file.

- Code like `img * [1,2,3]` converted the second argument into a 0D column vector image. The same code
  with e.g. a 3x3 matrix would convert the matrix to a scalar 3x3 image, rather than a 0D matrix image.
  These now behave more similarly to how they behaved in *DIPimage* 2.9.

- Assignments into multiple tensor dimensions like `img{2:3} = [4,5]` or `img{2:3} = gaussf(img{2:3})` produced
  an error message.

- `im2mat` again puts the tensor dimension (and color channels) as the last dimension of the output array, as
  it did in *DIPimage* 2.9.

(See also bugfixes to *DIPlib*.)




## Changes to *PyDIP*

### New functionality

- Added `dip.FlushToZero()`, `dip.SplitRegions()`, `dip.ApplyWindow()`.

- Added `dip.ImageReadNPY()`, `dip.ImageReadNPYInfo()`, `dip.ImageIsNPY()` and `dip.ImageWriteNPY()`.

- Added `dip.FourierMellinMatch2Dparams()`, identical to `dip.FourierMellinMatch2D()` but it also returns the
  transform parameters (which `dip.FourierMellinMatch2D()` should have done from the beginning).

- Added `dip.GetLabelBoundingBox()`.

- Added `dip.IsotropicDilation()`, `dip.IsotropicErosion()`, `dip.IsotropicClosing()` and `dip.IsotropicOpening()`
  for binary images.

- Added `dip.ImposeMinima()`, `dip.Toggle()` and `dip.MorphologicalSharpening()`.

- Added `dip.libraryInformation` and `dip.__version__`.

### Changed functionality

- `dip.ChordLength`, `dip.PairCorrelation`, `dip.ProbabilisticPairCorrelation`, `dip.Semivariogram`,
  `dip.CostesSignificanceTest`, `dip.StochasticWatershed`, `dip.KMeansClustering`, and `dip.Superpixels`
  now use the global random number generator instead of creating a default-initialized one.

(See also changes to *DIPlib*.)

### Bug fixes

- Updated default connectivity, end pixel condition, and boundary condition values for some functions in the
  morphology and binary image processing modules to match the defaults in *DIPlib*.

- Updated Linux wheels to correctly package FreeGLUT

(See also bugfixes to *DIPlib*.)




## Changes to *DIPjavaio*

### Bug fixes

- Signed integer images could not be read.
