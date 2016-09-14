# Changes from DIPlib 2.0 (the old DIPlib)

We used to call the image size its dimension. You had a function `dip_Dimensions`, which
is now `dip::Image::Sizes`. The reason is that it was too confusing talking about a dimension
as an image axis (the 2nd dimension), and the dimension of that dimension (the size of the
image along that axis).
