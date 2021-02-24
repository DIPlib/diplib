---
layout: post
title: "Changes DIPlib 3.0.0"
---

*DIPlib 3* is a complete rewrite in C++ of the *DIPlib 2* infrastructure, which was written
in C; only the code that implements actual image processing and analysis algorithms is ported
over.

The list below describes infrastructure changes, function interface changes, and functionality
changes in the *DIPlib* library. It is possible that we missed some changes here, but hopefully
this list will help in porting your old code that used *DIPlib* to the new version.


## Core/infrastructure changes

- Functions and types used to start with `dip_`, now they are in the `dip::` namespace.

- Functions no longer return a `dip_Error`, they throw an exception instead. The `DIPXJ`
  et al. macros are gone. So are the `DIP_FNR_DECLARE` / `DIP_FNR_INITIALISE` /
  `DIP_FNR_EXIT` macros.

- The array types are now based on either `std::vector` or `dip::DimensionArray`, which
  is a short-array optimized version of `std::vector` with a bunch of specialized algorithms
  as member functions. Consequently, all access functions are no longer, to be replaced by
  methods with different names and syntax.

- The core image class, `dip_Image`, now is `dip::Image`, with a much simplified interface.
  As with the array types, access functions are no longer, to be replaced by class methods.
  Many of these class methods have names similar to the original access functions, but not
  always. For example, `dip_ImageSetStride` now is `dip::Image::SetStrides`.

    - `dip_Assimilate` is the `dip::Image::ReForge` method.

    - We used to call the image size its dimension. `dip_Dimensions` is now `dip::Image::Sizes`.
      The reason is that it was too confusing talking about a dimension as an image axis (the
      2<sup>nd</sup> dimension), and the dimension of that dimension (the size of the image along that
      axis).

    - Images now can have a tensor as pixel value, as was possible in *DIPimage*. To port old
      functions, you can test for `dip::Image::IsScalar` and ignore this new feature, or
      modify the old function to work with tensor values.

    - Images now carry the physical dimension in them (referred to as pixel size). When porting
      functions, think about whether this data needs to be maintained, modified, or removed.

    - The same is true for the color space. A function that changes the number of tensor
      elements must also remove the color space information.

    - We have added 64-bit signed and unsigned integers to the set of allowed data types for pixels.

    - Please read the documentation to the `dip::Image` class before doing any work with the
      library!

- The framework functions all have different names:
    - `dip_SeparableFrameWork` &rarr; `dip::Framework::Separable`
    - `dip_MonadicFrameWork` &rarr; `dip::Framework::ScanMonadic` / `dip::Framework::SeparableMonadic`
    - `dip_MonadicPoint` &rarr; `dip::Framework::ScanMonadic`
    - `dip_MonadicPointData` &rarr; `dip::Framework::ScanMonadic`
    - `dip_PixelTableArrayFrameWork` &rarr; doesn't exist any more, was used meaningfully only in `dip_GeneralisedKuwaharaImproved`
    - `dip_PixelTableFrameWork` &rarr; `dip::Framework::Full`
    - `dip_ScanFrameWork` &rarr; `dip::Framework::Scan` / `dip::Framework::ScanDyadic`
    - `dip_SingleOutputFrameWork` &rarr; `dip::Framework::ScanSingleOutput`
    - `dip_SingleOutputPoint` &rarr; `dip::Framework::ScanSingleOutput`

  Their interfaces are not exactly compatible, but it should be relatively straightforward
  to port old line functions to use the new framework, yielding shorter code.
  However, all line functions are now expected to use strides.

- `dip_ovl.h` is now `diplib/overload.h`, and works like a normal header file defining
  macros to use in your code. `dip_tpi.h` is gone, the template strategy to define overloaded
  functions is now based on C++ templates, yielding code that is easier to write and easier
  to read.

- There is no longer a `dip_Initialise` function. There are no registries.
  There is only one global variable: the number of threads to use with OpenMP (see
  `dip::GetNumberOfThreads` and `dip::SetNumberOfThreads`).

- There is no longer a `dip_ImagesSeparate` function. Its functionality can be accomplished
  using a simple image copy (the copy shares the image data with the original image, and
  makes it possible to strip the original image while still keeping the input data available).

- Header files used to have names such as `dip_xxx.h`, they now are `diplib/xxx.h`. The &ldquo;`xxx`&rdquo;
  part has remained the same in most cases, though some function declarations have moved to a
  different header file. The documentation specifies which header file to include for each
  function.

- The *dipIO* library no longer exists. Some `dipio_Xxx` functions are now defined in the
  `diplib/file_io.h` header file (reading and writing ICS, TIFF and JPEG files).
  `dipio_MeasurementWriteCSV` is is `diplib/measurement.h`.
  `dipio_Colour2Gray` functionality is replaced (significantly improved) by `dip::ColorSpaceManager`.
  Other functionality no longer exists.


## Changes in algorithm interface

- Function parameters expressing options are now represented by strings rather than
  `#define` or `enum` constants (except for low-level library functionality).

- The library now makes a distinction between signed and unsigned integers. This affects many
  function parameters.

- Output arguments are now always on the left-hand side, except for output images.

- We try to sort the arguments most commonly left with default values at the end of the
  argument lists. This caused some functions to have a different parameter order.
  For example, the boundary condition is not commonly changed, and so the boundary condition
  array input argument is now typically further to the right in (usually at the end of) the
  argument list.

- Function names are often simplified, since in C++ it's possible to overload functions for
  different types. For example, `dip_BesselJ0` and `dipm_BesselJ0` are now both called
  `dip::BesselJ0`. `dip_BooleanArrayUseParameter`, `dip_IntegerArrayUseParameter`, etc. are
  now all `dip::ArrayUseParameter`.

- Some other function names that included only parts of a name or verb (they were shortened)
  have been renamed to use full names. For example: `dip_Sub` is now `dip::Subtract`;
  `dip_LaplaceMinDgg` is now called `dip::LaplaceMinusDgg`; `dip_RadialMin` and `dip_RadialMax`
  are now called `dip::RadialMinimum` and `dip::RadialMaximum`.

- `dip_Measure` is now `dip::MeasurementTool::Measure`, with `dip::MeasurementTool` an object
  that knows about defined measurement features.

    - Measurement features are registered with a `dip::MeasurementTool` object, not in the
      global registry.

    - Measurement features have a much simpler programmer's interface, it should be easier to
      write new features now.

    - Computed measurements are now in an object of type `dip::Measurement`, whose interface is
      much easier to use than the old `dip_Measurement`.

- `dip_ObjectToMeasurement` now takes a feature iterator rather than computing the feature;
  this might be more flexible.

- `dip_Arith` and `dip_Compare` used to implement all arithmetic and comparison operators,
  (with convenience macros `dip_Add`, `dip_Sub`, etc.). These functions no longer exist,
  each operator is implemented by its own function. The shortened names of these functions
  are no longer shortened, so instead of `dip_Sub` use `dip::Subtract`. `dip_AddInteger`,
  `dip_AddFloat`, etc. now are overloaded versions of `dip::Add` et al.

- `dip_WeightedAdd` is now generalized to `dip::LinearCombination`.
  `dip_WeightedSub` can be emulated by setting a negative weight.
  `dip_WeightedMul` and `dip_WeightedDiv` were not used anywhere, and don't seem very useful.

- Morphological filters now use a `dip::StructuringElement` to collect three parameters
  of *DIPlib 2*. Other filters that had a filter shape argument now use a
  `dip::Kernel` to collect three parameters.

- `dip_Min` and `dip_Max` are now `dip::Infimum` and `dip::Supremum`, and `dip_SignedMinimum`
  is now `dip::SignedInfimum`. There are too many functions with "Maximum" and "Minimum" in
  their name.

- `dip_MultiDimensionalHistogram` and `dip_ImageToHistogram` have been merged into the
  constructor for `dip::Histogram`, which replaces `dip_Histogram`.

- `dip::Distribution` is a new class that replaces `dip_Distribution`. `dip_Distribution` was
  identical to `dip_Histogram`. `dip::Distribution` is very different from `dip::Histogram`.

- The `dip_ImageLookup` function has been merged with the `dip::LookupTable` object, and
  includes the functionality of `dip_ImageLut`.

- `dip_Threshold` is now `dip::FixedThreshold`. `dip::Threshold` is a function that allows
  to select an automated threshold method.

- `dip_RandomXxx` functions are now methods to the `dip::Random` class. `dip_UniformRandomVariable`
  and similar functions are now classes `dip::UniformRandomGenerator` and similar.
  `dip::GaussianRandomGenerator` produces a single output value, the object stores the
  second value for the next call.

- `dip_LabelSetBorder`, `dip_ImageDoEdge` and `dip_PlaneDoEdge` are merged into `dip::SetBorder`,
  which additionally can specify the border size.

- `dip_LocalMinima` is now called `dip::WatershedMinima`. There's also a maxima variant, `dip::WatershedMaxima`.

- `dip_GetMaximumAndMinimum` is now called `dip::MaximumAndMinimum` for consistency.

- `dip_ResamplingFT` is no longer a separate function, call `dip::Resampling` with `"ft"` as
  method. `dip_Skewing` was renamed to `dip::Skew` for consistency. `dip_Rotation` has
  been renamed `dip::Rotation2D`, and a generalized `dip::Rotation` is similar but takes
  two additional integers to specify the rotation plane. `dip_Rotation3d_Axis` and
  `dip_Rotation3d` are now both called `dip::Rotation3D`, their argument types are different.
  `dip::Resampling`, `dip::Skew`, `dip::Rotation` and similar now take a boundary condition
  as optional argument. `dip::Skew` has a new parameter that allows selection of the image
  line that has 0 shift. `dip::Rotation` now rotates around the pixel at the origin (this is
  the pixel to the right of the geometric center for even-sized images), rather than the geometric
  center of the image.

- `dip_Rotation2d90`, `dip_Rotation2d90`, `dip_Map`, `dip_Mirror`, and `dip_Crop` are now all
  member functions of `dip::Image`, and modify the existing image (without copying data).
  `dip_GetSlice`, `dip_PutSlice`, `dip_PutSubSpace`, `dip_GetLine`, `dip_PutLine`, `dip_Set`,
  `dip_SetXxx`, `dip_Get`, `dip_GetXxx` are now all implemented through indexing operations
  on the `dip::Image` object (see the many overloads of `dip::Image::At`) and assignment to
  the output of these operations.

- `dip_KuwaharaImproved` is now called `dip::Kuwahara`, and `dip_Kuwahara` is no longer
  (C++ default values make it redundant).

- `dip_GeneralisedKuwaharaImproved` is now called `dip::SelectionFilter`, and
  `dip_GeneralisedKuwahara` is no longer (C++ default values make it redundant).

- `dip_PaintBox`, `dip_PaintDiamond` and `dip_PaintEllipsoid` are now called `dip::DrawBox`,
  `dip::DrawDiamond` and `dip::DrawEllipsoid`, respectively (for consistency). `dip_DrawLineFloat` and
  `dip_DrawLineComplex` are now a single function `dip::DrawLine`; the same is true for `dip::DrawLines`.

- `dip_RadialDistribution` renamed to `dip::DistanceDistribution`.

- `dip_Bilateral` renamed to `dip::SeparableBilateralFilter`, and `dip_BilateralFilter`
  renamed to `dip::FullBilateralFilter`. Added new function `dip::BilateralFilter` as
  an interface to all bilateral filter variants.

- `dip_TestObjectCreate`, `dip_TestObjectModulate`, `dip_TestObjectBlur` and
  `dip_TestObjectAddNoise` have been merged into a single function: `dip::TestObject`.
  The new function additionally has the option to generate test objects with Gaussian
  edges directly in the spatial domain.

- `dip_GenerateRamp` is now `dip::CreateRamp`, and has convenience functions `dip::FillRamp`,
  `dip::FillXCoordinate`, `dip::CreateXCoordinate`, `dip::FillYCoordinate`, `dip::CreateYCoordinate`,
  `dip::FillZCoordinate`, `dip::CreateZCoordinate`.

- `dip::EuclideanDistanceToPoint` and friends now take the sizes of the output image to create. They
  are generalized in the functions `dip::DistanceToPoint` and `dip::FillDistanceToPoint`.

- `dip::ClipLow` and `dip::ClipHigh` are convenience interfaces to `dip::Clip`.


## Changes in functionality

- Second order extrapolation boundary extension didn't do as advertised in *DIPlib 2*.
  Also the first order extrapolation didn't work correctly with unsigned integers.
  The new implementation fits a 1<sup>st</sup> or 2<sup>nd</sup> order polynomial that reaches
  0 at the end of the extended boundary, with the 2<sup>nd</sup> order polynomial matching
  two samples at the image edge and therefore connects smoothly.
  A 3<sup>rd</sup> order extrapolation has been added, which works similarly to the
  2<sup>nd</sup> order extrapolation but reaches 0 smoothly at the end of the boundary.
  These functions are quite noise sensitive, however, and produce high frequencies along
  the edge.

- The mirror and asymmetric boundary extension methods have been fixed to work correctly
  when the boundary is larger than the image. Mirror no longer repeats the pixel at the
  edge of the image.

- Fixed little bug in the "Perimeter" measurement feature, which didn't check the corner count
  for the transition from the last to the first chain code, yielding a perimeter that was often
  0.0910 pixels too long. The feature "Radius" now computes the center of mass of the object
  correctly.

- The measurement features "Skewness" and "ExcessKurtosis" are no longer separate features, but
  instead put together in the new "Statistics" feature, which also computes the mean and standard
  deviation. The measurement feature "StdDev" is now called "StandardDeviation".
  The algorithms to compute these statistics have changed to be more stable. The measurement
  feature "Sum" was an alias to "Mass", and is not (yet?) available.

- The measurement feature "Convexity" has been renamed to "Solidity", the more common name
  for the ratio of area to convex area.

- New measurement features: "SolidArea", "Circularity", "Convexity", "Eccentricity", "EllipseVariance",
  "Roundness", "DimensionsCube", "DimensionsEllipsoid", "GreyDimensionsCube" and "GreyDimensionsEllipsoid".
  Note that "Convexity" therefore is not the same as what it was in the previous version of the library.

- The measurement features "Mu", "Inertia", "MajorAxes", and their grey-value versions have been generalized
  to arbitrary number of dimensions.

- `dip::SeparableConvolution` treats input filter definitions slightly differently, and there
  no longer are `left` and `right` options.

- `dip::ImageDisplay` no longer does any spatial scaling. Also, it's a class, not a function, and provides
  much more functionality than the function of the same name in the old version of the library.

- `dip::ColorSpaceManager` is functionality ported from *MATLAB*-code in *DIPimage 2*,
  with a few new color spaces added.

- `dip::FourierTransform` now does normalization in the more common way (forward transform not
  normalized, inverse transform normalized by 1/N), but an option (`"symmetric"`) allows to change
  the normalization to be consistent with *DIPlib 2*, which used a symmetric normalization
  scheme (both forward and backward transforms use 1/N<sup>1/2</sup>). It also has options
  to specify where the origin of the transform is (`"corner"`) and for padding to a nice
  size (`"fast"`).

- `dip::Histogram` misses a few of the options that `dip_MultiDimensionalHistogram` had, but I
  don't think they are relevant. They can be added easily if necessary. Histograms now use
  64-bit unsigned integers for the bins. It is possible to add histograms together.

- Many threshold selection strategies have been ported from *MATLAB* code in *DIPimage*, see
  `dip::Threshold`.

- `dip::Random` uses a different pseudo-random number generator. Previous versions used the
  Mersenne Twister. We now use the PCG scheme (permuted linear congruential generator), which
  is much faster than the Mersenne Twister, has a much smaller internal state, and produces
  better quality randomness, though with the given parameters, the period is not as large as
  that of the Mersenne Twister (2<sup>128</sup> vs 2<sup>19937</sup>, but do note that
  2<sup>128</sup> is a very, very long period).

- Morphological filters define line structuring elements differently than before. They used to
  be available only for 2D images, now they are generalized to nD. The filter parameter, instead
  of being a length and an angle, now represents the bounding box, with direction encoded by the
  signs. Besides the discrete line and the interpolated line, we now have several other options,
  including the periodic line, which makes translation-invariant operations with line SEs much
  more efficient. The `"diamond"` SE is now implemented using line SEs, and we have added an
  `"octagonal"` SE that is computed as a combination of a diamond SE and a rectangular SE.

- `dip::Rotation` and dependent functions now rotate around the pixel that is right from center
  (the same pixel that is assumed the origin in `dip::FourierTransform` and other functions),
  instead of the point in between pixels, in case of an even-sized image. The implementation is
  slightly more complex, but this definition is more useful. This function now works for any
  number of dimensions, though it only rotates around one cartesian axis. `dip::Rotation3D` is
  a new function that does a full 3D rotation.

- `dip::Skew` can now skew in multiple dimensions at the same time.

- `dip::Shift` now calls `dip::Resampling`, shifting the image in the spatial domain. `dip::ShiftFT`
  uses the frequency domain, as `dip_Shift` did.

- `dip::Resampling` (and by extension `dip::Shift`) shifts the image in the opposite direction
  from what it did in *DIPlib 2*, where the shift was unintuitive.

- `dip::FindShift` now returns the shift with inverse sign compared to before, to match the reversal
  in `dip::Shift`.

- `dip::Label` now returns an unsigend integer image (previously it was a signed integer image).
  All functions that work on labeled images expect an unsigned integer image as input.

- `dip::ImageWriteICS` now has a `"fast"` option that causes dimensions to be written to file in
  the order in which they are stored in memory, rather than in standard increasing order. This
  makes writing a lot faster if strides are non-standard. `dip::ImageReadICS` has a similar option
  that re-orders strides for the output image to match those of the file, again potentially decreasing
  reading times significantly.

- `dip::ImageReadJPEG` and `dip::ImageReadTIFF` now set the color space of RGB images read to `"sRGB"`,
  rather than (linear) `"RGB"`.

- `dip::Canny` and `dip::MaximumSuppression` have been generalized from 2D only to any
   number of dimensions.

- `dip::GreyWeightedDistanceTransform` now works for images of any dimensionality, and no longer
  excludes the pixels at the edge of the image. It also accepts an optional mask image. The Euclidean
  distance output argument has been merged with the grey-weighted distance output, a flag is used to
  choose which of these two outputs should be given. By default, it now uses the fast marching algorithm,
  which produces more isotropic results than the old algorithm.

- `dip::GrowRegions` no longer takes a grey-value image as input. Use `dip::SeededWatershed` instead.

- `dip::EuclideanDistanceTransform` has a new algorithm which produces exact distances in nD, and
  is parallelized and very fast. This new algorithm is the default.

- `dip::ResampleAt` has various signatures and a lot more flexibility than in the old library. There
  is also a `dip::ResampleAtUnchecked`, to be used together with `dip::PrepareResampleAtUnchecked`,
  for low-level use.

- `dip::OrientationSpace` is a simplified version of the old `dip_OrientationSpace` and `dip_ExtendedOrientationSpace`,
  with only the most useful options.

- Documented functions not (yet) ported: `dip_AdaptivePercentile`, `dip_BiasedSigma`, `dip_GaussianSigma`,
  `dip_HartleyTransform`, `dip_PseudoInverse`, `dip_Sigma`, `dip_TikhonovMiller`,
  `dip_TikhonovRegularizationParameter`, and `dip_UpperEnvelope`. It is unlikely that these will be ported,
  unless someone really needs them.


## New functionality

- New external library *DIPviewer* adds interactive image display, see [its documentation](https://diplib.org/diplib-docs/dipviewer.html).

- New external library *DIPjavaio* adds the option to use *Bio-Formats* to read hundreds of image file formats,
  see [its documentation](https://diplib.org/diplib-docs/dipjavaio.html).

- New analysis functions: `dip::AutoCorrelationFT`, `dip::MeanShift`, `dip::FourierMellinMatch2D`, `dip::MonogenicSignal`,
  `dip::MonogenicSignalAnalysis`, `dip::Semivariogram`, `dip::Granulometry`, `dip::FractalDimension`.

- New detection functions: `dip::HoughTransformCircleCenters`, `dip::FindHoughMaxima`, `dip::PointDistanceDistribution`,
  `dip::FindHoughCircles`, `dip::RadonTransformCircles`, `dip::HarrisCornerDetector`, `dip::ShiTomasiCornerDetector`,
  `dip::NobleCornerDetector`, `dip::WangBradyCornerDetector`, `dip::FrangiVesselness`,
  `dip::MatchedFiltersLineDetector2D`, `dip::DanielssonLineDetector`, `dip::RORPOLineDetector`.

- New display functions: `dip::ApplyColorMap`, `dip::Overlay`, `dip::MarkLabelEdges`.

- New distance function: `dip::GeodesicDistanceTransform`.

- New binary filtering functions: `dip::FillHoles`, `dip::ConditionalThickening2D`, `dip::ConditionalThinning2D`,
  `dip::BinaryAreaOpening`, `dip::BinaryAreaClosing`, `dip::CountNeighbors`, `dip::MajorityVote`, `dip::GetSinglePixels`,
  `dip::GetEndPixels`, `dip::GetLinkPixels`, `dip::GetBranchPixels`, `dip::SupGenerating`, `dip::InfGenerating`,
  `dip::UnionSupGenerating`, `dip::UnionSupGenerating2D`, `dip::IntersectionInfGenerating`,
  `dip::IntersectionInfGenerating2D`, `dip::Thickening`, `dip::Thickening2D`, `dip::Thinning`, `dip::Thinning2D`,
  `dip::HomotopicThinningInterval2D`, `dip::HomotopicThickeningInterval2D`, `dip::EndPixelInterval2D`,
  `dip::HomotopicEndPixelInterval2D`, `dip::HomotopicInverseEndPixelInterval2D`, `dip::SinglePixelInterval`,
  `dip::BranchPixelInterval2D`, `dip::BoundaryPixelInterval2D`, `dip::ConvexHullInterval2D`.

- New linear filtering functions: `dip::SeparateFilter`, `dip::Dx`, `dip::Dy`, `dip::Dz`, `dip::Dxx`, `dip::Dyy`,
  `dip::Dzz`, `dip::Dxy`, `dip::Dxz`, `dip::Dyz`, `dip::Gradient`, `dip::GradientDirection`, `dip::Curl`,
  `dip::Divergence`, `dip::Hessian`, `dip::UnsharpMask`, `dip::GaborFIR`, `dip::Gabor2D`, `dip::LogGaborFilterBank`,
  `dip::NormalizedConvolution`, `dip::NormalizedDifferentialConvolution`, `dip::MeanShiftVector`.

- New morphological functions: `dip::RankFilter`, `dip::RankMinClosing`, `dip::RankMaxOpening`,
  `dip::LimitedMorphologicalReconstruction`, `dip::HMaxima`, `dip::HMinima`, `dip::Leveling`, `dip::AreaClosing`,
  `dip::OpeningByReconstruction`, `dip::ClosingByReconstruction`, `dip::AlternatingSequentialFilter`,
  `dip::HitAndMiss`, `dip::VolumeOpening`, `dip::VolumeClosing`.

- New nonlinear filtering functions: `dip::MoveToLocalMinimum`,
  `dip::PeronaMalikDiffusion`, `dip::GaussianAnisotropicDiffusion`,
  `dip::RobustAnisotropicDiffusion`, `dip::CoherenceEnhancingDiffusion`.

- New generation functions: `dip::FillRadiusCoordinate`, `dip::CreateRadiusCoordinate`,
  `dip::FillRadiusSquareCoordinate`, `dip::CreateRadiusSquareCoordinate`, `dip::FillPhiCoordinate`,
  `dip::CreatePhiCoordinate`, `dip::FillThetaCoordinate`, `dip::CreateThetaCoordinate`, `dip::FillCoordinates`,
  `dip::CreateCoordinates`,
  `dip::ApplyWindow`,
  `dip::DrawPolygon2D`, `dip::DrawBandlimitedPoint`, `dip::DrawBandlimitedLine`, `dip::DrawBandlimitedBall`,
  `dip::DrawBandlimitedBox`,
  `dip::SaltPepperNoise`, `dip::FillColoredNoise`, `dip::ColoredNoise`,
  `dip::GaussianEdgeClip`, `dip::GaussianLineClip`,
  `dip::FillDelta`, `dip::CreateDelta`, `dip::CreateGauss`, `dip::CreateGabor`,
  `dip::FillPoissonPointProcess`, `dip::CreatePoissonPointProcess`, `dip::FillRandomGrid`, `dip::CreateRandomGrid`.

- New geometric transformation functions: `dip::RotationMatrix2D`, `dip::RotationMatrix3D`,
  `dip::AffineTransform`, `dip::WarpControlPoints`, `dip::LogPolarTransform2D`,
  `dip::Tile`, `dip::TileTensorElements`, `dip::Concatenate`, `dip::JoinChannels`.

- New grey-value mapping functions: `dip::Zero`, `dip::HistogramEqualization`, `dip::HistogramMatching`.

- New histogram functions: `dip::CumulativeHistogram`, `dip::Smooth`,
  `dip::Mean`, `dip::Covariance`, `dip::MarginalMedian`, `dip::Mode`, `dip::PearsonCorrelation`, `dip::Regression`,
  `dip::MutualInformation`, `dip::Entropy`,
  `dip::IsodataThreshold`, `dip::OtsuThreshold`, `dip::MinimumErrorThreshold`, `dip::GaussianMixtureModelThreshold`,
  `dip::TriangleThreshold`, `dip::BackgroundThreshold`,
  `dip::KMeansClustering`, `dip::MinimumVariancePartitioning`,
  `dip::EqualizationLookupTable`, `dip::MatchingLookupTable`, `dip::PerObjectHistogram`.

- New labeled regions function: `dip::Relabel`, `dip::RegionAdjacencyGraph`.

- New math and statistics functions: `dip::IsNotANumber`, `dip::IsInfinite`, `dip::IsFinite`, `dip::InRange`,
  `dip::OutOfRange`,
  `dip::MaximumAbsoluteError`, `dip::PSNR`, `dip::SSIM`, `dip::MutualInformation`,
  `dip::SpatialOverlap`, `dip::DiceCoefficient`, `dip::JaccardIndex`, `dip::Specificity`, `dip::Sensitivity`,
  `dip::Accuracy`, `dip::Precision`, `dip::HausdorffDistance`, `dip::ModifiedHausdorffDistance`,
  `dip::SumOfMinimalDistances`, `dip::ComplementWeightedSumOfMinimalDistances`,
  `dip::Entropy`, `dip::EstimateNoiseVariance`,
  `dip::GeometricMean`, `dip::MeanSquare`, `dip::SumSquare`, `dip::MaximumAbs`, `dip::MinimumAbs`,
  `dip::MedianAbsoluteDeviation`, `dip::All`, `dip::Any`,
  `dip::Count`, `dip::CumulativeSum`, `dip::SampleStatistics`, `dip::Covariance`,  `dip::PearsonCorrelation`,
  `dip::SpearmanRankCorrelation`, `dip::CenterOfMass`, `dip::Moments`,
  `dip::Transpose`, `dip::ConjugateTranspose`, `dip::DotProduct`, `dip::CrossProduct`,
  `dip::Norm`, `dip::SquareNorm`, `dip::Angle`, `dip::Orientation`, `dip::CartesianToPolar`, `dip::PolarToCartesian`,
  `dip::Determinant`, `dip::Trace`, `dip::Rank`, `dip::Eigenvalues`, `dip::LargestEigenvalue`, `dip::SmallestEigenvalue`,
  `dip::EigenDecomposition`, `dip::LargestEigenvector`, `dip::SmallestEigenvector`, `dip::Inverse`, `dip::PseudoInverse`,
  `dip::SingularValues`, `dip::SingularValueDecomposition`, `dip::Identity`,
  `dip::SumTensorElements`, `dip::ProductTensorElements`, `dip::AllTensorElements`, `dip::AnyTensorElement`,
  `dip::MaximumTensorElement`, `dip::MaximumAbsTensorElement`, `dip::MinimumTensorElement`,
  `dip::MinimumAbsTensorElement`, `dip::MeanTensorElement`, `dip::GeometricMeanTensorElement`,
  `dip::SortTensorElements`, `dip::SortTensorElementsByMagnitude`,
  `dip::Hypot`.

- New measurement functions: see the class `dip::Measurement`, and functions that apply to it: `dip::Minimum`, `dip::Maximum`,
  `dip::Percentile`, `dip::Median`, `dip::Mean`, `dip::MaximumAndMinimum`, `dip::SampleStatistics`. See the classes
  `dip::ChainCode`, `dip::Polygon`, `dip::ConvexHull`, `dip::CovarianceMatrix`.

- New microscopy functions: `dip::BeerLambertMapping`, `dip::InverseBeerLambertMapping`, `dip::UnmixStains`, `dip::MixStains`,
  `dip::MandersOverlapCoefficient`, `dip::IntensityCorrelationQuotient`, `dip::MandersColocalizationCoefficients`,
  `dip::CostesColocalizationCoefficients`, `dip::CostesSignificanceTest`.

- New segmentation functions: `dip::CompactWatershed`, `dip::StochasticWatershed`,
  `dip::MinimumVariancePartitioning`, `dip::OtsuThreshold`, `dip::MinimumErrorThreshold`, `dip::GaussianMixtureModelThreshold`,
  `dip::TriangleThreshold`, `dip::BackgroundThreshold`, `dip::VolumeThreshold`, `dip::MultipleThresholds`, `dip::Superpixels`.

- New testing functions: `dip::testing::PrintPixelValues`, `dip::testing::CompareImages`. New class: `dip::testing::Timer`.

- New transform functions: `dip::OptimalFourierTransformSize`, `dip::RieszTransform`, `dip::StationaryWaveletTransform`.

- There is also a lot of new functionality in the library infrastructure, which we cannot all list here.
  See [the library infrastructure documentation](https://diplib.org/diplib-docs/infrastructure.html).
