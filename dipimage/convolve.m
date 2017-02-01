%CONVOLVE   General convolution filter
%
% SYNOPSIS:
%  image_out = convolve(image_in,kernel)
%
%  (TODO) If KERNEL is separable, the convolution is computed in the spatial
%  domain using NDIMS(IMAGE_IN) one-dimensional convolutions. If not,
%  the convolution is computed through the Fourier Domain. In either
%  case, the output is the same size as the input image
%
%  Currently: kernel is a cell array with vectors. Each vector is applied
%  to a different dimension. There should be as many vectors as dimensions
%  in IMAGE_IN, or only one to apply it to each dimension. For example:
%
%     a = readim('cermet');
%     convolve(a,{[1,1,1],[-1,0,1]})
