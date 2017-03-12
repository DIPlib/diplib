%IFT   Fourier transform (inverse)
%
% SYNOPSIS:
%  image_out = ift(image_in)

function out = ift(in)
out = ft(in,'inverse');
