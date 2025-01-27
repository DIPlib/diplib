---
layout: post
title: "Changes DIPlib 3.x.x"
date: 2020-00-00
---

## Changes to *DIPlib*

### New functionality

- Added `dip::RankFromPercentile()`, which computes the rank (or index into the sorted array) for the given
  percentile and array size. This was a computation done in many different functions in the library.

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

### Updated dependencies

### Build changes




## Changes to *DIPimage*

### New functionality

### Changed functionality

(See also changes to *DIPlib*.)

### Bug fixes

None, but see bugfixes to *DIPlib*.
(See also bugfixes to *DIPlib*.)

### Build and installation changes




## Changes to *PyDIP*

### New functionality

### Changed functionality

(See also changes to *DIPlib*.)

### Bug fixes

None, but see bugfixes to *DIPlib*.
(See also bugfixes to *DIPlib*.)

### Build and installation changes




## Changes to *DIPviewer*

### New functionality

### Changed functionality

### Bug fixes

### Build changes




## Changes to *DIPjavaio*

### New functionality

### Changed functionality

### Bug fixes

### Build changes
