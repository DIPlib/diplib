%WINDOW   Multiplies the image with a windowing function
%
% SYNOPSIS:
%  image_out = window(image_in,type,parameter)
%
% PARAMETERS:
%  type:      'Hamming', 'Gaussian', 'Tukey', or 'GaussianTukey'.
%  parameter: parameter to the windowing function, see below.
%
% DEFAULT:
%  type:      'Hamming'
%  parameter: 0.5
%
% WINDOWING FUNCTIONS:
%  - Hamming: A cosine window. Set PARAMETER to 0.5 to get a Hann window,
%    and to 0.53836 to get a Hamming window.
%
%  - Gaussian: A Gaussian window, this is the only one that is isotropic.
%    PARAMETER is the sigma, as a function of the image half-width. Choose a
%    value smaller or equal to 0.5. At 0.5, 4 sigmas fit in the image width.
%
%  - Tukey: A rectangular window convolved with a Hann window. PARAMETER is
%    the fraction of image width occupied by the cosine lobe. If PARAMETER is
%    1.0, it is a Hann window, if it is 0.0 it is a rectangular window.
%
%  - GaussianTukey: A rectangular window convolved with a Gaussian window.
%    PARAMETER is the sigma in pixels, a value of the order of 10 is a good
%    choice. The rectangular window is of the size of the image minus 3 sigma
%    on each edge.
%    This is the only window where the tapering is independent of the image
%    width, and thus equal along each image dimension even if the image is not
%    square. If the image size along one dimension is too small to accomodate
%    the window shape, a Gaussian window is created instead.
%
% NOTES:
%  The window is applied to each dimension independently, meaning that the
%  multi-dimensional window is the outer product of the 1D windows.
%
% DIPlib:
%  This function calls the DIPlib function dip::ApplyWindow.

% (c)2018, Cris Luengo.
%
% Licensed under the Apache License, Version 2.0 (the "License");
% you may not use this file except in compliance with the License.
% You may obtain a copy of the License at
%
%    http://www.apache.org/licenses/LICENSE-2.0
%
% Unless required by applicable law or agreed to in writing, software
% distributed under the License is distributed on an "AS IS" BASIS,
% WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
% See the License for the specific language governing permissions and
% limitations under the License.

function out = window(varargin)
out = dip_generation('window',varargin{:});
