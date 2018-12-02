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

This is a list of tasks that still need to be done, not in any particular order.

**NOTE:** The online documentation is not updated continuously, please see
<a href="https://github.com/DIPlib/diplib/blob/master/src/documentation/workplan.md">this
document's source</a> for the most up-to-date version.

(2017/06/05) We just passed the 2000 documented entities (functions, classes, constants) mark!  
(2018/05/23) We now have well over 3000 documented entities and close to 100k lines of code.

## Gaps in infrastructure:

-   Test framework: We need to add more tests for some stuff that was implemented before
    the test framework was integrated.

-   Parallelization of some non-framework functions using *OpenMP*. For example, the core of the
    projection functions, and `dip::SelectionFilter` can be easily parallelized.

-   *DIPimage* toolbox: MEX-files for *DIPlib* functions to be added as these functions
    are written.

    For R2018a and newer, complex data are no longer stored in two separate data segments.
    This is good overall, but requires changes to the DIPlib-MATLAB interface. It is possible
    to compile *DIPimage* for R2018a or newer, but passing complex matrices in and out of
    the MEX-files causes two copies of the data, instead of none. `dip_image` objects never
    contain complex data (complex images are stored as real arrays to prevent copies).

    Another issue with R2018a and newer is that `mxGetPropertyShared` is no longer supported.
    This means that image data will get copied when used as input to DIPimage MEX-files.
    The only way around this is to use the new C++ interface. This might mean we'd need to
    rewrite all MEX-files.

-   *PyDIP* Python module: Write GUI as exists in *MATLAB*. Interface *DIPlib* functions
    to be added as these functions are written. Make the module more "Pythonic"?
    Find a way to automatically generate documentation from Doxygen, one approach is
    [here](https://stackoverflow.com/q/34896122/7328782).

-   Other interfaces. Header files that define
    functions to create a `dip::Image` object around image data from other libraries,
    as well as functions to convert a `dip::Image` to an image of that library
    (either as a view over the same data segment, or by copying the data). We have
    this for *OpenCV*. We can add *ITK* and/or *SimpleITK*, *Vigra*.
    Are there any other libraries of interest?

-   Improve style sheets for the documentation.

-   The Fourier transform: Further improvements could be making specific paths for real input or
    output, which could mean a small increase in performance. When compiled with *FFTW*, such
    paths already exist.


## Functionality currently not in *DIPlib* that would be important to include

-   Stuff that is in *DIPimage*:
    - 2D snakes
    - general 2D affine transformation, 3D rotation (is already C code)
    - ...and many more, see the *DIPimage 2* M-files for inspiration.

-   Image I/O: Interfacing to [*Bio-Formats*](http://www.openmicroscopy.org/site/products/bio-formats),
    using [JNI](http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/jniTOC.html)
    to interface between C++ and Java, see for example
    [this tutorial](https://www.codeproject.com/Articles/993067/Calling-Java-from-Cplusplus-with-JNI).
    See also [this ITK module](https://github.com/scifio/scifio-imageio) for
    interfacing to *Bio-Formats*, which uses [SCIFIO](https://github.com/scifio/scifio).
    This should be an optional module, as *Bio-Formats* is GPL.

-   Radon transform for lines and circles, Hough transform for lines (Wouter already did the one for circles).

-   Level-set segmentation, graph-cut segmentation.

-   Super pixels.

-   Building a graph out of a labeled image (e.g. from watershed). Graph format?
    Graph manipulation functions, e.g. MST (external lib?), region merging, etc.

-   Wavelet transforms.

-   Colocalization measurements.

-   Scalespace analysis?

-   A function to write text into an image, using the
    [*FreeType*](https://www.freetype.org) library.


## List of *DIPlib 2* functions that are not yet ported

Between brackets is the name of the old header file it's declared in. They are grouped by
the new header file they should be declared in.

Many of the following functions are not documented. Undocumented functions have a very
low priority in the porting process, and some might not be ported at all. It being listed
here is not an indication that the function needs to be ported, but if it's not listed here,
it should not be ported (or already is ported).

- diplib/analysis.h
    - `dip_OrientationSpace` (`dip_structure.h`)
    - `dip_ExtendedOrientationSpace` (`dip_structure.h`)
    - `dip_CurvatureFromTilt` (`dip_structure.h`)
    - `dip_OSEmphasizeLinearStructures` (`dip_structure.h`)
    - `dip_DanielsonLineDetector` (`dip_structure.h`)

- diplib/distance.h
    - `dip_FastMarching_PlaneWave` (`dip_distance.h`) (this function needs some input image checking!)
    - `dip_FastMarching_SphericalWave` (`dip_distance.h`) (this function needs some input image checking!)

- diplib/linear.h
    - `dip_OrientedGauss` (`dip_linear.h`)

- diplib/math.h
    - `dip_RemapOrientation` (`dip_point.h`)
    - `dip_ChangeByteOrder` (`dip_manipulation.h`)
    - `dip_SimpleGaussFitImage` (`dip_numerical.h`)
    - `dip_EmFitGaussians` (`dip_numerical.h`)
    - `dip_EmGaussTest` (`dip_numerical.h`)

- diplib/microscopy.h
    - `dip_RestorationTransform` (`dip_restoration.h`)
    - `dip_TikhonovRegularizationParameter` (`dip_restoration.h`)
    - `dip_TikhonovMiller` (`dip_restoration.h`)
    - `dip_Wiener` (`dip_restoration.h`)
    - `dip_PseudoInverse` (`dip_restoration.h`) (this name is wrong, maybe InverseFilterRestoration?)

- diplib/morphology.h
    - `dip_UpperEnvelope` (`dip_morphology.h`)
    - `dip_UpperSkeleton2D` (`dip_binary.h`)

- diplib/nonlinear.h
    - `dip_RankContrastFilter` (`dip_rankfilters.h`)
    - `dip_Sigma` (`dip_filtering.h`)
    - `dip_BiasedSigma` (`dip_filtering.h`)
    - `dip_GaussianSigma` (`dip_filtering.h`)
    - `dip_ArcFilter` (`dip_bilateral.h`)
    - `dip_StructureAdaptiveGauss` (`dip_adaptive.h`)
    - `dip_AdaptivePercentile` (`dip_adaptive.h`)
    - `dip_AdaptivePercentileBanana` (`dip_adaptive.h`)
    - `dip_PGST3DLine` (`dip_pgst.h`) (this could have a better name!)
    - `dip_PGST3DSurface` (`dip_pgst.h`) (this could have a better name!)

- diplib/transform.h
    - `dip_HartleyTransform` (`dip_transform.h`)


## List of *DIPimage 2* functions that still need to be ported

Some functions that haven't been ported are not listed here -- these will not be ported.

Pure M-files:
- `afm_flatten`
- `backgroundoffset`
- `correctshift`
- `cpf`
- `dpr`
- `find_lambda`
- `findlocalshift`
- `findlocmax`
- `frc`
- `jpeg_quality_score`
- `lfmse`
- `mappg`
- `mcd`
- `morphscales`
- `opticflow`
- `orientationplot`
- `quadraturetensor` (could be based on loggabor?)
- `scale2rgb`
- `scalespace`
- `tikhonovmiller`
- `wiener`

Requiring C++ code:
- `arcf`       (depends on `dip_arcfilter`)
- `cal_readnoise`
- `deblock`    (depends on `arcf`)
- `distancebetweenpointsets`
- `findospeaks`
- `hybridf`    (depends on `arcf`)
- `localshift`
- `nufft_type1`
- `nufft_type2`
- `orientationspace`
- `percf_adap`
- `percf_adap_banana`
- `pst`
- `radoncircle`
- `rotation3d`
- `splitandmerge`
- `structf`    (depends on `dip_arcfilter`)
- `write_add`

Interface to Rainer Heintzmann's 5D Viewer written in Java:
- `view5d`

Color stuff, which needs lots of work to bring in shape:
- `change_chroma`
- `change_gamma`
- `change_xyz`
- `color_rotation`
- `gamut_destretch`
- `gamut_mapping`
- `gamut_stretch`
- `huecorr`
- `iso_luminance_lines`
- `luminance_steered_dilation`
- `luminance_steered_erosion`
- `make_gamut`
- `measure_gamma_monitor`
- `mon_rgb2xyz`
- `mon_xyz2rgb`
- `out_of_gamut`
- `plot_gamut`
- `print_cmy2xyz`
- `print_xyz2cmy`
- `rgb_to_border`
- `scan_rgb2xyz`
- `scan_xyz2rgb`
- `scanner_calibration`
- `spectra2xyz`

ICC profile stuff to be done through LittleCMS, no point in maintaining our own implementation:
- `monitor_icc`
- `printer_icc`
- `scanner_icc`
- `read_icc_profile`
- `write_icc_profile`
