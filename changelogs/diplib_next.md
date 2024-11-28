---
layout: post
title: "Changes DIPlib 3.x.x"
date: 2020-00-00
---

## Changes to *DIPlib*

### New functionality

- `dip::ImageWriteTiff()` now can write 3D images to a TIFF file as a series of slices.
  See [PR #182](https://github.com/DIPlib/diplib/pull/182).

- Added `dip::Graph::EdgeVertex()` for convenience. `graph.EdgeVertex( edge, which )` is the same as what
  previously was written `graph.Edges()[ edge ].vertices[ which ].`

- Added `dip::Graph::IsValidEdge()`.

- Added `dip::Graph::UpdateEdgeWeights<>()`, an overload that takes a function as input; this function
  is applied to the two vertex weights for each edge, and should return an edge weight.

- Added `dip::DirectedGraph`, a directed version of `dip::Graph`.

- Added `dip::GraphCut()`, a function that computes the minimum cut of a directed graph. This is a segmentation
  algorithm that splits the graph into two sections based on two marker vertices (nodes).

- Added `dip::Label()` with a `dip::Graph` and a `dip::DirectedGraph` as input.
  It finds connected components in the graph.

### Changed functionality

- `dip::AlignedAllocInterface` now aligns each of the scanlines (rows of the image), not just the first one.
  This means it can be used to create images that are compatible with some image display widgets (Windows
  bitmap, Qt image, etc.).

- `dip::LowestCommonAncestorSolver` is no longer in the public API. This class contained code used in the
  Exact Stochastic Watershed (`dip::StochasticWatershed` with `seeds` set to `"exact"` or `nIterations` set to 0).

- `dip::MinimumSpanningForest()` is now a free function. The `dip::Graph::MinimumSpanningForest()` class
  function still exists for backwards-compatibility, it calls the free function.

### Bug fixes

- `dip::Log2` computed the natural logarithm instead of the base-2 logarithm.
  See [PR #168](https://github.com/DIPlib/diplib/pull/168).

- `dip::StructureTensorAnalysis3D()` would try to read a non-existing tensor element when requesting the 'l3' output
  (producing an obscure error message).

- `dip::GaussFT()` could, under some circumstances, try to reforge the output image to an intermediate, extended size.
  This would throw an exception if the image was protected, but was always a bad thing to do.
  See [issue #170](https://github.com/DIPlib/diplib/issues/170).

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

- Added the function `dip.Doc()`, which will open the documentation for the given function or class in the
  default web browser.

- Automatically copying the brief section of the documentation to each function as the Python doc string.
  We finally have some documentation in Python!  
  NOTE: We could copy the full documentation in the future, but that requires more extensive Markdown
  parsing to produce good results.

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
