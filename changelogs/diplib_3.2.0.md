---
layout: post
title: "Changes DIPlib 3.2.0"
date: 2022-02-08
---

## Changes to *DIPlib*

### New functionality

- Added `dip::BlendBandlimitedMask()`.

- Added `dip::FreeTypeTool` to render text into images. It depends on the FreeType library to use any font file
  for rendering the text. This functionality is opt-in (that is, by default it is not enabled).

- Added `dip::DrawText()` to render text into images using a built-in, fixed-size font.

- `dip::Image::Pad()` can now pad with an arbitrary pixel value.

- `dip::Histogram` now has a stream output operator (`<<`), which shows basic information about the histogram.

- Added `dip::ObjectMaximum()` and `dip::ObjectMinimum()` to identify the object with the largest or smallest
  measurement result.

- Added `dip::Image::UnexpandSingletonDimension()` and `dip::Image::UnexpandSingletonTensor()`.

- Added `dml::IsString()` to the *DIPlib--MATLAB* interface.

### Changed functionality

- The output stream operator for `dip::Image` writes the same information in a slightly different way.

- `dip::Histogram::Bin( FloatArray const& value )` now takes the input as a const reference.

- The scan framework will preserve expanded singleton dimensions: if one dimension is singleton-expanded across
  all input images, then the output images will have the same dimension be singleton-expanded.
  See [issue #96](https://github.com/DIPlib/diplib/issues/96).

- The `dip::libraryInformation` struct now also shows whether *FFTW* and *FreeType* are being used.

### Bug fixes

- When inserting a `dip::Measurement` object into a stream, the table it produced sometimes had feature value columns
  narrower than the feature name header.

- Fixed addition and subtraction operator for `dip::Histogram` changing the LHS operand: `h3 = h1 + h2` would
  end up with `h1` equal to `h3`.

- Fixed bug that prevented compilation with *MinGW*. See [issue #90](https://github.com/DIPlib/diplib/issues/90).

- The documentation was missing a lot of functions in the arithmetic, comparison and trigonometric operators modules,
  as well as the macros in the *DIPlib*-*MATLAB* interface.

- `dip::Distribution::Y` and `dip::Distribution::Ybegin` didn't throw an exception when attempting to access one
  past the last y value.

- `dip::Distribution` now defaults to no x-units (rather than pixels), making `dip::PerObjectHistogram` correctly
  not have units for the x-axis, which is intensity, not pixels.

- `dip::GreyWeightedDistanceTransform` didn't properly use pixel sizes as intended with the default `"fast marching"`
  method.

- Updated DocTest to version 2.4.8, which should allow building and running the tests on Apple Silicon devices.




## Changes to *DIPimage*

### New functionality

- Added `drawtext` function.

### Changed functionality

- In the DIPimage GUI, "About DIPimage" and "About DIPlib" show the same URL (using HTTPS) as a link.
  The "About DIPlib" page also reports whether *FFTW* is used.

- `readim` and `writeim` recognize `'NPY'` as a format. This is the NumPy array format supported natively by *DIPlib*.

- `bdilation`, `berosion`, `bclosing` and `bopening` have a new option `'isotropic'`.

(See also changes to *DIPlib*.)

### Bug fixes

- Fixed build script to allow building with newest version of *MATLAB*.
  See [issue #88](https://github.com/DIPlib/diplib/issues/88).

- Updated `dipmex` to work properly with the *MinGW* compiler on Windows, and with older versions of *MATLAB*.

(See also bugfixes to *DIPlib*.)




## Changes to *PyDIP*

### New functionality

- Added Python 3.10 to the list of deployed wheels.

- Added `dip.Histogram` as a class, matching *DIPlib*'s `dip::Histogram`. This gives a lot more flexibility
  when creating the histogram and when working with histograms. Most of the class methods from *DIPlib* have
  been made accessible from Python, as have all the histogram processing and analysis functions:
    - `dip.Histogram.Configuration` class.
    - New `dip.Histogram` class methods: `IsInitialized()`, `Copy()`, `ReverseLookup()`, `Dimensionality()`,
      `Bins()`, `BinSize()`, `LowerBound()`, `UpperBound()`, `BinBoundaries()`, `BinCenters()`, `BinCenter()`,
      `Bin()`, `GetImage()`, `Count()`, `Cumulative()`, `GetMarginal()`, `Smooth()`.
    - `dip.Histogram` addition and subtraction operators, and indexing operators.
    - New functions `dip.CumulativeHistogram()`, `dip.Smooth()`, `dip.Mean()`, `dip.Covariance()`,
      `dip.MarginalPercentile()`, `dip.MarginalMedian()`, `dip.Mode()`, `dip.PearsonCorrelation()`,
      `dip.Regression()`, `dip.MutualInformation()`, `dip.Entropy()`, `dip.IsodataThreshold()`,
      `dip.OtsuThreshold()`, `dip.MinimumErrorThreshold()`, `dip.GaussianMixtureModelThreshold()`,
      `dip.TriangleThreshold()`, `dip.BackgroundThreshold()`, `dip.KMeansClustering()`,
      `dip.MinimumVariancePartitioning()`, `dip.EqualizationLookupTable()`, `dip.MatchingLookupTable()`.
    - New function `dip.PerObjectHistogram()`.

- Added `dip.HistogramShow()` and `dip.Histogram.Show()` to visualize histograms with proper axis scaling
  and tick labels.

- Added `dip.LookupTable` as a class, matching *DIPlib*'s `dip::LookupTable`.

- Added `dip.ObjectMaximum()` and `dip.ObjectMinimum()`.

- Added `dip.BlendBandlimitedMask()`.

- Added `dip.Image.IsForged()`.

- Added the class `dip.Graph`, and the functions `dip.RegionAdjacencyGraph()` and `dip.Relabel()` with
  a graph as input. 

### Changed functionality

- The function `dip.Histogram()` is now called `dip.Histogram_old()`, to make space for the new `dip.Histogram`
  class. **NOTE! This breaks backwards compatibility.** To keep old code from working, do
  `dip.Histogram = dip.Histogram_old` at the top of the code, after `import diplib as dip`.

- The function `dip.LookupTable()` is now called `dip.LookupTable_old()`, to make space for the new `dip.LookupTable`
  class. **NOTE! This breaks backwards compatibility.** To keep old code from working, do
  `dip.LookupTable = dip.LookupTable_old` at the top of the code, after `import diplib as dip`.

- `dip.HistogramMatching()` now also accepts a `dip.Histogram` object as the second input argument, matching
  the *DIPlib* function.

- `dip.Show()` has a new parameter `extent`, uses proper warnings rather than just printing messages to
  the console, and no longer uses `dip.ImageDisplay()` for 1D images.

- The `dip::StructuringElement` class was exposed to Python as `dip.SE` (to make typing easier). It is now
  exposed as `dip.StructuringElement`, with `dip.SE` an alias for backwards compatibility (and for less typing).

- Added some constructor overloads and implicit conversions that make it much easier to work with pixel sizes.

(See also changes to *DIPlib*.)

### Bug fixes

- Fixed memory corruption bug when releasing data from a different thread.

- Fixed name of last input argument to `dip.GreyWeightedDistanceTransform`, which is now `mode`.

- A tuple with numbers now implicitly converts to a `dip.StructuringElement` and a `dip.Kernel`.

(See also bugfixes to *DIPlib*.)




## Changes to *DIPviewer*

### New functionality

- Added possibility to build *DIPviewer* without *GLFW* or *FreeGLUT*. This must be explicitly
  enabled by setting `DIP_BUILD_DIPVIEWER`. This is useful when building *DIPviewer* for use
  in *MATLAB*, where Java provides the window functionality.

- Can now step dimensions by scrolling in the status bar when it is showing the operating point.

- Can now set the visualized dimensions programmatically from Python.

- Made window linking easier in Python.

- Added `dip.Image.ShowSlice()` as an alias for `dip.viewer.Show()`.

- Better integration into the Python/IPython REPL (use `%gui dip` in IPython)

### Bug fixes

- Fixed an issue that could cause a segmentation violation after all the *DIPviewer* windows were closed.

- Fixed an issue that prevented Visual Studio from building *PyDIPviewer*.

- Fixed range calculation for images with infinities.




## Changes to *DIPjavaio*

None.
