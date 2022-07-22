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

- `dip::WienerDeconvolution()` has a new option `"pad"` to reduce the border effects of the frequency-domain filtering.

- Added deconvolution functions `dip::TikhonovMiller()`, `dip::IterativeConstrainedTikhonovMiller()`,
  `dip::RichardsonLucy()`, and `dip::FastIterativeSoftThresholding()`.

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

- The `dip::DFT` class now uses either *FFTW* or *PocketFFT*, depending on compile-time configuration.
  *PocketFFT* is much faster than our previous code for non-nice transform sizes. All Fourier transforms in
  *DIPlib* are now based on `dip::DFT`. Previously, `dip::FourierTransform()` would use either `dip::DFT`
  with our internal FFT algorithm (modified from *OpenCV*), or *FFTW*. The switch between the two libraries
  is now fully contained within the `dip::DFT` class, meaning that other functions that use the FFT also benefit
  from linking against *FFTW*.
    - `dip::DFT::Apply()` no longer requires a buffer. If passing in the buffer, a deprecation warning is issued by
      the compiler.
    - `dip::DFT::BufferSize()` is deprecated, and now always returns 0.
    - `dip::DFT::Initialize()`, as well as the constructor, has a new argument `inplace`, which defaults to `false`,
      and needs to be set correctly when using *FFTW*. When configured for inplace operation, the input and output
      buffers given to `Apply()` must be the same.
    - Added `dip::DFT::IsInplace()`.
    - `dip::DFT` now caches plans, it is cheap to instantiate a new object for the same transform size.

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

- Added the following deconvolution functions: `tikhonovmiller`, `ictm`, `richardsonlucy`, `fista`.

- `wiener` now uses padding to reduce the edge effects.

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

- Added deconvolution functions `dip.TikhonovMiller()`, `dip.IterativeConstrainedTikhonovMiller()`,
  `dip.RichardsonLucy()`, and `dip.FastIterativeSoftThresholding()`.

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
