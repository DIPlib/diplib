---
layout: post
title: "Changes DIPlib 3.x.x"
date: 2020-00-00
---

## Changes to *DIPlib*

### New functionality

### Changed functionality

### Bug fixes

- Fixed `dip::IsotropicErosion()` to use the same structuring element size as `dip::IsotropicDilation()`.
  See [discussion #192](https://github.com/DIPlib/diplib/discussions/192).

- Parallel processing code assumed that OpenMP would always create the requested number of threads.
  But this is not the case if the process uses dynamic adjustment of the number of threads (by calling
  `omp_set_dynamic(true)`). See [issue #191](https://github.com/DIPlib/diplib/issues/191).

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
