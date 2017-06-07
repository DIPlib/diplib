# Work plan for DIPlib 3.0 {#workplan}

[//]: # (DIPlib 3.0)

[//]: # ([c]2016-2017, Cris Luengo.)
[//]: # (Based on original DIPlib code: [c]1995-2014, Delft University of Technology.)

[//]: # (Licensed under the Apache License, Version 2.0 [the "License"];)
[//]: # (you may not use this file except in compliance with the License.)
[//]: # (You may obtain a copy of the License at)
[//]: # ()
[//]: # (   http://www.apache.org/licenses/LICENSE-2.0)
[//]: # ()
[//]: # (Unless required by applicable law or agreed to in writing, software)
[//]: # (distributed under the License is distributed on an "AS IS" BASIS,)
[//]: # (WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.)
[//]: # (See the License for the specific language governing permissions and)
[//]: # (limitations under the License.)

This is a list of tasks that need to be done, in order of dependencies.
This list includes dependencies and an estimate of how much design work needs
to be done before that component can be implemented.

Header files mentioned are from *DIPlib* 2.x ("the old *DIPlib*").

Priority for these items to be discussed. Framework and other infrastructure
elements have highest priority, because other things depend on them. Some filters
and algorithms are much more important than others.

The **DEP** markers indicate points that depend on functionality not yet implemented.

**NOTE:** The online documentation is not updated continuously, please see
<a href="https://github.com/DIPlib/diplib/blob/master/src/documentation/workplan.md">this
document's source</a> for the most up-to-date version.

## What is already done:

(2017/06/05) We just passed the 2000 documented entities (functions, classes, constants) mark!

-   CMake compilation environment.

-   Test framework.

-   Class `dip::Image`.

-   `dip::Framework::Scan`, `dip::Framework::Separable` and `dip::Framework::Full`.
    These frameworks are the core of most algorithms. Not yet parallelized.

-   Arithmetic, bitwise and comparison operators.

-   Image iterators.

-   Measurement framework, a class to hold measurement data, and a class that knows all
    measurement features and can apply them to images.
    All measurement features have been ported, including the previously only defined in
    *DIPimage*.

-   The Fourier transform (based on the code out of OpenCV, much faster than the code in
    the old *DIPlib*).

-   Histograms, including multi-dimensional histograms constructed from tensor images.

-   Look-up table, a single object that includes functionality of various related look-up
    functions in the old *DIPlib*.

-   Global threshold algorithms, ported from a *DIPimage* M-file.

-   Random number generation.

-   An increasing number of filters and operators based on the various frameworks.

-   Color support. More color spaces could be added in time.

-   Interactive image display, as a separate module DIPviewer.

-   *MATLAB* interface.

-   *DIPimage* toolbox: The `dip_image` object is defined and has most methods defined.
    The `dipshow` function works. Functions are being added as functionality is ported
    to the new library.


## What still needs to be done:

-   Test framework: We need to add more tests for some stuff that was implemented before
    the test framework was integrated.

-   Image I/O. Has high priority because it will make testing other functions easier.
    Porting current code in dipIO to read TIFF and ICS files. Interfacing to
    BioFormats.

-   Measurement I/O. Write as CSV is the most important feature here.

-   Parallelization of frameworks. Decision: OpenMP or Intel TBB?

-   Porting filters, analysis routines, etc. See the list at the bottom of this page.

-   The Fourier transform: Use *FFTW* when a compile switch is set, we must be able to
    disable that so *DIPlib* can be used in non-open-source projects. Further improvements
    could be making specific paths for real input or output (could mean a small increase in
    performance).

-   Stuff that is in *DIPimage*:
    - 2D snakes
    - general 2D affine transformation, 3D rotation (is already C code)

-   Other stuff that's not in the old *DIPlib* (see below).

-   *DIPimage* toolbox: MEX-files for *DIPlib* functions to be added as these functions
    are written.
    The `dip_measurement` class needs to be rewritten. The `dipimage` GUI needs to
    be ported, with function input parameter definitions to be provided by `dipmenus`.

-   Python interface. Using Pybind11. We already have a start for this. Write interactive
    image display and GUI as exists in *MATLAB*.

-   Other interfaces. Header files that define
    functions to create a `dip::Image` object around image data from other libraries,
    as well as functions to convert a `dip::Image` to an image of that library
    (either as a view over the same data segment, or by copying the data). We'll
    do this for: OpenCV, ITK, SimpleITK. Any other libraries of interest?


## Functionality currently not in *DIPlib* that would be important to include

- An overlay function that adds a binary or labelled image on top of a grey-value or
  color image.

- Stain unmixing for bright-field microscopy.

- Some filters that are trivial to add:
    - Scharr (slightly better than Sobel)
    - h-minima & h-maxima
    - opening by reconstruction
    - alternating sequential open-close filter (3 versions: with structural opening,
      opening by reconstruction, and area opening)

- Dilation/erosion by a rotated line is currently implemented by first skewing the image,
  applying filter along rows or columns, then skewing back. We can add a 2D-specific
  version that operates directly over rotated lines. The diamond structuring element can
  then be decomposed into two of these operations. We can also add approximations of the
  circle with such lines.

- We're also lacking some other morphological filters:
    - hit'n'miss, where the interval is rotated over 180, 90 or 45 degrees.
    - thinning & thickening, to be implemented as iterated hit'n'miss.
    - levelling

- Radon transform for lines and circles, Hough transform for lines.

- Level-set segmentation, graph-cut segmentation.

- Wavelet transforms.

- The `dip::Label` function should return the number of labels. It could optionally also
  return the sizes of the objects, since these are counted anyway. The labelling algorithm
  by Mike is quite efficient, but we should compare with the more common union-find
  algorithm, which is likely to be optimal for this application (Mike's code uses a
  priority queue, union-find doesn't need it).

- The FreeType 2 library is really cool. I'd like to write a *DIPlib* function
  that can write text into an image.


## List of old *DIPlib* functions that still need to be ported

Between brackets is the name of the old header file it's declared in. They are grouped by
header file they should probably be declared in in the new library.

Many of the following functions are not documented. Undocumented functions have a very
low priority in the porting process, and some might not be ported at all. It being listed
here is not an indication that the function needs to be ported, but if it's not listed here,
it should not be ported (or already is ported).

Some of the following functions already have their prototype written in the new library.

- diplib.h
    - dip_GlobalNumberOfThreadsGet (dip_globals.h) (we don't do globals in DIPlib 3.0, but this could be an exception)
    - dip_GlobalNumberOfThreadsSet (dip_globals.h)

- diplib/analysis.h
    - dip_PairCorrelation (dip_analysis.h)
    - dip_ProbabilisticPairCorrelation (dip_analysis.h)
    - dip_ChordLength (dip_analysis.h)
    - dip_RadialDistribution (dip_analysis.h)
    - dip_CorrelationWrite (dip_analysis.h)
    - dip_StructureAnalysis (dip_analysis.h)
    - dip_SubpixelMaxima (dip_analysis.h)
    - dip_SubpixelMinima (dip_analysis.h)
    - dip_SubpixelLocation (dip_analysis.h)
    - dip_Canny (dip_detection.h) (or in diplib/segmentation.h?)
    - dip_FindShift (dip_findshift.h)
    - dip_CrossCorrelationFT (dip_findshift.h)
    - dip_FtRadialAngularSeparableFilter (dip_structure.h)
    - dip_RARadialPoweredGaussian (dip_structure.h)
    - dip_RARadialGaussian (dip_structure.h)
    - dip_RAAngularPsinc (dip_structure.h)
    - dip_RAAngularGaussian (dip_structure.h)
    - dip_RAAngularDerivative (dip_structure.h)
    - dip_RAAngularTrueDerivative (dip_structure.h)
    - dip_RAAngularCosine (dip_structure.h)
    - dip_RALineEndings (dip_structure.h)
    - dip_OrientationSpace (dip_structure.h)
    - dip_ExtendedOrientationSpace (dip_structure.h)
    - dip_StructureTensor2D (dip_structure.h)
    - dip_StructureDerivatives2D (dip_structure.h)
    - dip_StructureTensor3D (dip_structure.h)
    - dip_StructureDerivatives3D (dip_structure.h)
    - dip_CurvatureFromTilt (dip_structure.h)
    - dip_OSEmphasizeLinearStructures (dip_structure.h)
    - dip_DanielsonLineDetector (dip_structure.h)

- diplib/binary.h
    - dip_BinaryDilation (dip_binary.h)
    - dip_BinaryErosion (dip_binary.h)
    - dip_BinaryClosing (dip_binary.h)
    - dip_BinaryOpening (dip_binary.h)
    - dip_BinaryPropagation (dip_binary.h)
    - dip_EdgeObjectsRemove (dip_binary.h)
    - dip_EuclideanSkeleton (dip_binary.h)
    - dip_BinarySkeleton3D (dip_binary.h) (don't port this one, it's crap; instead fix the bug in EuclideanSkeleton for 3D)

- diplib/distance.h
    - dip_EuclideanDistanceTransform (dip_distance.h)
    - dip_VectorDistanceTransform (dip_distance.h)
    - dip_GreyWeightedDistanceTransform (dip_distance.h)
    - dip_FastMarching_PlaneWave (dip_distance.h)
    - dip_FastMarching_SphericalWave (dip_distance.h)

- diplib/filtering.h
    - dip_PercentileFilter (dip_rankfilter.h)
    - dip_MedianFilter (dip_rankfilter.h)
    - dip_RankContrastFilter (dip_rankfilters.h)
    - dip_VarianceFilter (dip_filtering.h)
    - dip_Kuwahara (dip_filtering.h)
    - dip_GeneralisedKuwahara (dip_filtering.h)
    - dip_KuwaharaImproved (dip_filtering.h) (merge into dip_Kuwahara)
    - dip_GeneralisedKuwaharaImproved (dip_filtering.h) (merge into dip_GeneralisedKuwahara)
    - dip_Sigma (dip_filtering.h)
    - dip_BiasedSigma (dip_filtering.h)
    - dip_GaussianSigma (dip_filtering.h)
    - dip_NonMaximumSuppression (dip_filtering.h)
    - dip_ArcFilter (dip_bilateral.h)
    - dip_Bilateral (dip_bilateral.h) (all three flavours into one function!)
    - dip_BilateralFilter (dip_bilateral.h)
    - dip_QuantizedBilateralFilter (dip_bilateral.h)
    - dip_AdaptiveGauss (dip_adaptive.h)
    - dip_AdaptiveBanana (dip_adaptive.h)
    - dip_StructureAdaptiveGauss (dip_adaptive.h)
    - dip_AdaptivePercentile (dip_adaptive.h)
    - dip_AdaptivePercentileBanana (dip_adaptive.h)
    - dip_PGST3DLine (dip_pgst.h) (this could have a better name!)
    - dip_PGST3DSurface (dip_pgst.h) (this could have a better name!)

- diplib/generation.h
    - dip_FTSphere (dip_generation.h)
    - dip_FTBox (dip_generation.h)
    - dip_FTCube (dip_generation.h)
    - dip_FTGaussian (dip_generation.h)
    - dip_FTEllipsoid (dip_generation.h)
    - dip_FTCross (dip_generation.h)
    - dip_EuclideanDistanceToPoint (dip_generation.h)
    - dip_EllipticDistanceToPoint (dip_generation.h)
    - dip_CityBlockDistanceToPoint (dip_generation.h)
    - dip_TestObjectCreate (dip_generation.h)
    - dip_TestObjectModulate (dip_generation.h)
    - dip_TestObjectBlur (dip_generation.h)
    - dip_TestObjectAddNoise (dip_generation.h)
    - dip_ObjectCylinder (dip_generation.h)
    - dip_ObjectEdge (dip_generation.h)
    - dip_ObjectPlane (dip_generation.h)
    - dip_ObjectEllipsoid (dip_generation.h)
    - dip_DrawLineFloat (dip_paint.h)
    - dip_DrawLineComplex (dip_paint.h) (merge with dip_DrawLineFloat)
    - dip_DrawLinesFloat (dip_paint.h)
    - dip_DrawLinesComplex (dip_paint.h) (merge with dip_DrawLinesFloat)
    - dip_PaintEllipsoid (dip_paint.h)
    - dip_PaintDiamond (dip_paint.h)
    - dip_PaintBox (dip_paint.h)

- diplib/geometry.h
    - dip_Resampling (dip_interpolation.h)
    - dip_ResampleAt (dip_interpolation.h)
    - dip_Subsampling (dip_interpolation.h)
    - dip_Skewing (dip_interpolation.h)
    - dip_SkewingWithBgval (dip_interpolation.h) (merge into dip_Skewing)
    - dip_Rotation (dip_interpolation.h)
    - dip_RotationWithBgval (dip_interpolation.h) (merge into dip_Rotation)
    - dip_Rotation3d_Axis (dip_interpolation.h)
    - dip_Rotation3d (dip_interpolation.h)
    - dip_Rotation2d90 (dip_interpolation.h) (as method to dip::Image)
    - dip_Rotation3d90 (dip_interpolation.h) (as method to dip::Image, generalize to nD)
    - dip_AffineTransform (dip_interpolation.h)
    - dip_Shift (dip_manipulation.h)
    - dip_Wrap (dip_manipulation.h)
    - dip_ResamplingFT (dip_manipulation.h)

- diplib/linear.h
    - dip_OrientedGauss (dip_linear.h)
    - dip_GaborIIR (dip_iir.h)
    - dip_Dgg (dip_derivatives.h)
    - dip_LaplacePlusDgg (dip_derivatives.h)
    - dip_LaplaceMinDgg (dip_derivatives.h)
    - dip_Sharpen (dip_derivatives.h)

- diplib/math.h (should we split this one into two files?)
    - dip_Clip (dip_point.h)
    - dip_ErfClip (dip_point.h)
    - dip_ContrastStretch (dip_point.h)
    - dip_RemapOrientation (dip_point.h)
    - dip_MulConjugate (dip_math.h)
    - dip_NormaliseSum (dip_math.h) (it's rather trivial, worth it?)
    - dip_WeightedAdd (dip_math.h)
    - dip_WeightedSub (dip_math.h)
    - dip_WeightedMul (dip_math.h)
    - dip_WeightedDiv (dip_math.h)
    - dip_AmplitudeModulation (dip_math.h)
    - dip_CosinAmplitudeModulation (dip_math.h)
    - dip_CosinAmplitudeDemodulation (dip_math.h)
    - dip_GeneratePhase (dip_math.h)
    - dip_PositionMaximum (dip_math.h)
    - dip_PositionMinimum (dip_math.h)
    - dip_PositionMedian (dip_math.h)
    - dip_PositionPercentile (dip_math.h)
    - dip_MeanError (dip_math.h)
    - dip_MeanSquareError (dip_math.h)
    - dip_RootMeanSquareError (dip_math.h)
    - dip_MeanAbsoluteError (dip_math.h)
    - dip_IDivergence (dip_math.h)
    - dip_ULnV (dip_math.h)
    - dip_InProduct (dip_math.h)
    - dip_LnNormError (dip_math.h)
    - dip_RadialMean (dip_math.h)
    - dip_RadialSum (dip_math.h)
    - dip_RadialMaximum (dip_math.h)
    - dip_RadialMinimum (dip_math.h)
    - dip_Moments (dip_math.h)
    - dip_CenterOfMass (dip_math.h)
    - dip_ChangeByteOrder (dip_manipulation.h)
    - dip_SimpleGaussFitImage (dip_numerical.h)
    - dip_EmFitGaussians (dip_numerical.h)
    - dip_EmGaussTest (dip_numerical.h)

- diplib/microscopy.h
    - dip_IncoherentPSF (dip_microscopy.h)
    - dip_IncoherentOTF (dip_microscopy.h)
    - dip_ExponentialFitCorrection (dip_microscopy.h)
    - dip_AttenuationCorrection (dip_microscopy.h)
    - dip_SimulatedAttenuation (dip_microscopy.h)
    - dip_RestorationTransform (dip_restoration.h)
    - dip_TikhonovRegularizationParameter (dip_restoration.h)
    - dip_TikhonovMiller (dip_restoration.h)
    - dip_Wiener (dip_restoration.h)
    - dip_PseudoInverse (dip_restoration.h)

- diplib/morphology.h
    - dip_UpperEnvelope (dip_morphology.h)
    - dip_AreaOpening (dip_morphology.h)
    - dip_PathOpening (dip_morphology.h)
    - dip_DirectedPathOpening (dip_morphology.h)
    - dip_UpperSkeleton2D (dip_binary.h)

- diplib/regions.h
    - dip_Label (dip_regions.h)
    - dip_LabelSetBorder (dip_regions.h)
    - dip_ImageDoEdge (dip_regions.h)
    - dip_RegionConnectivit (dip_regions.h)
    - dip_GrowRegions (dip_regions.h)
    - dip_GrowRegionsWeighte (dip_regions.h)
    - dip_SmallObjectsRemove (dip_measurement.h)

- diplib/segmentation.h
    - dip_KMeansClustering (dip_analysis.h)

- diplib/transform.h
    - dip_HartleyTransform (dip_transform.h)

- diplib/io.h (file.h?)
    - dipio_ImageRead (dipio_image.h) (should always return color image)
    - dipio_ImageReadColourSeries (dipio_image.h) (should be named ImageReadSeries)
    - dipio_ImageReadROI (dipio_image.h)
    - dipio_ImageFileGetInfo (dipio_image.h)
    - dipio_ImageWrite (dipio_image.h)
    - dipio_FileGetExtension (dipio_image.h)
    - dipio_FileAddExtension (dipio_image.h)
    - dipio_FileCompareExtension (dipio_image.h)
    - dipio_ImageFindForReading (dipio_image.h)
    - dipio_ImageReadICS (dipio_ics.h)
    - dipio_ImageReadICSInfo (dipio_ics.h)
    - dipio_ImageIsICS (dipio_ics.h)
    - dipio_AppendRawData (dipio_ics.h)
    - dipio_ImageWriteICS (dipio_ics.h)
    - dipio_ImageReadTIFF (dipio_tiff.h) (needs to support tiled images at some point!)
    - dipio_ImageReadTIFFInfo (dipio_tiff.h)
    - dipio_ImageIsTIFF (dipio_tiff.h)
    - dipio_ImageWriteTIFF (dipio_tiff.h)
    - dipio_MeasurementWriteCSV (dipio_msrcsv.h)
