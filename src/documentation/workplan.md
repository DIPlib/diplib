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

## What is already done:

-   CMake compilation environment.

-   Test framework. We need to add more tests for the stuff that's already implemented.

-   `class dip::Image`. Some features need to be tested more thoroughly.

-   `dip::Framework::Scan`, `dip::Framework::Separable` and `dip::Framework::Full`.
    These frameworks are the core of most algorithms. They will be tested more thoroughly
    through the functions that use it. Not yet parallelized.

-   Arithmetic, bitwise and comparison operators.

-   Image iterators. Need to be tested more thoroughly, some features have not been
    tested at all...
    Iterators are useful as simpler substitutes of the frameworks, to be used in only a
    few library functions, meant for the library user to implement pixel-wise processing,
    and filters, without the steep learning curve nor pointers.

-   Measurement framework, a class to hold measurement data, and a class that knows all
    measurement features and can apply them to images.
    All measurement features have been ported, including the previously only defined in
    *DIPimage*.

-   Histograms, including multi-dimensional histograms constructed from tensor images.

-   The Fourier transform (based on the code out of OpenCV, much faster than the code in
    the old *DIPlib*).

-   An increasing number of filters and operators based on the various frameworks.

-   Color support. More color spaces could be added in time.

-   Interactive image display, as a separate module DIPviewer.

-   *MATLAB* interface.

-   *DIPimage* toolbox: The `dip_image` object is defined and has most methods defined.
    The `dipshow` function works, but needs further testing and refinement.
    More to be added as the corresponding *DIPlib* functions are implemented.

## What still needs to be done:

-   Test framework. We need to add more tests for the stuff that's already implemented.

-   Image I/O. Has high priority because it will make testing other functions easier.
    Porting current code in dipIO to read TIFF and ICS files. Interfacing to
    BioFormats.

-   Parallelization of frameworks. **Requires special expertise**. Decision:
    OpenMP or Intel TBB? The `dip::Framework::Scan` and `dip::Framework::Separable`
    are ready to be parallelized.

-   *MATLAB* interface and *DIPimage* toolbox. **Requires special expertise**.
    MEX-files for *DIPlib* functions to be added as these functions are written.
    The `dip_measurement` class needs to be rewritten. The `dipimage` GUI needs to
    be ported, with function input parameter definitions to be provided by `dipmenus`.

-   Python interface. **Requires special expertise**. Using one of the C++/Python
    interface generators. Write interactive image display and GUI as exists in
    *MATLAB*. This can be developed in parallel to the *MATLAB* interface, or after
    the *MATLAB* interface is complete.

-   Other interfaces. **Requires special expertise**. Header files that define
    functions to create a `dip::Image` object around image data from other libraries,
    as well as functions to convert a `dip::Image` to an image of that library
    (either as a view over the same data segment, or by copying the data). We'll
    do this for: OpenCV, ITK, SimpleITK. Any other libraries of interest?

-   Pixel-based algorithms built on `dip::Framework::Scan`: monadic and dyadic
    operators, statistics, etc. This is mostly porting old code to the new framework.
    - dip_math.h (many already done)
    - dip_noise.h
    - dip_point.h

-   Lookup table: we should have a single lookup table function that handles scalar,
    tensor and color images, with floating-point images using interpolation.
    **Requires design work**. Depends on `dip::Framework::Scan`.
    - dip_lookup_table.h
    - dip_imarlut.h

-   Global threshold algorithms, depending on the histogram, currently implemented
    in dipimage/threshold.m (Otsu, triangle, background, etc.).

-   Image generation algorithms using `dip::Framework::ScanSingleOutput`.
    - dip_generation.h
    - dip_paint.h

-   Filters. This is mostly porting old code to the new frameworks.
    - dip_derivatives.h (a few functions left)
    - dip_iir.h (Gabor)
    - dip_interpolation.h
    - dip_linear.h (a few functions left)
    - dip_manipulation.h (a few functions, most are already implemented in `dip::Image`)
    - dip_adaptive.h
    - dip_bilateral.h
    - dip_filtering.h
    - dip_rankfilters.h

-   The Fourier transform: Use *FFTW* when a compile switch is set, we must be able to
    disable that so *DIPlib* can be used in non-open-source projects. Further improvements
    could be making specific paths for real input or output (could mean a small increase in
    performance).

-   RadialAngular filters, orientation space, structure tensor.
    Mostly porting old code, but some changes expected due to output not being an
    image array, but a single tensor image.
    - dip_structure.h

-   Algorithms that do not depend on any framework: all binary morphology, labelling,
    region growing, distance transforms, etc.
    Simply porting old code.
    - dip_binary.h
    - dip_distance.h
    - dip_morphology.h (the parts that are not yet ported)
    - dip_paint.h
    - dip_pgst.h
    - dip_regions.h

-   **DEP**
    Assorted stuff that depends on combinations of other algorithms, and stuff that
    I haven't looked yet into how it's implemented. Simply porting old code.
    - dip_analysis.h
    - dip_detection.h
    - dip_findshift.h
    - dip_microscopy.h
    - dip_restoration.h

-   Stuff that is in *DIPimage*:
    - **DEP** 2D snakes (**requires design work**)
    - general 2D affine transformation, 3D rotation (is already C code)
    - xx, yy, zz, rr, phiphi, ramp; extend this to `dip::Coordinates`, which makes a
      tensor image (based on `dip::Framework::ScanSingleOutput`)

-   Other stuff that's not in the old *DIPlib* (see below).
    **Requires special expertise**.


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

- The `dip::Label` function should return the number of labels. It could optionally also
  return the sizes of the objects, since these are counted anyway. The labelling algorithm
  by Mike is quite efficient, but we should compare with the more common union-find
  algorithm, which is likely to be optimal for this application (Mike's code uses a
  priority queue, union-find doesn't need it).
