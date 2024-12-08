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

- Added `dip::GraphCut()`, a function that computes the grap-cut segmentation of an image.
  
- Added `dip::Histogram::Configuration::Mode::IS_COMPLETE`, which prevents a configuration from being
  modified when computing a histogram. It is set by `dip::Histogram::Configuration::Complete()`.
  This option is dangerous to use!

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

- The Fourier Transform, when using the default PocketFFT, used a plan cache that was not thread safe. When calling
  any function using a Fourier Transform from multiple threads, a race condition could occur. We've added a mutex
  to the function that maintains the cache to avoid this.

- In 3.5.0, the original version of `dip::GetImageChainCodes()` was deprecated in favor of a new one that takes
  a `std::vector< dip::LabelType >` as input, as opposed to a `dip::UnsignedArray`. This caused code that called
  the function with an initializer array (`dip::GetImageChainCodes( image, { 1 } )`) to become ambiguous. A new
  overload that takes an initializer list as input fixes this ambiguity.

### Updated dependencies

### Build changes

- The documentation building target was renamed to "doc" (as "apidoc"). This target builds much more documentation
  than just the DIPlib API documentation (the DIPimage and PyDIP user manuals, build instructions, etc.).




## Changes to *DIPimage*

### New functionality

- The function documentation now has links to the online DIPlib documentation. Instead of only
  naming the DIPlib functions used, the function names are now hyperlinks.

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

- The function `dip.MaximumAndMinimum()` now returns a `namedtuple` of type `MinMaxValues` instead of a plain `tuple`.
  It behaves in the same way as the previous tuple, but the two values can additionally (and preferably) be accessed
  using the dot notation: `mm.maximum` instead of `mm[1]`.
  See [issue #184](https://github.com/DIPlib/diplib/issues/184).

- Likewise, the functions `dip.MandersColocalizationCoefficients()` and `dip.CostesColocalizationCoefficients()`
  now return a `namedtuple` of type `ColocalizationCoefficients` instead of a plain `tuple`.

- Likewise, the functions `dip.ChainCode.BoundingBox()` and `dip.Polygon.BoundingBox()` now return a `namedtuple`
  of type `BoundingBoxInteger` and `BoundingBoxFloat` respectively, which contain two `namedtuple`s of type
  `VertexInteger` or `VertexFloat`. Again, these types mimic the old `tuple` outputs, but are self-documenting and
  easier to use.

- Likewise, other `dip.Polygon` functions such as `dip.Polygon.Centroid()` now return a `namedtuple` of type `VertexFloat`
  instead of a plain `tuple`.

- Likewise, `dip.ChainCode.start` is now a `namedtuple` of type `VertexInteger` instead of a plain `tuple`.

- The types `SubpixelLocationResult`, `RadonCircleParameters`, `RegressionParameters`, `GaussianParameters`,
  `FeatureInformation`, `ValueInformation`, `EllipseParameters`, `FeretValues`, `RadiusValues`, `QuartilesResult`,
  `StatisticsValues`, `CovarianceValues`, `MomentValues` and `SpatialOverlapMetrics`, all used only as outputs to
  functions, and all simply emulating a C++ `struct`, are no longer special types in the `diplib.PyDIP_bin` namespace,
  but `namedtuple`s. They new types behave identically, but can additionally be unpacked, for example:
  `_, q1, _, q3, _ = dip.Quartiles(img)`.

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
