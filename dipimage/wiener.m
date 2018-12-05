%WIENER  Wiener deconvolution filter
%
% SYNOPSIS:
%  image_out = wiener(image_in,psf,S,N)
%  image_out = wiener(image_in,psf,reg)
%
% ALTERNATIVE SYNOPSIS FOR BACKWARDS-COMPATABILITY:
%  image_out = wiener(image_in,psf,-1,N,S)
%  (note the order of the parameters S and N is different)
%
% PARMETERS:
%  psf: Point Spread Function
%  S:   Signal power spectrum (set to [] for the default)
%  N:   Noise power spectrum
%  reg: Regularization parameter
%
% DEFAULTS:
%  reg = 1e-4
%  S = ft(autocorrelation(image_in))
%
% DESCRIPTION:
%  The filter is defined in the frequency domain as
%     W = H'S/(H'HS+N),
%  where H = ft(psf), and H' is the complex conjugate of H.
%
%  Alternatively, S and N can be replaced with a scalar value K equivalent to
%  the ratio N/S:
%     W = H*/(H*H+K),
%  where K is reg*max(H*H).
%
% EXAMPLE:
%  a = readim('blurr1.tif')    % an out-of-focus image
%  psf = rr([15,15])<6;
%  psf = psf/sum(psf);         % a guess of the PSF used
%  S = 1/(rr(a)^2);            % expected frequency spectrum of a natural image
%  S(128,128) = 1;             % remove NaN
%  S = S^2;                    % corresponding power spectrum
%  wiener(a,psf,S,1.5e-8)      % experimentally determined noise power, constant
%  wiener(a,psf,1.5e-2)        % experimentally determined regularization parameter
%
% SEE ALSO:
%  tikhonovmiller, mappg
%
% DIPlib:
%  This function calls the DIPlib functions dip::WienerDeconvolution.

% (c)2018, Cris Luengo.
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

function out = wiener(varargin)
out = dip_microscopy('wiener',varargin{:});
