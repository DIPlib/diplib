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
