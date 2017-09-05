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

This is a list of tasks that need to be done, no longer in any particular order.

Header files mentioned are from *DIPlib* 2.x ("the old *DIPlib*").

**NOTE:** The online documentation is not updated continuously, please see
<a href="https://github.com/DIPlib/diplib/blob/master/src/documentation/workplan.md">this
document's source</a> for the most up-to-date version.

## What is already done:

(2017/06/05) We just passed the 2000 documented entities (functions, classes, constants) mark!

-   CMake compilation environment.

-   Test framework.

-   Class `dip::Image`, including tensor representations taken from *DIPimage* and improved.

-   `dip::Framework::Scan`, `dip::Framework::Separable` and `dip::Framework::Full`.
    These frameworks are the core of most algorithms. Parallelized using *OpenMP*.

-   Arithmetic, bitwise and comparison operators.

-   Image iterators.

-   Measurement framework, a class to hold measurement data, and a class that knows all
    measurement features and can apply them to images.
    All measurement features have been ported, including the previously only defined in
    *DIPimage*.

-   Color support. More color spaces could be added in time.

-   The Fourier transform (based on the code out of OpenCV, much faster than the code in
    the old *DIPlib*).

-   Histograms, including multi-dimensional histograms constructed from tensor images.

-   Look-up table, a single object that includes functionality of various related look-up
    functions in the old *DIPlib*.

-   Global threshold algorithms, ported from a *DIPimage* M-file.

-   Random number generation, using the new PCG scheme.

-   Image I/O (ICS and TIFF formats, the plan is to not support other formats going forward).

-   An increasing number of filters and operators.

-   Interactive image display, as a separate module *DIPviewer*.

-   *MATLAB* interface.

-   *DIPimage* toolbox: The `dip_image` object is defined and has all methods defined.
    The `dipshow` function works. Functions are being added as functionality is ported
    to the new library.

-   *PyDIP* Python module: Currently just a thin wrapper around the *DIPlib*, with classes
    and functions behaving in the same way they do in C++. Written using *Pybind11*.


## What still needs to be done:

-   Test framework: We need to add more tests for some stuff that was implemented before
    the test framework was integrated.

-   Image I/O: Interfacing to [*Bio-Formats*](http://www.openmicroscopy.org/site/products/bio-formats),
    using [JNI](http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/jniTOC.html)
    to interface between C++ and Java, see for example
    [this tutorial](https://www.codeproject.com/Articles/993067/Calling-Java-from-Cplusplus-with-JNI).
    See also [this ITK module](https://github.com/scifio/scifio-imageio) for
    interfacing to *Bio-Formats*, which uses [SCIFIO](https://github.com/scifio/scifio).
    This should be an optional module, as *Bio-Formats* is GPL.

-   Measurement I/O. Write as CSV is the most important feature here.

-   Parallelization of some non-framework functions using *OpenMP*. For example, the core of the
    projection functions, and `dip::SelectionFilter` can be easily parallelized.

-   Porting filters, analysis routines, etc. See the list at the bottom of this page.

-   The Fourier transform: Use [*FFTW*](http://www.fftw.org) when a compile switch is set.
    We must be able to disable *FFTW* because it's GPL, and we want to allow people to use
    *DIPlib* in non-GPL environments.
    Further improvements could be making specific paths for real input or output (could mean
    a small increase in performance).

-   Stuff that is in *DIPimage*:
    - 2D snakes
    - general 2D affine transformation, 3D rotation (is already C code)

-   Other stuff that's not in the old *DIPlib* (see below).

-   *DIPimage* toolbox: MEX-files for *DIPlib* functions to be added as these functions
    are written.
    The `dip_measurement` class needs to be rewritten. The `dipimage` GUI needs to
    be ported, with function input parameter definitions to be provided by `dipmenus`.

-   *PyDIP* Python module: Write interactive image display and GUI as exists in *MATLAB*.
    Make the module more "Pythonic"?

-   Other interfaces. Header files that define
    functions to create a `dip::Image` object around image data from other libraries,
    as well as functions to convert a `dip::Image` to an image of that library
    (either as a view over the same data segment, or by copying the data). We'll
    do this for: *OpenCV*, *ITK*, *SimpleITK*. Any other libraries of interest?

-   Style sheets for the documentation.

## Functionality currently not in *DIPlib* that would be important to include

- An overlay function that adds a binary or labelled image on top of a grey-value or
  color image.

- Stain unmixing for bright-field microscopy.

- Some filters that are trivial to add:
    - Scharr (slightly better than Sobel)
    - alternating sequential open-close filter (3 versions: with structural opening,
      opening by reconstruction, and area opening)

- We're also lacking some other morphological filters:
    - hit'n'miss, where the interval is rotated over 180, 90 or 45 degrees.
    - thinning & thickening, to be implemented as iterated hit'n'miss.
    - levelling

- Radon transform for lines and circles, Hough transform for lines.

- Level-set segmentation, graph-cut segmentation.

- Super pixels.

- Building a graph out of a labeled image (e.g. from watershed). Graph format?
  Graph manipulation functions, e.g. MST (external lib?), region merging, etc.

- Wavelet transforms.

- A function to write text into an image, using the [*FreeType*](https://www.freetype.org) library.


## List of old *DIPlib* functions that still need to be ported

Between brackets is the name of the old header file it's declared in. They are grouped by
the new header file they should be declared in.

Many of the following functions are not documented. Undocumented functions have a very
low priority in the porting process, and some might not be ported at all. It being listed
here is not an indication that the function needs to be ported, but if it's not listed here,
it should not be ported (or already is ported).

Some of the following functions already have their prototype written in the new library.

- diplib/analysis.h
    - dip_PairCorrelation (dip_analysis.h)
    - dip_ProbabilisticPairCorrelation (dip_analysis.h)
    - dip_ChordLength (dip_analysis.h)
    - dip_RadialDistribution (dip_analysis.h)
    - dip_StructureAnalysis (dip_analysis.h)
    - dip_Canny (dip_detection.h) (or in diplib/segmentation.h?)
    - dip_OrientationSpace (dip_structure.h)
    - dip_ExtendedOrientationSpace (dip_structure.h)
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

- diplib/distance.h
    - dip_FastMarching_PlaneWave (dip_distance.h) (this function needs some input image checking!)
    - dip_FastMarching_SphericalWave (dip_distance.h) (this function needs some input image checking!)

- diplib/generation.h
    - dip_FTSphere (dip_generation.h)
    - dip_FTBox (dip_generation.h)
    - dip_FTCube (dip_generation.h)
    - dip_FTGaussian (dip_generation.h)
    - dip_FTEllipsoid (dip_generation.h)
    - dip_FTCross (dip_generation.h)
    - dip_EuclideanDistanceToPoint (dip_generation.h) (related to dip::FillRadiusCoordinate)
    - dip_EllipticDistanceToPoint (dip_generation.h) (related to dip::FillRadiusCoordinate)
    - dip_CityBlockDistanceToPoint (dip_generation.h) (related to dip::FillRadiusCoordinate)
    - dip_TestObjectCreate (dip_generation.h)
    - dip_TestObjectModulate (dip_generation.h)
    - dip_TestObjectBlur (dip_generation.h)
    - dip_TestObjectAddNoise (dip_generation.h)
    - dip_ObjectCylinder (dip_generation.h)
    - dip_ObjectEdge (dip_generation.h)
    - dip_ObjectPlane (dip_generation.h)
    - dip_ObjectEllipsoid (dip_generation.h)
    - dip_DrawLineFloat (dip_paint.h) (merge into a single dip::DrawLine)
    - dip_DrawLineComplex (dip_paint.h) (merge into a single dip::DrawLine)
    - dip_DrawLinesFloat (dip_paint.h) (merge into a single dip::DrawLine)
    - dip_DrawLinesComplex (dip_paint.h) (merge into a single dip::DrawLine)
    - dip_PaintEllipsoid (dip_paint.h)
    - dip_PaintDiamond (dip_paint.h)
    - dip_PaintBox (dip_paint.h)

- diplib/geometry.h
    - dip_ResampleAt (dip_interpolation.h)
    - dip_AffineTransform (dip_interpolation.h) (was actually never finished)

- diplib/linear.h
    - dip_OrientedGauss (dip_linear.h)
    - dip_GaborIIR (dip_iir.h)
    - dip_Dgg (dip_derivatives.h)
    - dip_LaplacePlusDgg (dip_derivatives.h)
    - dip_LaplaceMinDgg (dip_derivatives.h)

- diplib/math.h
    - dip_RemapOrientation (dip_point.h)
    - dip_AmplitudeModulation (dip_math.h)
    - dip_CosinAmplitudeModulation (dip_math.h)
    - dip_CosinAmplitudeDemodulation (dip_math.h)
    - dip_ChangeByteOrder (dip_manipulation.h)
    - dip_SimpleGaussFitImage (dip_numerical.h)
    - dip_EmFitGaussians (dip_numerical.h)
    - dip_EmGaussTest (dip_numerical.h)

- diplib/measurement.h
    - dipio_MeasurementWriteCSV (dipio_msrcsv.h)

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
    - dip_UpperSkeleton2D (dip_binary.h)

- diplib/nonlinear.h
    - dip_RankContrastFilter (dip_rankfilters.h)
    - dip_Sigma (dip_filtering.h)
    - dip_BiasedSigma (dip_filtering.h)
    - dip_GaussianSigma (dip_filtering.h)
    - dip_NonMaximumSuppression (dip_filtering.h)
    - dip_ArcFilter (dip_bilateral.h)
    - dip_Bilateral (dip_bilateral.h) (all three flavours into one function)
    - dip_BilateralFilter (dip_bilateral.h) (all three flavours into one function)
    - dip_QuantizedBilateralFilter (dip_bilateral.h) (all three flavours into one function)
    - dip_AdaptiveGauss (dip_adaptive.h)
    - dip_AdaptiveBanana (dip_adaptive.h)
    - dip_StructureAdaptiveGauss (dip_adaptive.h)
    - dip_AdaptivePercentile (dip_adaptive.h)
    - dip_AdaptivePercentileBanana (dip_adaptive.h)
    - dip_PGST3DLine (dip_pgst.h) (this could have a better name!)
    - dip_PGST3DSurface (dip_pgst.h) (this could have a better name!)

- diplib/regions.h
    - dip_RegionConnectivity (dip_regions.h) (is experimental code)
    - dip_GrowRegions (dip_regions.h)
    - dip_GrowRegionsWeighted (dip_regions.h)

- diplib/segmentation.h
    - dip_KMeansClustering (dip_analysis.h)

- diplib/statistics.h
    - dip_PositionMaximum (dip_math.h)
    - dip_PositionMinimum (dip_math.h)
    - dip_PositionMedian (dip_math.h)
    - dip_PositionPercentile (dip_math.h)
    - dip_RadialMean (dip_math.h)
    - dip_RadialSum (dip_math.h)
    - dip_RadialMaximum (dip_math.h)
    - dip_RadialMinimum (dip_math.h)

- diplib/transform.h
    - dip_HartleyTransform (dip_transform.h)
