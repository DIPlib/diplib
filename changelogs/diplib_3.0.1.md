---
layout: post
title: "Changes DIPlib 3.0.1"
---

## Changes to DIPlib

### New functionality

- Added `dip::MarginalPercentile()` histogram-based statistics function.

### Changed functionality

- `dip::Measurement::ObjectIdToIndexMap()` is much faster by changing `dip::ObjectIdToIndexMap`
  from a `std::map` to a `std::unordered_map`. This makes `dip::MeasurementTool::Measure()` is significantly
  faster.

- Added `dip::Measurement::SetObjectIDs()`, improved speed of `dip::Measurement::AddObjectIDs()` and
  `dip::Measurement::operator+()`.

### Bug fixes

- `dip::DrawPolygon2D()` produced wrong results for filled polygons when vertices were very close together
  (distances smaller than a pixel).

## Changes to DIPimage


## Changes to PyDIP


## Changes to DIPviewer


## Changes to DIPjavaio

### Bug fixes

- Signed integer images could not be read.
