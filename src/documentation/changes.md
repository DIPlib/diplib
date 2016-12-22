# Changes from DIPlib 2.0 (the old DIPlib) {#changes}

We list here some old names that have not been kept in the new DIPlib. We do not
list all the infrastructure changes and so on, those should be clear from reading
the introductory documentation.

- We used to call the image size its dimension. You had a function `dip_Dimensions`, which
  is now `dip::Image::Sizes`. The reason is that it was too confusing talking about a dimension
  as an image axis (the 2nd dimension), and the dimension of that dimension (the size of the
  image along that axis).

- `dip_Assimilate()` is now the `ReForge()` method.

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
  to port old line functions to use the new framework.

- Images now carry the physical dimension in them (referred to as pixel size). When porting
  functions, think about whether this data needs to be maintained, modified, or removed.

- The same is true for the color space. A function that changes the number of thensor
  elements must also remove the color space information.

- There is no longer a `dip_Initialise` function. There are no global variables.

- Second order extrapolation boundary extension didn't do as advertised in the old DIPlib.
  Also the first order extrapolation couldn't have worked correctly with unsigned integers.
  The new implementation fits a 2nd order polynomial that reaches 0 at the end of the extended
  boundary, yielding a continuous first derivative at the boundary. A third order extrapolation
  has been added, which works similarly but yields a continuous second derivative at the boundary.
  These functions are quite noise sensitive, however, and I expect they might produce high
  frequencies along the edge.
  TODO: should we make first order extrapolation also reach zero?

- `dip_ObjectToMeasurement()` now takes a feature iterator rather than computing the feature;
  this might be more flexible.
