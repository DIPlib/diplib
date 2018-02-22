# Work plan for DIPlib 3 {#workplan}

[//]: # (DIPlib 3.0)

[//]: # ([c]2016-2018, Cris Luengo.)
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

Header files mentioned are from *DIPlib 2*.

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

-   The Fourier transform: built-in code is based on the code out of OpenCV, much faster than
    the version in *DIPlib 2*; optionally uses [*FFTW*](http://www.fftw.org) instead
    (which is GPL).

-   Histograms, including multi-dimensional histograms constructed from tensor images.

-   Look-up table, a single object that includes functionality of various related look-up
    functions in *DIPlib 2*.

-   Global threshold algorithms, ported from a *DIPimage* M-file.

-   Random number generation, using the new PCG scheme.

-   Image I/O (ICS and TIFF formats, the plan is to not support other formats going forward).

-   An increasing number of filters and operators.

-   Interactive image display, as a separate module *DIPviewer*.

-   *MATLAB* interface.

-   *DIPimage* toolbox: The infrastructure is complete (the `dip_image` and `dip_measurement`
    objects are defined and all methods ported; most `dipshow` functionality is ported; the
    GUI is ported and partially rewritten). Functions are being added as functionality is ported
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

-   Parallelization of some non-framework functions using *OpenMP*. For example, the core of the
    projection functions, and `dip::SelectionFilter` can be easily parallelized.

-   Porting filters, analysis routines, etc. See the list at the bottom of this page.

-   The Fourier transform: Further improvements could be making specific paths for real input or
    output, which could mean a small increase in performance. When compiled with *FFTW*, such
    paths already exist.

-   Stuff that is in *DIPimage*:
    - 2D snakes
    - general 2D affine transformation, 3D rotation (is already C code)
    - ...and many more, see the *DIPimage 2* M-files for inspiration.

-   Other stuff that's not in *DIPlib 2* (see below).

-   *DIPimage* toolbox: MEX-files for *DIPlib* functions to be added as these functions
    are written.

-   *PyDIP* Python module: Write GUI as exists in *MATLAB*. Interface *DIPlib* functions
    to be added as these functions are written. Make the module more "Pythonic"?
    Find a way to automatically generate documentation from Doxygen, one approach is
    [here](https://stackoverflow.com/q/34896122/7328782).

-   Other interfaces. Header files that define
    functions to create a `dip::Image` object around image data from other libraries,
    as well as functions to convert a `dip::Image` to an image of that library
    (either as a view over the same data segment, or by copying the data). We'll
    do this for: *OpenCV*, *ITK*, *SimpleITK*. Any other libraries of interest?

-   Style sheets for the documentation.

## Functionality currently not in *DIPlib* that would be important to include

- We're also lacking some other morphological filters:
    - hit'n'miss, where the interval is rotated over 180, 90 or 45 degrees.
    - levelling

- The monogenic signal, including derived quantities, using a similar interface to that used
  for the structure tensor.

- Radon transform for lines and circles, Hough transform for lines.

- Level-set segmentation, graph-cut segmentation.

- Super pixels.

- Building a graph out of a labeled image (e.g. from watershed). Graph format?
  Graph manipulation functions, e.g. MST (external lib?), region merging, etc.

- Wavelet transforms.

- A function to write text into an image, using the [*FreeType*](https://www.freetype.org) library.


## List of *DIPlib 2* functions that still need to be ported

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
    - dip_OrientationSpace (dip_structure.h)
    - dip_ExtendedOrientationSpace (dip_structure.h)
    - dip_CurvatureFromTilt (dip_structure.h)
    - dip_OSEmphasizeLinearStructures (dip_structure.h)
    - dip_DanielsonLineDetector (dip_structure.h)

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

- diplib/linear.h
    - dip_OrientedGauss (dip_linear.h)

- diplib/math.h
    - dip_RemapOrientation (dip_point.h)
    - dip_AmplitudeModulation (dip_math.h)
    - dip_CosinAmplitudeModulation (dip_math.h)
    - dip_CosinAmplitudeDemodulation (dip_math.h)
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
    - dip_PseudoInverse (dip_restoration.h) (this name is wrong, maybe InverseFilterRestoration?)

- diplib/morphology.h
    - dip_UpperEnvelope (dip_morphology.h)
    - dip_UpperSkeleton2D (dip_binary.h)

- diplib/nonlinear.h
    - dip_RankContrastFilter (dip_rankfilters.h)
    - dip_Sigma (dip_filtering.h)
    - dip_BiasedSigma (dip_filtering.h)
    - dip_GaussianSigma (dip_filtering.h)
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

- diplib/transform.h
    - dip_HartleyTransform (dip_transform.h)
