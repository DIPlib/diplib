%TIKHONOVMILLER  Tikhonov-Miller deconvolution filter
%
% SYNOPSIS:
%  image_out = tikhonovmiller(image_in,psf,reg)
%
% PARMETERS:
%  psf: Point Spread Function
%  reg: Regularization parameter
%
% DEFAULTS:
%  reg = 0.1
%
% DESCRIPTION:
%  The filter is defined in the frequency domain as
%     W = H' / (H'H + reg C'C),
%  where H = ft(psf), and H' is the complex conjugate of H.
%
% EXAMPLE:
%  a = readim('blurr1.tif')    % an out-of-focus image
%  psf = rr([15,15])<6;
%  psf = psf/sum(psf);         % a guess of the PSF used
%  tikhonovmiller(a,psf,0.5)   % experimentally determined regularization parameter
%
% SEE ALSO:
%  wiener, richardsonlucy, ictm, fista
%
% DIPlib:
%  This function calls the DIPlib functions <a href="https://diplib.org/diplib-docs/deconvolution.html#dip-TikhonovMiller-Image-CL-Image-CL-Image-L-dfloat--StringSet-CL">dip::TikhonovMiller</a>.

% (c)2022, Cris Luengo.
% Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
% Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
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

function out = tikhonovmiller(varargin)
out = dip_microscopy('tikhonovmiller',varargin{:});
