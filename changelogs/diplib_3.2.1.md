---
layout: post
title: "Changes DIPlib 3.2.1"
---

## Changes to *DIPlib*

### New functionality

### Changed functionality

- `dip::MomentAccumulator` has a new method `PlainSecondOrder()`, which returns the plain old second
  order central moments, compared to the moment of inertia tensor returned by `SecondOrder()`.

### Bug fixes




## Changes to *DIPimage*

### New functionality

### Changed functionality

(See also changes to *DIPlib*.)

### Bug fixes

(See also bugfixes to *DIPlib*.)




## Changes to *PyDIP*

### New functionality

### Changed functionality

- The structure returned by `dip.Moments()` has a new component `plainSecondOrder`, which contains
  the plain old second order central moments, compared to the moment of inertia tensor contained
  in `secondOrder`. 

- Operators overloaded for `dip.Image` objects can use lists of numbers as a second argument, which
  is interpreted as a 0D tensor image (column vector). This makes `a / a[0]` possible.

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
