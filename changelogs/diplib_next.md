---
layout: post
title: "Changes DIPlib 3.x.x"
date: 2020-00-00
---

## Changes to *DIPlib*

### New functionality

- Added the *DIPlib*-*Qt* interface in the header file `dip_qimage_interface.h`, with namespace `dip_qimage`.
  See [PR #233](https://github.com/DIPlib/diplib/pull/233).

- Added new measurement feature `"CentroidDiameter"`.
  See [PR #232](https://github.com/DIPlib/diplib/pull/232).

- Measurement features can now have aliases. The alias can be used anywhere of its canonical name to refer
  to the feature, and one does not need to be consistent about which of the names is used.

### Changed functionality

- The "Mass" measurement feature is renamed to "Sum", which should reduce confusion. "Mass" is an alias,
  so old code will continue to work as before.

### Bug fixes

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
