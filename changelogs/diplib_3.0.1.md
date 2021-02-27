---
layout: post
title: "Changes DIPlib 3.0.1"
---

## Changes to DIPlib

### New functionality

- Added `dip::MarginalPercentile()` histogram-based statistics function.

### Changed functionality

### Bug fixes

- `dip::DrawPolygon2D()` produced wrong results for filled polygons when vertices were very close together
  (distances smaller than a pixel).

## Changes to DIPimage


## Changes to PyDIP


## Changes to DIPviewer


## Changes to DIPjavaio

### Bug fixes

- Signed integer images could not be read.
