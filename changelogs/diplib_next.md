---
layout: post
title: "Changes DIPlib 3.x.x"
---

## Changes to *DIPlib*

### New functionality

- Added `dip::Kernel::IgnoreZeros()`, which causes subsequent calls to `dip::Kernel::PixelTable()` for that
  object to also exclude zero values in the gray-scale kernel image.

- Added `dip::ExtendImageToSize()`, which is like `dip::ExtendImage()` but allows instead specifying the
  size of the output image, like in `dip::Image::Pad()`.

- Added a static function `dip::Image::CropWindow()`, similar to the existing non-static one, which takes
  the image size as input.

- Added functions `dip::MeanRelativeError()` and `dip::MaximumRelativeError()`.

- Added `dip::Option::CompareImagesMode::APPROX_REL` as an option to `dip::testing::CompareImages()`, to
  compare pixel values with a relative tolerance.

- The function `dip::ConvolveFT()` has a new parameter `boundaryCondition`, if used the input image will be
  extended before computing the FFT, and the result cropped after the inverse FFT.

- Added `dip::Convolution()`, a gateway to the three implementations of the convolution.

- `dip::GeneralConvolution()` accepts a complex-valued filter kernel.

- `dip::Kernel`, `dip::PixelTable` and `dip::PixelTableOffsets` can now represent complex-valued weights. 
  Added `dip::Kernel::HasComplexWeights()`, `dip::PixelTable::WeightsAreComplex()` and
  `dip::PixelTableOffsets::WeightsAreComplex()`.

- Added `dip::HaarWaveletTransform()`.

- Added `dip::Shrinkage()`.

- Added overload for `dip::IncoherentOTF()` that takes output image sizes rather than an image as input.

### Changed functionality

- `dip::testing::Timer::CpuResolution()` and `WallResolution()` are static members.

- Stream output operator for `dip::testing::Timer` decouples the two values, choosing appropriate units
  for them independently.

- Added some more `[[nodiscard]]` annotations.

- Updated included *libics*, which fixes some potential bugs, and adds support for metadata related
  to multi-detector microscopes.

- `dip::BinaryPropagation()` is now much faster when `iterations` is 0, using an algorithm similar to the
  one used for `dip::MorphologicalReconstruction()`, but specialized for the binary case.
  `dip::MorphologicalReconstruction()` now calls `dip::BinaryPropagation()` for binary images.

- `dip::MedianFilter()`, `dip::PercentileFilter()` and `dip::RankFilter()` now use an algorithm with
  reduced complexity for larger kernels, which significantly reduces the computational time. For a kernel
  of size *k*x*k*, the naive algorithm is O(*k*^2^ log(*k*)), whereas the new one is O(*k* log(*k*)).
  This algorithm is applicable to kernels of any size and any number of dimensions.

### Bug fixes

- `dip::ResampleAt(in, map)` didn't copy the color space information from the input image to the output image.

- `dip::GeneralConvolution()` skips zero pixels in the kernel image, as was described in the documentation.
  This makes the operation significantly faster if the kernel has many zero pixels.

- `dip::ExpandTensor()` didn't write to an existing data segment in its output image, even if it was allocated
  to the right dimensions.

- `dip::BoundaryCondition::ASYMMETRIC_MIRROR` didn't work in some functions because it had the same value as
  `dip::BoundaryCondition::ALREADY_EXPANDED`.

- `dip::Histogram::Configuration`, when given an upper and lower bounds and a bin size, computed the number of
  bins in such a way that, for small difference between the bounds, the bin size was not respected.

- `dip::PixelTable::Mirror()` mirrored the footprint of the kernel, but not the weights.

- `dip::SeparateFilter()` yielded the complex conjugate of the correct 1D filter for all dimensions except the first.




## Changes to *DIPimage*

### New functionality

### Changed functionality

- Added a new parameter to `convolve`, which allows the selection of the computation method. The default, automatic
  selection of the method is now handled by *DIPlib* though the `dip::Convolution()` function. When computing
  through the Fourier domain, the periodic boundary condition is no longer the default; the operation should now
  produce the same result (up to rounding error) no matter which computation method is chosen.

(See also changes to *DIPlib*.)

### Bug fixes

(See also bugfixes to *DIPlib*.)




## Changes to *PyDIP*

### New functionality

- Added bindings for `dip::testing::Timer`.

- Added functions `dip.MeanRelativeError()` and `dip.MaximumRelativeError()`.

- Added `dip.Convolution()`, a gateway to the three implementations of the convolution.

- Added `dip.HaarWaveletTransform()`.

- Added `dip.Shrinkage()`.

### Changed functionality

- `dip.IncoherentOTF()` now takes output image sizes as first argument, instead of taking the output image. It
  now returns the output image.

- `dip.IncoherentPSF()` no longer takes the output image as an input argument, instead returning a new image of
  appropriate sizes.

(See also changes to *DIPlib*.)

### Bug fixes

(See also bugfixes to *DIPlib*.)




## Changes to *DIPviewer*

### New functionality

### Changed functionality

### Bug fixes




## Changes to *DIPjavaio*

### New functionality

### Changed functionality

### Bug fixes
