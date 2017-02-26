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

Header files mentioned are from *DIPlib* 2.0 ("the old *DIPlib*").

Priority for these items to be discussed. Framework and other infrastructure
elements have highest priority, because other things depend on them. Some filters
and algorithms are much more important than others.

The **DEP** markers indicate points that depend on functionality not yet implemented.

## What is aleady done:

-   CMake compilation environment. Will need to be expanded as new interfaces are
    created, etc.

-   Test framework. We need to add more tests for the stuff that's already implemented.

-   `class dip::Image`. Some features need to be tested more thoroughly.

-   `dip::Framework::Scan`, `dip::Framework::Separable` and `dip::Framework::Full`.
    These frameworks are the core of most algorithms. They will be tested more thoroughly
    through the functions that use it. Not yet parallelized.

-   Arithmetic, bitwise and comparison operators (dependent on `dip::Framework::Scan`)

-   Image iterators. Need to be tested more thoroughly, some features have not been
    tested at all...
    Iterators are useful as simpler substitutes of the frameworks, to be used in only a
    few library functions, meant for the library user to implement pixel-wise processing,
    and filters, without the steep learning curve nor pointers.

-   Measurement framework, a class to hold measurement data, and a class that knows all
    measurement features and can apply them to images.
    All measurement features have been ported, including the previously only defined in
    *DIPimage*.

-   *MATLAB* interface is partially completed: It is possible to convert `dip::Image`
    objects to MATLAB `dip_image` objects and vice-versa, as well as a series of numeric
    and string parameter types.

-   *DIPimage* toolbox: The `dip_image` object is defined and has a few functions,
    including all binary and unary operators in one MEX-file, and the `measure` function
    in another MEX-file.

## What still needs to be done:

-   Test framework. We need to add many more tests for existing features.

-   Image I/O. Has high priority because it will make testing other functions easier.
    Porting current code in dipIO to read TIFF and ICS files. Interfacing to
    BioFormats.

-   Color support. **Design work done**. Port existing *MATLAB* code within
    current framework, and use `dip::Framework::Scan` to apply conversions to all
    pixels in an image.

-   Parallelization of frameworks. **Requires special expertise**. Decision:
    OpenMP or Intel TBB? The `dip::Framework::Scan` and `dip::Framework::Separable`
    are ready to be parallelized.

-   *MATLAB* interface and *DIPimage* toolbox. **Requires special expertise**.
    MEX-files for *DIPlib* functions to be added as these functions are written.
    The `dip_measurement` class needs to be rewritten.
    `dipshow` to use a *DIPlib* function to generate an RGB image for display.

-   Python interface. **Requires special expertise**. Using one of the C++/Python
    interface generators. Write interactive image display and GUI as exists in
    *MATLAB*. This can be developed in parallel to the *MATLAB* interface, or after
    the *MATLAB* interface is complete.

-   Other interfaces. **Requires special expertise**. Header files that define
    functions to create a `dip::Image` object around image data from other libraries,
    as well as functions to convert a `dip::Image` to an image of that library
    (either as a view over the same data segment, or by copying the data). We'll
    do this for: OpenCV, ITK, SimpleITK. Any other libraries of interest?

-   Pixel-based algorithms built on `dip::Framework::Scan`: monadic and
    diadic operators (i.e. the stuff in the old dip_math.h), statistics, etc. This is
    mostly porting old code to the new framework.
    - dip_math.h
    - dip_noise.h
    - dip_point.h

-   Lookup table: we should have a single lookup table function that handles scalar,
    tensor and color images, with floating-point images using interpolation.
    **Requires design work**. Depends on `dip::Framework::Scan`.
    - dip_lookup_table.h
    - dip_imarlut.h

-   Histograms. **Some design work needed** (a class to hold the histogram data, possibly
    based on `dip::Image` as is the case in the old *DIPlib*). The multi-dimensional
    histogram will work on tensor images. Depends on `dip::Framework::Scan`.
    - dip_histogram.h

-   **DEP**
    Global threhsold algorithms, depending on the histogram, currently implemented
    in dipimage/threshold.m (Otsu, triangle, background, etc.).

-   Image generation algorithms using `dip::Framework::ScanSingleOutput`.
    - dip_generation.h
    - dip_paint.h

-   Algorithms built on `dip::Framework::Separable`: Gaussian filter, Fourier
    and other transforms, derivative filters, projections, etc. This is mostly porting
    old code to the new framework.
    - dip_derivatives.h
    - dip_iir.h
    - dip_interpolation.h
    - dip_linear.h (parts)
    - dip_manipulation.h (some functions, most are already implemented in `class dip::Image`)

-   The Fourier and associated transforms (built on `dip::Framework::Separable`).
    The code in the old *DIPlib* is very slow. We could copy code from OpenCV as a
    free to use implementation, and provide an optional module that uses FFTW (not
    free, it's GPL which turns any application using it into GPL),

-   **DEP**
    Algorithms built on derivatives: the gradient, the structure tensor, etc.
    Mostly porting old code, but some changes expected due to output not being an
    image array, but a single tensor image.
    - dip_structure.h

-   Algorithms built on `dip::Framework::Full`: Rank filters, adaptive filters, etc.
    Simply porting old code.
    - dip_adaptive.h
    - dip_bilateral.h
    - dip_filtering.h
    - dip_rankfilters.h

-   Algorithms built on `dip::Framework::Separable` and `dip::Framework::Full`:
    Filters that are separable for some filter shapes and non-separable for others,
    such as dilation and erosion, uniform filter, etc. Simply porting old code.
    - dip_morphology.h (parts)
    - dip_linear.h (parts)

-   Algorithms that do not depend on any framework: all binary morphology, the
    watershed, labelling, region growing, distance transforms, etc.
    Simply porting old code.
    - dip_binary.h
    - dip_distance.h
    - dip_morphology.h (parts)
    - dip_paint.h
    - dip_pgst.h
    - dip_regions.h

-   **DEP**
    Assorted stuff that depends on combinations of other algorithms, and stuff that
    I haven't looked yet into how it's implemented. Simply porting old code.
    - dip_analysis.h
    - dip_detection.h
    - dip_display.h
    - dip_findshift.h
    - dip_microscopy.h
    - dip_restoration.h

-   Stuff that is in DIPimage:
    - **DEP** 2D snakes (**requires design work**)
    - general 2D affine transformation, 3D rotation (is already C code)
    - xx, yy, zz, rr, phiphi, ramp; extend this to `dip::Coordinates`, which makes a
      tensor image (based on `dip::Framework::ScanSingleOutput`)

-   Other stuff that's not in the old *DIPlib* (see below).
    **Requires special expertise**, and presumably depends on frameworks and other
    infrastructure to be available.


## Functionality currently not in *DIPlib* that would be important to include

- An overlay function that adds a binary or labelled image on top of a grey-value or 
  color image.

- Stain unmixing for bright-field microscopy.

- Some form of image display for development and debugging. We can have the users resort
  to third-party libraries or saving intermediate images to file, or we can try to copy
  *OpenCV*'s image display into *dipIO*.

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

- The `dip::Label` function should return the number of labels. It could optionally also
  return the sizes of the objects, since these are counted anyway. The labelling algorithm
  by Mike is quite efficient, but we should compare with the more common union-find
  algorithm, which is likely to be optimal for this application (Mike's code uses a
  priority queue, union-find doesn't need it).
