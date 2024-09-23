---
layout: post
title: "Changes DIPlib 3.x.x"
date: 2020-00-00
---

## Changes to *DIPlib*

### New functionality

### Changed functionality

- `dip::AlignedAllocInterface` now aligns each of the scanlines (rows of the image), not just the first one.
  This means it can be used to create images that are compatible with some image display widgets (Windows
  bitmap, Qt image, etc.).

### Bug fixes

- `dip::Log2` computed the natural logarithm instead of the base-2 logarithm. #168
  See [PR #168](https://github.com/DIPlib/diplib/pull/168).

- `dip::StructureTensorAnalysis3D()` would try to read a non-existing tensor element when requesting the 'l3' output
  (producing an obscure error message).

### Updated dependencies

### Build changes




## Changes to *DIPimage*

### New functionality

### Changed functionality

- The `'FileWriteWarning'` setting now also applies to the warning regarding conversion to `uint8` when
  `imwrite` delegates to MATLAB's built-in image writing capability. A warning is no longer produced if
  the image already was `uint8`.

(See also changes to *DIPlib*.)

### Bug fixes

- The `dip_image` constructor (and consequently some functions such as `newim`) again accept
  some data type aliases that existed in DIPimage 2.x (`'bin8'`, `'int'`, `'uint'`, `'float'`, `'complex'`).
  These are not terribly useful, but there's no reason not to accept them.

- When `imread` delegated to MATLAB's built-in image file reading capability, it failed to tag
  CMYK images as such.

(See also bugfixes to *DIPlib*.)

### Build and installation changes

- Ported and improved the old *DIPimage* test suite, which is not (yet?) run automatically with `make test`
  because it requires *DIPimage* to be installed.




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

- The Java interface used `Native.loadLibrary()`, which was deprecated. It now uses `Native.load()` instead.

### Build changes




## Changes to *DIPjavaio*

### New functionality

### Changed functionality

### Bug fixes

### Build changes
