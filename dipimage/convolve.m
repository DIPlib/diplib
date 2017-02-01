%CONVOLVE   General convolution filter
%
% SYNOPSIS:
%  image_out = convolve(image_in,kernel,boundary_condition)
%
%  If KERNEL is separable, the convolution is computed in the spatial
%  domain using NDIMS(IMAGE_IN) one-dimensional convolutions. If not, the
%  convolution is computed either through the Fourier Domain or by direct
%  implementation of the convolution sum, depending on the size of the kernel.
%  The function uses the number of pixels in the kernel and the number of
%  pixels in the image to determine which method is the most efficient.
%  In all cases, the output is the same size as the input image.
%
%  (TODO: everything in the above paragraph is not yet implemented.)
%  Currently: kernel is a cell array with vectors. Each vector is applied
%  to a different dimension. There should be as many vectors as dimensions
%  in IMAGE_IN, or only one to apply it to each dimension. For example:
%
%     a = readim('cermet');
%     convolve(a,{[1,1,1],[-1,0,1]})
%
%  BOUNDARY_CONDITION is a string or a cell array of strings (one per image
%  dimension) specifying how the convolution handles pixel values outside
%  of the image domain.
%
% DEFAULTS:
%  boundary_condition = {} (equivalent to 'mirror')
%
% NOTE:
%  See the user guide for the available boundary condition strings.
%
% DIPlib:
%  This function calls the DIPlib functions dip::SeparableConvolution,
%  dip::ConvolveFT or dip::GeneralConvolution, depending on the input.
