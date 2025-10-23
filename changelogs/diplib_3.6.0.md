---
layout: post
title: "Changes DIPlib 3.6.0"
date: 2025-10-23
---

## Changes to *DIPlib*

### New functionality

- Added `dip::RankFromPercentile()`, which computes the rank (or index into the sorted array) for the given
  percentile and array size. This was a computation done in many different functions in the library.

- Added `dip::Polygon::FitCircle()` and `dip::Polygon::FitEllipse()`. The former returns a new data structure
  `dip::CircleParameters`.

- Added `dip::Polygon::Contains()` to test if a point is inside the polygon.

- Added `dip::ColorMapLut()`, expanding the functionality of `dip::ApplyColorMap()`.

- Added `dip::ThinPlateSpline::Dimensionality()` and `dip::ThinPlateSpline::NumberOfControlPoints()` to provide
  some information about the `dip::ThinPlateSpline` object.

- Added the `"sRGBA"` color space. Converting from it simply discards the alpha channel. It is intended to allow
  4-channel PNG files to be interpreted correctly.

- Added `dip::AlphaBlend()`, `dip::AlphaMask()` and `dip::ApplyAlphaChannel()`.

- Added `dip::ContainsNotANumber()`, `dip::ContainsInfinity()` and `dip::ContainsNonFiniteValue()`, which are
  shortcuts to (and potentially faster than) `dip::Any(dip::IsNotANumber(...)).As<bool>()`,
  `dip::Any(dip::IsInfinite(...)).As<bool>()` and `dip::ContainsNotANumber(...) || dip::ContainsInfinity(...)`.

- Added `dip::Histogram::Configuration::IsInRange()`.

- `dip::Eigenvalues()`, `dip::LargestEigenvalue()`, `dip::SmallestEigenvalue()` and `dip::EigenDecomposition()`
  add a `method` argument, allowing the caller to select a faster, but potentially less precise method, when
  computing the eigendecomposition of a real-valued, symmetric, 2x2 or 3x3 tensor image.
  The low-level functions `dip::SymmetricEigenDecomposition2()` and `dip::SymmetricEigenDecomposition3()` add
  the same argument, but as an enum rather than a string.

- `dip::CreateGauss()` has a new argument, `extent`, which defaults to `"full"`. When set to `"half"`, the output
  is the first half (in 1D) or quadrant (in 2D) of the Gaussian kernel.

### Changed functionality

- All functions that compute a percentile (`dip::Percentile()`, `dip::PercentilePosition()`,
  `dip::MarginalPercentile()`, `dip::Quartiles()`, and `dip::PercentileFilter()`) now use the new function
  `dip::RankFromPercentile()` to find out which value from the sorted input set to return.
  The computation in most cases has not changed other than using a different rounding when the percentile falls
  exactly half-way between two values. Previously the rounding for these cases was always up.
  The new function rounds up if the percentile is 50% or below, or down if it's larger.
  This ensures a symmetric treatment of percentiles.
  In the case of `dip::MarginalPercentile()`, and the case `dip::Percentile()` applied to a measurement feature,
  the computation was not consistent with the other percentile computations.

- `dip::ChainCode::Polygon()` has a new, optional argument to exclude border pixels from the polygon.

- The overloads for `dip::MinimumVariancePartitioning()` and `dip::KMeansClustering()` that take a histogram
  as input now also have a version that take the output histogram as an argument, and return the cluster centers.
  This version of `dip::KMeansClustering()` additionally has overloads that take a `dip::Random` object as input,
  instead of using a default-initialized one.

- `dip::CovarianceMatrix::EllipseParameters` is now `dip::EllipseParameters`, and adds a member `center` to encode
  the coordinates of the center of the ellipse. This is the data structure returned by `dip::CovarianceMatrix::Ellipse()`,
  which does not fill in the new `center` value. This new value is useful for the new function `dip::Polygon::FitEllipse()`.
  An alias `dip::CovarianceMatrix::EllipseParameters` exists to avoid existing code breaking.

- The `dip::LibraryInformation` struct now has a series of boolean values that encode the compilation settings, so
  it no longer is necessary to decode the `type` field to find out if a certain feature is available or not.

- `dip::Polygon`, `dip::ConvexHull`, and the support structs that they depend on are now declared in `diplib/polygon.h`.
  `diplib/chain_code.h` includes this new file, so old code will continue to work.

- `dip::ThinPlateSpline` functions now check their input, so that this tool can be used in other contexts outside
  `dip::WarpControlPoints()`. A new function `dip::ThinPlateSpline::EvaluateUnsafe()` provides the old unchecked,
  slightly faster evaluation. `dip::ThinPlateSpline::Evaluate()` is now marked `const`.

- `dip::Percentile()`, `dip::PositionPercentile()` and `dip::Quartiles()` ignore NaN values. Note that `dip::Median()`
  and `dip::PositionMedian()` are simple interfaces to the percentile functions, and therefore now also ignore NaN values.

- `dip::Histogram` ignores NaN values when constructing a histogram. When including out-of-bounds values, NaN values
  are added to the lower bin.

- `dip::Histogram::Configuration::FindBin()` is overloaded for different data types, and properly handles NaN values.
  `dip::Histogram::Bin()` also properly handles NaN values.

### Bug fixes

- Fixed `dip::IsotropicErosion()` to use the same structuring element size as `dip::IsotropicDilation()`.
  See [discussion #192](https://github.com/DIPlib/diplib/discussions/192).

- Parallel processing code assumed that OpenMP would always create the requested number of threads.
  But this is not the case if the process uses dynamic adjustment of the number of threads (by calling
  `omp_set_dynamic(true)`). See [issue #191](https://github.com/DIPlib/diplib/issues/191).

- `dip::MinimumVariancePartitioning()` could throw an unintended exception with specific unimodal inputs.
  This was caused by the use of the sum of variances, instead of the sum of weighted variances as required
  with the Otsu threshold logic. See [discussion #193](https://github.com/DIPlib/diplib/discussions/193).

- `dip::MinimumVariancePartitioning()` now gracefully handles the case where more clusters are requested
  than can be generated.

- `dip::GetImageChainCodes()` and `dip::GetSingleChainCode()` marked a chain code as being on the image border
  if the end pixel was on the image border, instead of only if the step represented by the code is along the
  image border.

- `dip::ImageReadPNG()` reads in 4-channel images as `"sRGBA"`, not as `"sRGB"` as it did previously.

- `dip::Histogram` should no longer crash on construction when the input has NaN values.

- `dip::EdgeObjectsRemove()` with a binary image as input did not preserve the pixel sizes.

### Build changes

- For building the documentation, it no longer is necessary to set the `DOXPP_*_EXECUTABLE` CMake variables,
  there is now a single `DIP_DOXPP_PATH` that will tell the CMake where to find all four programs.




## Changes to *DIPimage*

None, but see changes to *DIPlib*.




## Changes to *PyDIP*

### New functionality

- Added bindings for `dip::CovarianceMatrix`.

- Added bindings for some `dip::Polygon` member functions: `CovarianceMatrixVertices()`,
  `CovarianceMatrixSolid()`, and the overloads of `RadiusStatistics()` and `EllipseVariance()`
  that take a centroid as input argument.

- Added bindings for `dip::ThinPlateSpline`.

- Added bindings for `dip::pi`, `dip::nan` and `dip::infinity`. These constants are available in other
  packages in Python, but might be convenient to have here too.

### Changed functionality

- The `dip.Image` constructor no longer takes a pixel value as input. This means that `dip.Image(256)` is now the same
  as `dip.Image((256,))`. Previously, the former created a 0D scalar image with the value 256, and the latter created
  an uninitialized 1D image with 256 pixels. This was inconsistent and confusing. It could be surprising when a scalar
  value input to a *DIPlib* function was accepted in the place of an image instead of producing a useful error message.
  One now always needs to use `dip.Create0D()` to create a scalar image (but one can still convert a scalar NumPy array).  
  **NOTE! This breaks backwards compatibility.**

- `dip.libraryInformation` is now a `namedtuple`. There should be no compatibility concerns with this change,
  except that it prints differently.

(See also changes to *DIPlib*.)

### Bug fixes

- `dip::VertexFloat` input arguments were required to be a sequence with two float values.
  Integer values are now also accepted.

- `dip.LookupTable.Apply()` overload resolution fixed so that a scalar input is not converted to an image.

(See also bugfixes to *DIPlib*.)

### Build and installation changes

- Official packages for Python 3.14 added, support for Python 3.10 dropped.




## Changes to *DIPviewer*

### Changed functionality

- Creating multiple windows with `dip::viewer::Show()` or `dip::viewer::ShowSimple()` will arrange them across
  the screen instead of placing them all at the same location. The arrangement is simplistic, assumes a single
  screen, and assumes default window sizes. But the result should be more user-friendly than the previous behavior.




## Changes to *DIPjavaio*

None.
