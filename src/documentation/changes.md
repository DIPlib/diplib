# Changes from *DIPlib* 2.0 (the old *DIPlib*) {#changes}

*DIPlib* 3.0 is a complete rewrite in C++ of the *DIPlib* 2.0 infrastructure, which was written
in C; only the code that implements actual image processing and analysis algorithms is ported
over. 

## Core/infrastructure changes

- Functions and types used to start with `dip_`, now they are in the `dip::` namespace. All
  

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

   - `dip_Assimilate` is the `dip::Image::ReForge`.

   - We used to call the image size its dimension. `dip_Dimensions` is now `dip::Image::Sizes`.
     The reason is that it was too confusing talking about a dimension as an image axis (the
     2nd dimension), and the dimension of that dimension (the size of the image along that
     axis).

   - Images now can have a tensor as pixel value, as was possible in DIPimage. To port old
     functions, you can test for `dip::Image::IsScalar` and ignore this new feature, or
     modify the old function to work with tensor values.

   - Images now carry the physical dimension in them (referred to as pixel size). When porting
     functions, think about whether this data needs to be maintained, modified, or removed.

   - The same is true for the color space. A function that changes the number of thensor
     elements must also remove the color space information.

   - Please read the documentation to the `dip::Image` class before doing any work with the
     library!

- The framework functions all have different names:
    - `dip_SeparableFrameWork` -> `dip::Framework::Separable()`
    - `dip_MonadicFrameWork` -> `dip::Framework::ScanMonadic()` / `dip::Framework::SeparableMonadic()`
    - `dip_MonadicPoint` -> `dip::Framework::ScanMonadic()`
    - `dip_MonadicPointData` -> `dip::Framework::ScanMonadic()`
    - `dip_PixelTableArrayFrameWork` -> `dip::Framework::Full()`
    - `dip_PixelTableFrameWork` -> `dip::Framework::FullMonadic()`
    - `dip_ScanFrameWork` -> `dip::Framework::Scan()` / `dip::Framework::ScanDyadic()`
    - `dip_SingleOutputFrameWork` -> `dip::Framework::ScanSingleOutput()`
    - `dip_SingleOutputPoint` -> `dip::Framework::ScanSingleOutput()`

  Their interfaces are not exactly compatible, but it should be relatively straightforward
  to port old line functions to use the new framework, yielding shorter code.
  However, all line functions are now expected to use strides.

- `dip_ovl.h` is now `diplib/overload.h`, and works like a normal header file defining
  macros to use in your code. `dip_tpi.h` is gone, the template strategy to define overloaded
  functions is now based on C++ templates, yielding code that is easier to write and easier
  to read.

- There is no longer a `dip_Initialise` function. There are no global variables. There are
  no registries.

## Changes in algorithm interface

- Function parameters are now represented by strings rather than `#define` or `enum`
  constants.

- `dip_Measure` is now `dip::MeasurementTool::Measure`, with `dip::MeasurementTool` an object
  that knows about defined measurement features.

   - Measurement features are registered with a `dip::MeasurementTool` object, not in the
     global registry.

   - Measurement features have a much simpler programmer's interface, it should be easier to
     write new features now.

- `dip_ObjectToMeasurement` now takes a feature iterator rather than computing the feature;
  this might be more flexible.

## Changes in functionality

- Second order extrapolation boundary extension didn't do as advertised in the old *DIPlib*.
  Also the first order extrapolation couldn't have worked correctly with unsigned integers.
  The new implementation fits a 2nd order polynomial that reaches 0 at the end of the extended
  boundary, yielding a continuous first derivative at the boundary. A third order extrapolation
  has been added, which works similarly but yields a continuous second derivative at the boundary.
  These functions are quite noise sensitive, however, and I expect they might produce high
  frequencies along the edge.
