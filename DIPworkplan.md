---
title: 'Work plan for DIPlib 3.0'
author: 'Cris Luengo'
...

# Work plan for DIPlib 3.0

This is a list of tasks that need to be done, in order of dependencies.
This list includes dependencies and an estimate of how much design work needs
to be done before that component can be implemented.

Header files mentioned are from the currently released version of DIPlib ("the old DIPlib").

Priority for these items to be discussed. Framework and other infrastructure
elements have highest priority, because other things depend on them. Some filters
and algorithms are much more important than others.

The **(X)** markers indicate points that should be done first, and can be
done in parallel.

## What is aleady done:

1.  CMake compilation environment. Will need to be expended as new interfaces are
    created, etc.

1.  Test framework. We need to add more tests for the stuff that's already implemented.

1.  `class dip::Image`. Some features need to be tested more thoroughly.

1.  `dip::Framework::Scan()`. Will be tested more thoroughly through the functions
    that use it. Not yet parallelized.

1.  Arithmetic, bitwise and comparison operators (dependent on `dip::Framework::Scan()`)

1.  MATLAB interface is partially completed. It is possible to convert `dip::Image`
    objects to MATLAB arrays and vice-versa. No support yet for arrays, constants
    and strings. No support yet on the MATLAB side for a `dip_image` object.

1.  Image iterators. Need to be tested more thoroughly, some features have not been
    tested at all...
    Iterators are useful as simpler substitutes of the frameworks, to be used in only a
    few library functions, meant for the library user to implement pixel-wise processing,
    and filters, without the steep learning curve nor pointers.

1.  Measurement framework, a class to hold measurement data, and a class that knows all
    measurement features and can apply them to images. Depends on `dip::Framework::Scan()`.

1. `dip::Framework::Separable()`. Will be tested more thoroughly through the functions
   that use it. Not yet parallelized.


## What still needs to be done:

1.  **(X)**
    Test framework. We need to add many more tests for existing features.

1.  **(X)**
    `dip::Framework::Full()`. **Design mostly done**. Porting of pixel table
    code to a sensible class that makes its use easier (and that includes iterators).

1.  **(X)**
    Image I/O. Has high priority because it will make testing other functions easier.
    Porting current code in dipIO to read TIFF and ICS files. Interfacing to
    BioFormats.

1.  **(X)**
    Color support. **Design work done.** Port existing MATLAB code within
    current framework, and use `dip::Framework::Scan()` to apply conversions to all
    pixels in an image.

1.  **(X)**
    Port measurement features.
    - dip_measurement.h

1.  **(X)**
    Parallelization of frameworks. **Requires special expertise**. Decision:
    OpenMP or Intel TBB? The `dip::Framework::Scan()` and `dip::Framework::Separable`
    are ready to be parallelized.

1.  **(X)**
    MATLAB interface. **Requires special expertise**. Add conversion of strings,
    constants and arrays for input and output parameters. Rewrite the MATLAB
    `dip_image` class. Add support for the `dip_image` class in the MATLAB interface.
    MEX-files for DIPlib functions to be added as these functions are written.
    Certain functionality in the `dip_image` class depends on functionality in
    DIPlib that still needs to be written. The `dip_measurement` class needs
    rewriting too. `dipshow` to use a DIPlib function to generate an RGB image for
    display.

1.  Python interface. **Requires special expertise**. Using one of the C++/Python
    interface generators. Write interactive image display and GUI as exists in
    MATLAB. This can be developed in parallel to the MATLAB interface, or after
    the MATLAB interface is complete.

1.  Other interfaces. **Requires special expertise**. Header files that define
    functions to create a `dip::Image` object around image data from other libraries,
    as well as functions to convert a `dip::Image` to an image of that library
    (either as a view over the same data segment, or by copying the data). We'll
    do this for: OpenCV, ITK, SimpleITK. Any other libraries of interest?

1.  **(X)**
    Pixel-based algorithms built on `dip::Framework::Scan()`: monadic and
    diadic operators (i.e. the stuff in the old dip_math.h), statistics, etc. This is
    mostly porting old code to the new framework. There are already a few examples.
    - dip_derivatives.h
    - dip_noise.h
    - dip_point.h

1.  **(X)**
    Lookup table: we should have a single lookup table function that handles scalar,
    tensor and color images, with floating-point images using interpolation.
    **Requires design work**. Depends on `dip::Framework::Scan()`.
    - dip_lookup_table.h
    - dip_imarlut.h

1.  **(X)**
    Histograms. **Some design work needed** (a class to hold the histogram data, possibly
    based on `dip::Image` as is the case in the old DIPlib). The multi-dimensional
    histogram will work on tensor images also. Depends on `dip::Framework::Scan()`.
    - dip_histogram.h

1.  Global threhsold algorithms, depending on the histogram, currently implemented
    in dipimage/threshold.m.

1.  **(X)**
    Image generation algorithms using `dip::Framework::ScanSingleOutput()`.
    - dip_generation.h
    - dip_paint.h

1.  **(X)**
    Algorithms built on `dip::Framework::Separable()`: Gaussian filter, Fourier
    and other transforms, derivative filters, projections, etc. This is mostly porting
    old code to the new framework.
    - dip_iir.h
    - dip_interpolation.h
    - dip_linear.h (parts)
    - dip_manipulation.h (some functions, most are already implemented in `class dip::Image`)

1.  **(X)**
    The Fourier and associated transforms (built on `dip::Framework::Separable()`).
    The code in the old DIPlib is very slow. We could copy code from OpenCV as a
    free to use implementation, and provide an optional module that uses FFTW (not
    free, it's GPL which turns any application using it into GPL),

1.  Algorithms built on derivatives: the gradient, the structure tensor, etc.
    Mostly porting old code, but some changes expected due to output not being an
    image array, but a single tensor image.
    - dip_structure.h

1.  Algorithms built on `dip::Framework::Full()`: Rank filters, adaptive filters, etc.
    Simply porting old code.
    - dip_adaptive.h
    - dip_bilateral.h
    - dip_filtering.h
    - dip_rankfilters.h

1.  Algorithms built on `dip::Framework::Separable()` and `dip::Framework::Full()`:
    Filters that are separable for some filter shapes and non-separable for others,
    such as dilation and erosion, uniform filter, etc. Simply porting old code.
    - dip_morphology.h (parts)
    - dip_linear.h (parts)

1.  **(X)**
    Algorithms that do not depend on any framework: all binary morphology, the
    watershed, labelling, region growing, distance transforms, etc.
    Simply porting old code.
    - dip_binary.h
    - dip_distance.h
    - dip_morphology.h (parts)
    - dip_paint.h
    - dip_pgst.h
    - dip_regions.h

1.  Assorted stuff that depends on combinations of other algorithms, and stuff that
    I haven't looked yet into how it's implemented. Simply porting old code.
    - dip_analysis.h
    - dip_detection.h
    - dip_display.h
    - dip_findshift.h
    - dip_microscopy.h
    - dip_restoration.h

1.  Stuff that's in DIPimage:
    - 2D snakes (**requires design work**)
    - general 2D affine transformation, 3D rotation (is already C code)
    - xx, yy, zz, rr, phiphi, ramp; extend this to `Coordinates()`, which makes a
      tensor image (based on `dip::Framework::ScanSingleOutput()`)

1.  Other stuff that's not in the old DIPlib (see DIPthoughts.md).
    **Requires special expertise**, and presumably depends on frameworks and other
    infrastructure to be available.
