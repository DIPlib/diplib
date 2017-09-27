# Changes from DIPlib 2.x (the old DIPlib) {#changes}

[//]: # (DIPlib 3.0)

[//]: # ([c]2016-2017, Cris Luengo.)
[//]: # (Based on original DIPlib code: [c]1995-2014, Delft University of Technology.)
[//]: # (Based on original DIPimage code: [c]1999-2014, Delft University of Technology.)

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

*DIPlib* 3.0 is a complete rewrite in C++ of the *DIPlib* 1.x/2.x infrastructure, which was written
in C; only the code that implements actual image processing and analysis algorithms is ported
over.

The list below describes infrastructure changes, function interface changes, and functionality
changes in the *DIPlib* library, as well as changes to the *DIPimage* toolbox. It is possible
that some changes were not listed here, but hopefully this list will help in porting your old
code that used *DIPlib* or *DIPimage* to the new version.

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

   - Please read the documentation to the `dip::Image` class before doing any work with the
     library!

- The framework functions all have different names:
    - `dip_SeparableFrameWork` &rarr; `dip::Framework::Separable`
    - `dip_MonadicFrameWork` &rarr; `dip::Framework::ScanMonadic` / `dip::Framework::SeparableMonadic`
    - `dip_MonadicPoint` &rarr; `dip::Framework::ScanMonadic`
    - `dip_MonadicPointData` &rarr; `dip::Framework::ScanMonadic`
    - `dip_PixelTableArrayFrameWork` &rarr; doesn't exist any more, was used meaningfully only in `dip_GeneralisedKuwaharaImproved`
    - `dip_PixelTableFrameWork` &rarr; `dip::Framework::Full`
    - `dip_ScanFrameWork` &rarr; `dip::Framework::Scan()` / `dip::Framework::ScanDyadic`
    - `dip_SingleOutputFrameWork` &rarr; `dip::Framework::ScanSingleOutput`
    - `dip_SingleOutputPoint` &rarr; `dip::Framework::ScanSingleOutput`

  Their interfaces are not exactly compatible, but it should be relatively straightforward
  to port old line functions to use the new framework, yielding shorter code.
  However, all line functions are now expected to use strides.

- `dip_ovl.h` is now `diplib/overload.h`, and works like a normal header file defining
  macros to use in your code. `dip_tpi.h` is gone, the template strategy to define overloaded
  functions is now based on C++ templates, yielding code that is easier to write and easier
  to read.

- There is no longer a `dip_Initialise` function. There are no global variables. There are
  no registries.

- There is no longer a `dip_ImagesSeparate` function. Its functionality can be accomplished
  using a simple image copy (the copy shares the image data with the original image, and
  makes it possible to strip the original image while still keeping the input data available).

- Header files used to have names such as `dip_xxx.h`, they now are `diplib/xxx.h`. The "`xxx`"
  part has remained the same in most cases, though some function declarations have moved to a
  different header file. The documentation specifies which header file to include for each
  function.

- The *dipIO* library no longer exists. Some `dipio_Xxx` functions are now defined in the
  `diplib/file_io.h` header file (reading and writing ICS and TIFF files).
  `dipio_MeasurementWriteCSV` is now a method to `dip::Measurement` (TODO!).
  `dipio_Colour2Gray` functionality is replaced by `dip::ColorSpaceManager`.
  Other functionality no longer exists.

## Changes in algorithm interface

- Function parameters expressing options are now represented by strings rather than
  \c \#define or `enum` constants (except for low-level library functionality).

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

- `dip_Measure` is now `dip::MeasurementTool::Measure`, with `dip::MeasurementTool` an object
  that knows about defined measurement features.

   - Measurement features are registered with a `dip::MeasurementTool` object, not in the
     global registry.

   - Measurement features have a much simpler programmer's interface, it should be easier to
     write new features now.

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
  of the old *DIPlib*. Other filters that had a filter shape argument now use a
  `dip::Kernel` to collect three parameters.

- `dip_Min` and `dip_Max` are now `dip::Infimum` and `dip::Supremum`, and `dip_SignedMinimum`
  is now `dip::SignedInfimum`. There are too many functions with "Maximum" and "Minimum" in
  their name.

- `dip_MultiDimensionalHistogram` and `dip_ImageToHistogram` have been merged into the
  constructor for `dip::Histogram`, which replaces both `dip_Distribution` and `dip_Histogram`.

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

- `dip_LocalMinima` is now called `dip::WatershedMinima`. There's also a maxima variant.

- `dip_GetMaximumAndMinimum` is now called `dip::MaximumAndMinimum` for consistency.

- `dip_ResamplingFT` is no longer a separate function, call `dip::Resampling` with "ft" as
  method. `dip_Skewing` was renamed to `dip::Skew` for consistency. `dip_Rotation` has
  been renamed `dip::Rotation2d`, and a generalized `dip::Rotation` is similar but takes
  two additional integers to specify the rotation plane. `dip_Rotation3d_Axis` and
  `dip_Rotation3d` are now both called `dip::Rotation3d`, their argument types are different.
  `dip::Resampling`, `dip::Skew`, `dip::Rotation` and similar now take a boundary condition
  as optional argument.

- `dip_KuwaharaImproved` is now called `dip::Kuwahara`, and `dip_Kuwahara` is no longer
  (C++ default values make it redundant).

- `dip_GeneralisedKuwaharaImproved` is now called `dip::SelectionFilter`, and
  `dip_GeneralisedKuwahara` is no longer (C++ default values make it redundant).

## Changes in functionality

- Second order extrapolation boundary extension didn't do as advertised in the old *DIPlib*.
  Also the first order extrapolation couldn't have worked correctly with unsigned integers.
  The new implementation fits a 2<sup>nd</sup> order polynomial that reaches 0 at the end of the extended
  boundary, yielding a continuous first derivative at the boundary. A third order extrapolation
  has been added, which works similarly but yields a continuous second derivative at the boundary.
  These functions are quite noise sensitive, however, and I expect they might produce high
  frequencies along the edge.

- Fixed little bug in the "Perimeter" measurement feature, which didn't check the corner count
  for the transition from the last to the first chain code, yielding a perimeter that was often
  0.0910 pixels too long. The feature "Radius" now computes the center of mass of the object
  correctly.

- The measurement features "Skewness" and "ExcessKurtosis" are no longer separate features, but
  instead put together in the new "Statistics" feature, which also computes the mean and standard
  deviation. The measurement feature "StdDev" is now called "StandardDeviation".
  The algorithms to compute these statistics have changed to be more stable. The measurement
  feature "Sum" was an alias to "Mass", and is not (yet?) available.

- New measurement features: "EllipseVariance", "DimensionsCube", "DimensionsEllipsoid",
  "GreyDimensionsCube", and "GreyDimensionsEllipsoid". The features "Mu", "Inertia", "MajorAxes",
  and their grey-value versions have been generalized to arbitrary number of dimensions.

- `dip::SeparableConvolution` treats input filter definitions slightly differently, and there
  no longer are "left" and "right" options.

- `dip::ImageDisplay` no longer does any spatial scaling. Also, it's a class, not a function.

- `dip::ColorSpaceManager` is functionality ported from MATLAB-code in the old *DIPimage*,
  with a few new color spaces added.

- `dip::FourierTransform` now does normalization in the more common way (forward transform not
  normalized, inverse transform normalized by 1/N), but an option ("symmetric") allows to change
  the normalization to be consistent with the old *DIPlib*, which used a symmetric normalization
  scheme (both forward and backward transforms use 1/N<sup>1/2</sup>)

- `dip::Histogram` misses a few of the options that `dip_MultiDimensionalHistogram` had, but I
  don't think they are relevant. They can be added easily if necessary.

- Many threshold selection strategies have been ported from MATLAB code in *DIPimage*, see
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
  more efficient. The "diamond" SE is now implemented using line SEs, and we have added an
  "octagonal" SE that is computed as a combination of a diamond SE and a rectangular SE.

- New morphological functions: `dip::HMaxima`, `dip::HMinima`, `dip::OpeningByReconstruction`,
  and `dip::ClosingByReconstruction`.

- `dip::Resampling` (and by extension `dip::Shift`) shifts the image in the opposite direction
  from what it did in the old *DIPlib*, where the shift was unintuitive. `dip::Skew` can now skew
  in multiple dimensions at the same time. `dip::Rotation` now works for any number of dimensions,
  though it only rotates around one cartesian axis.

- `dip::FindShift` now returns the shift with inverse sign compared to before, to match the reversal
  in `dip::Shift`.

- `dip::ImageWriteICS` now has a `"fast"` option that causes dimensions to be written to file in
  the order in which they are stored in memory, rather than in standard increasing order. This
  makes writing a lot faster if strides are non-standard. `dip::ImageReadICS` has a similar option
  that re-orders strides for the output image to match those of the file, again potentially decreasing
  reading times significantly.

## Changes from DIPimage 2.x (the old DIPimage)

- The `dip_image` object has changed completely internally. Pixel data is stored differently:
  tensor images have all samples in the same *MATLAB* array. Complex images are stored as a
  single real matrix, with real and complex samples next to each other (this translates
  much more nicely to *DIPlib*, where complex data is stored in that same way).

  Consequently, the `dip_image_array` pseudo-class no longer exists. Both scalar and tensor
  images report to be of class `dip_image`. If you need to store multiple images in one
  object, create a cell array instead.

  Additionally, the tensor no longer can hold arbitrary number of dimensions, it is limited
  to vectors and matrices, as it is in *DIPlib*. I don't expect this to impact any user's code,
  and I'd be happy to hear if you actually used tensors with more than two dimensions.

  We tried keeping how the `dip_image` object is used as similar as possible to how it was
  in the old *DIPimage*, so that users need not change their code. Nonetheless, some changes
  must occur:

  - The `size` method was discouraged, we asked users to use `imsize` and `imarsize` instead.
    Those two functions work as before, except that they don't return 1 for dimensions that
    don't exist. `size` now always works like `imsize`, except that it always returns at least
    two values, and will return 1 for non-existing dimensions. This makes it similar to the
    default `size`, and plays nice with the `whos` command. `tensorsize` is like `imarsize`,
    but with a more sensical name, since image arrays no longer exist.

  - `end` was discouraged as well, as the meaning changed from `dip_image` to `dip_image_array`.
    Now `end` will always work for spatial dimensions, and cannot be used with tensor indexing.
    That is, `end` will work in `()` indexing, but not in `{}` indexing.

  - Methods that changed behavior for scalar and tensor images (i.e. `sum`, `max`, etc.) now
    always operate only along spatial dimensions. To sum over the tensor dimension, convert
    it to a spatial dimension using `tensortospatial`.

  - Likewise, methods such as `cat` used to concatenate images in an `dip_image_array`, and
    consequently concatenated the various tensor elements as separate images. These functions
    now all work along spatial dimensions only, leaving the tensor dimension unaffected.

  - `imarfun` and `iterate` no longer exist. There should be no need for them as all functions
    recognize tensor images. `slice_op` can replace `iterate` if needed (TODO: `slice_op` is not
    yet implemented).

  - The `reshape` method now takes pixels column-wise instead of row-wise from the input. This
    is the natural way of doing it, as it doesn't require data copy. I don't remember why it
    was row-wise in the old *DIPimage*, and I presume there are few (if any) programs that
    depend on the old behavior.

  - Related to the previous point, `squeeze` now might reorder dimensions. But it's cheaper
    this way!

  - The `inner` and `outer` methods no longer exist, use `cross` and `dot`.

- The `dip_measurement` object has changed completely internally. The interface is identical
  except:

  - It is not (yet?) possible to add a feature.

  - It is no longer possible to convert to/from a `struct`. However, it is possible to convert
    to a `table`.

  - Fixed a bug: `msr.featureID` now returns an array that is transposed w.r.t. previous versions.
    This was a bug that we never fixed because of backwards compatability, we took this
    opportunity to fix it. Now we have total consistency: no matter how the measurement data is
    extracted or converted, objects are always rows, and features are always columns.

- `dipsetpref` and `dipgetpref` have fewer settings than in the old *DIPimage*. Some settings
  were linked to *DIPlib* global variables, none of which exist any more, and some others
  are simply no longer relevant.

- `newimar` is now in the `alias` subdirectory, and identical to the new function `newtensorim`.
  Other functions that used to be in the `alias` directory are no longer. We recommend that
  you correct affected code, but if you want, you can always create those aliases again.

- Many filters now have a boundary condition parameter. In the old *DIPimage* one would change
  the boundary condition through a global parameter, which no longer exists. If you never
  touched the global parameter, nothing should change for you. If you did change this global
  parameter in a program, you now need to pass the value to the relevant functions instead.

- Many functions have been added to match new functionality in *DIPlib*, as well as previous
  functionality that was not accessible from MATLAB. Some old functions have gained additional
  parameters to expose more functionality; we tried to do so without breaking backwards
  compatibility.

- Some functions have changed in a way that is not backwards compatible, either by having a
  different interface or different behavior:

  - `dilation_se`, `erosion_se`, etc. are now folded into `dilation`, `erosion`, etc. For
    functions with an `_se` appended to the name, remove the `_se`. Alternatively, the `alias`
    sub-directory contains these names and forwards the calls to the correct functions.

  - `rankmax_opening` and `rankmin_closing` now have `rank` as the second parameter, instead
    of `percentile`. This should make them match the literature better, and make them more
    usable.

  - `smooth` is no longer relevant, moved to the `alias` directory.

  - `derivative` has the 2<sup>nd</sup> and 3<sup>rd</sup> arguments switched, it makes more
    sense having the order first.

  - `mdhistogram` has fewer options, but should still be able to produce the same results as
    previously.

  - `resample` and `shift` shift the image in the opposite direction from what it did in the old
    *DIPimage*, where the shift was unintuitive.

  - `readim` and `writeim` work differently now, in part because *DIPlib* natively only supports
    two file types now. The `file_info` struct output for `readim` has changed somewhat. The last
    two input parameters to the old `writeim` are no longer supported (`compression` and `physDim`):
    to change the compression method, call `writeics` or `writetiff` directly; the pixel size is
    always given by the image, use `dip_image/pixelsize` to set it.

  - `countneighbours` has been renamed to `countneighbors` for consistency in spelling.

  - `mse`, `mae` and several similar functions have been collected in a new function `errormeasure`.
    The old functions are still available in the `alias` sub-directory.

- New functions not mentioned above: `areaopening`, `coordinates`, `extendimage`, `getmaximumandminimum`,
  `getsamplestatistics`, `lee`, `pathopening`, `select`, `setborder`, `skew`, `smallobjectsremove`,
  `thetatheta`.
