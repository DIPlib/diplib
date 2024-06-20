---
layout: post
title: "Changes DIPlib 3.5.0"
date: 2024-06-17
---

## Changes to *DIPlib*

### New functionality

- Added `dip::OutputBuffer` as a virtual base class, and `dip::SimpleOutputBuffer` and `dip::FixedOutputBuffer`
  as specific instances. These are used to handle a buffer that a JPEG or PNG file can be written into by
  `dip::ImageWriteJPEG()` and `dip::ImageWritePNG()`.

- Added `dip::Framework::Projection()` as a public function. Previously it was a private library function for internal
  use only. This framework function simplifies the creation of projection operations, and forms the basis of
  functions such as `dip::Mean()` and `dip::Maximum()`.

- Added macros `DIP_PARALLEL_ERROR_DECLARE`, `DIP_PARALLEL_ERROR_START` and `DIP_PARALLEL_ERROR_END`
  to simplify writing parallel code with OpenMP that properly handles exceptions.

- Added an operator to convert `dip::Units` to `bool`. The units object will test false if it's unitless.

- Added `dip::Quartiles()`.

- Added `dip::Measurement::IteratorFeature::AsImage()` and `AsScalarImage()`.

- Added `dip::Histogram::OptimalConfiguration()` and `OptimalConfigurationWithFullRange()`, which make it possible
  to configure the histogram to use the Freedman--Diaconis rule to choose the bin size.

- Added `dip::LabelMap`, a class that holds labels (object IDs) and can map them to new values. The mapping can be
  applied to a labeled image or to a `dip::Measurement` object. You can also index into a `dip::Measurement`
  object with a `dip::LabelMap`.

- Added comparison operators for `dip::Measurement::IteratorFeature`. They return a `dip::LabelMap`.

- Added `dip::UnionFind::Size()`.

- Added `dip::IsOnEdge()`.

- Added an overload to `dip::GaussianMixtureModel()` that takes a 1D histogram as input.

### Changed functionality

- The `compressionLevel` argument to `dip::ImageWritePNG()` changed from `dip::uint` to `dip::sint`, allowing for
  -1 to configure the deflate algorithm to use RLE instead of its default strategy. For some images this option
  can lead to smaller files, and for some images to much faster compression.

- The versions of `dip::ImageWriteJPEG()` and `dip::ImageWritePNG()` that write to a memory buffer now take
  a version of a `dip::OutputBuffer` object to write to. Overloaded versions of these two functions maintain
  the previous syntax where the buffer is created internally and returned to the caller.

- `dip::Framework::Projection()` is now parallelized. All projection functions (for example `dip::Mean()` and
  `dip::Maximum()`) now can use multithreading for faster computation if the output has at least one dimension.
  For example, the mean value of each image row can be computed in parallel, but the mean value over the whole
  image is still not computed in parallel.

- `dip::PairCorrelation()`, `dip::ProbabilisticPairCorrelation()`, `dip::Semivariogram()`, and `dip::ChordLength()`
  Have been parallelized for `"random"` sampling.

- `dip::GrowRegionsWeighted()` with a `dip::Metric` parameter has been deprecated. The new version of the function
  uses the fast marching algorithm in the grey-weighted distance transform. If both `grey` and `mask` are not
  forged, `dip::EuclideanDistanceTransform()` is called instead of `dip::GreyWeightedDistanceTransform()`.
  This makes the function efficient for when isotropic growing is required. And the function now has a new float
  parameter, `distance` that determines how far the regions are grown.

- `dip::ImageRead()` throws a `dip::RunTimeError` instead of a `dip::ParameterError` if the file doesn't exist
  or is of an unrecognized type.

- `dip::GetOptimalDFTSize()` has a new argument `maxFactor` that determines what the largest factor for the output
  integer can be. Valid values are 5, 7 and 11. A new function `dip::MaxFactor()` will give the appropriate value
  depending on whether a real or complex-valued transform is being computed, and what FFT implementation is being
  used. When using *PocketFFT*, this function can now also return much larger values than before.

- `dip::OptimalFourierTransformSize()`, which depends on `dip::GetOptimalDFTSize()` (see the bullet point above
  this one) has a new argument `purpose` that can be either `"real"` or `"complex"`, and uses the new
  `dip::MaxFactor()` to fill in the `maxFactor` argument.

- `dip::GaussFT()` has a new parameter `boundaryCondition`, the default empty value gives the same behavior as
  previously. `dip::Gauss()` now passes the boundary condition through to `dip::GaussFT()`.
  See [issue #159](https://github.com/DIPlib/diplib/issues/159).

- The versions of `dip::IsodataThreshold()`, `dip::OtsuThreshold()`, `dip::MinimumErrorThreshold()`,
  `dip::GaussianMixtureModelThreshold()`, `dip::TriangleThreshold()` and `dip::BackgroundThreshold()` that take an
  image as input, now compute the histogram using the new `dip::Histogram::OptimalConfiguration()` configuration.
  This generally makes them more precise. In some cases, the previous behavior was not correct.
  See [issue #161](https://github.com/DIPlib/diplib/issues/161).
  To reproduce the previous behavior, compute a histogram with default configuration (`dip::Histogram(image)`),
  and pass this histogram to the equivalent threshold estimation function. This function will return a threshold
  value that can be applied to the image using the comparison operator (`image >= threshold`).

- `dip::EstimateNoiseVariance()` uses a potentially more precise threshold internally to exclude edges.

- Made more consistent use of `dip::LabelType`, and we're no longer accepting any label (or object ID) with a value
  that doesn't fit in that type. *DIPlib* has always produced labeled images of this type, but it accepts any
  unsigned integer image as a label image. Both internally and in the API, sometimes we used `dip::LabelType`
  (32-bit unsigned integer), and sometimes we used `dip::uint` (system-dependent size integer, usually 64-bit).
  We now consistently use `dip::LabelType` everywhere:

    - If the input labeled image is of type `uint64`, any pixel value outside the `dip::LabelType` range will cause
      an exception to the thrown. This is true for the measurement functionality and the functions in `regions.h`.

    - Added a version of `dip::Measurement::SetObjectIDs()` that takes a `std::vector< dip::LabelType >` as input.

    - `dip::ListObjectLabels()` is identical to `dip::GetObjectLabels()` (which is now deprecated), but it returns a
      `std::vector< dip::LabelType >` instead of a `dip::UnsignedArray`. `dip::GetObjectLabels()` is deprecated.

    - Added `dip::CastLabelType()`, which can be called with any unsigned integer as input, and the same value
      as a `dip::LabelType`. If the value is too large to fit in that type, an exception is thrown.

    - `dip::GetLabelBoundingBox()` with a `dip::uint` for the `objectID` parameter is deprecated,
      it now needs to be a `dip::LabelType`.

    - `dip::GetImageChainCodes()` with a `dip::UnsignedArray` for the `objectIDs` parameter is deprecated,
      it now needs to be a `std::vector< dip::LabelType >`.

    - `dip::ChainCode::objectID` is now of type `dip::LabelType`, was `dip::uint`.

    - The measurement functions and data structures still use `dip::uint` for object IDs.

- `dip::ListObjectLabels()` adds an option to list only labels touching the image edge.

- `dip::EdgeObjectsRemove()` also works for labeled images.

- `dip::Image::View::Copy()` with an image as input now allows a 1-pixel input to be used. This behaves like
  `dip::Image::View::Fill()` with a `dip::Image::Pixel` as input.

### Bug fixes

- `dip::PairCorrelation`, `dip::ProbabilisticPairCorrelation`, `dip::Semivariogram`, and `dip::ChordLength`
  did not properly compute step sizes for `"grid"` sampling, effectively ignoring the value of `probes`
  and using all pixels (as if `probes` were set to 0).

- Tensor indexing did not remove color space information as expected (bug introduced with the fix to the move
  constructor in 3.4.3).

- `dip::Image::CheckIsMask()` didn't properly check for singleton-expandable masks, accepting masks of any size.

- `dip::FourierTransform()` threw an exception when the real-to-complex dimension had a size of 2.
  See [issue #158](https://github.com/DIPlib/diplib/issues/158).

- A wrong assertion in internal code for `dip::FourierTransform()` made it impossible to inverse transform
  a real-valued image under certain conditions, when *DIPlib* was built with assertions enabled (bug introduced
  with the refactoring of the Fourier code in 3.4.0).

- The 2D parabola fitting function used in `dip::SubpixelLocation()`, `dip::SubpixelMaxima()` and
  `dip::SubpixelMinima()`, with the `"parabolic"` and `"gaussian"` methods, was using wrong weights,
  making the result not precise. The 3D case was not affected. The faster `"parabolic separable"` and
  `"gaussian separable"` methods were also not affected.

### Updated dependencies

- Updated included header-only library *robin-map* to version 1.3.0. Assorted minor improvements.

- Updated included header-only library *pcg-cpp* to the current master branch (last updated 2022-04-08).
  Assorted minor improvements.

- Updated bundled *pybind11* to version 2.12.0.

- Updated bundled library *PocketFFT* to the current master branch (last updated 2024-03-22).
  This fixes a potential compilation problem on macOS.

### Build changes

- Minimum required version of *CMake* is now 3.12.




## Changes to *DIPimage*

### Changed functionality

- Added `'optimal'` to `diphist`, `diphist2` and `mdhistogram` as an option to choose an optimal bin size.

(See also changes to *DIPlib*.)

### Bug fixes

None, but see bugfixes to *DIPlib*.

### Build and installation changes

- Added the *CMake* option `DIP_JAVA_VERSION`, which can be important when building *DIPimage* for older versions
  of MATLAB. The Java version must be equal or older than the version used by MATLAB. Running `version -java`
  in MATLAB will tell you what version of Java it is using.

- Added the function `download_bioformats`, which does the same thing as `python -m diplib download_bioformats` in
  *PyDIP*. This significantly simplifies the installation instructions for *Bio-Formats*.




## Changes to *PyDIP*

### New functionality

- Added bindings to `dip::GaussFIR()`, to complement `GaussIIR()` and `GaussFT()`. Though it is expected users will
  keep using `dip.Gauss()` to access these various algorithms.

- Added bindings for `dip.Units.HasSameDimensions`, `dip.Units.IsDimensionless`, `dip.Units.IsPhysical`,
  `dip.Units.AdjustThousands`, `dip.Units.Thousands`.

- Added `dip.Measurement.ToDataFrame()`, which converts the measurement results into a Pandas DataFrame.
  This function will import NumPy and Pandas when used, these packages need to be installed.

- Added `dip.Measurement.MeasurementFeature.Subset()` (binding for `dip::Measurement::IteratorFeature::Subset()`)
  and `dip.Measurement.FeatureValuesView()`.

- Added bindings for `dip.ColorSpaceManager.SetWhitePoint()`, as well as `dip.ColorSpaceManager.IlluminantA`,
  `IlluminantD50`, `IlluminantD55`, `IlluminantD65` and `IlluminantE`.

- Added bindings for `dip.SeparableConvolution()`, `dip.SeparateFilter()` and the `dip.OneDimensionalFilter`
  struct.

### Changed functionality

- The *DIPlib* exception classes are now properly bound. This changes the type of the exceptions raised by
  the library in Python: `dip.Error` is the new base class, raised exceptions are either `dip.ParameterError`,
  `dip.RunTimeError` or `dip.AssertionError`. `dip.Error` is derived from `Exception`.

- Made `diplib.PyDIPjavaio` more easily available as `diplib.javaio`. The new name matches the C++ library
  namespace, and when imported properly loads the JVM before importing *PyDIPjavaio*. One can now:
    ```python
    import diplib as dip
    import diplib.javaio
    image = dip.javaio.ImageReadJavaIO("foo")
    ```
    `dip.ImageRead()` still automatically loads *PyDIPjavaio* on first use.

- `diplib.viewer` is no longer an alias, but a true submodule, like we did for `diplib.javaio`. This should
  not change anything for the user, other than some IDEs having less trouble identifying the name.

- `dip.ImageRead()` and `dip.javaio.ImageReadJavaIO()` now also have a version that takes the output image
  as a keyword-only argument `out`.

- All the functions named `dip.ImageRead...()` that have a keyword-only argument `out` now return the
  `dip::FileInformation` structure (as a dict, just like `dip.ImageRead...Info()` does).

- Unitless `dip.Units` now test false.

- Moved `dip.MeasurementTool.MeasurementFeature` to `dip.Measurement.MeasurementFeature` and
  `dip.MeasurementTool.MeasurementObject` to `dip.Measurement.MeasurementObject`.
  They are the bindings to `dip::Measurement::IteratorFeature` and `dip::Measurement::IteratorObject` respectively.

(See also changes to *DIPlib*.)

### Bug fixes

- `dip.viewer.Show()` would cause Python to crash if given a raw image.

(See also bugfixes to *DIPlib*.)

### Updated dependencies

- Updated bundled *pybind11* to version 2.12.0.

### Build and installation changes

- The *CMake* option `PYTHON_EXECUTABLE` is no longer used, set `Python_EXECUTABLE` instead. The build script
  will, for the time being, copy the value of the former to the latter if the former is set but the latter isn't.

- The *CMake* option `PYBIND11_PYTHON_VERSION` is no longer used. It was mentioned in the documentation, but
  probably never really worked.

- Significantly improved the command to download *Bio-Formats*: `python -m diplib download_bioformats` now
  has an optional argument `-u` to force an update of the existing library, and another one to specify
  which verison to download. By default, it downloads whatever the latest version is, we no longer hard-code
  a specific version number.




## Changes to *DIPviewer*

### Bug fixes

- Trying to display an image with wrong color space information would throw an exception. The image is now seen
  as a generic tensor image.

- Continuous scrolling devices were way too sensitive when using GLFW, it was really hard to zoom in and out
  of an image. (The fix works at least on macOS, let us know if this produces issues on other platforms.)




## Changes to *DIPjavaio*

### Changed functionality

- `dip::javaio::ImageReadJavaIO()` has an additional parameter `imageNumber` to select which image in a multi-image
  file to read. The `dip::FileInformation` structure returned now correctly indicates how many images are in the file
  in the `numberOfImages` element.

### Build changes

- It is no longer necessary to have Bio-Formats present. The repository now includes a (non-functional) stub
  of Bio-Formats that is used during the compilation process. It is still necessary to separately download the
  Bio-Formats JAR file to use *DIPjavaio*.
