---
layout: post
title: "Changes DIPlib 3.4.3"
date: 2024-03-20
---

## Changes to *DIPlib*

### Bug fixes

- Fixed `dip::Image` move constructor to leave moved-from object in valid state.
  See [issue #149](https://github.com/DIPlib/diplib/issues/149).

- Fixed `dip::RadonTransformCircles()` to avoid out-of-bounds reads.




## Changes to *DIPimage*

### Bug fixes

None, but see bugfixes to *DIPlib*.




## Changes to *PyDIP*

### Bug fixes

- Importing the package won't fail if *DIPviewer* fails to load for whatever reason.

- The Linux package again depends on libGL rather than libOpenGL and libGLX.
  See [issue #148](https://github.com/DIPlib/diplib/issues/148).

- The macOS packages again are multi-threading enabled though OpenMP.

(See also bugfixes to *DIPlib*.)




## Changes to *DIPviewer*

None




## Changes to *DIPjavaio*

None
