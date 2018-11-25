%NOISESTD   Estimate noise standard deviation
%
% SYNOPSIS:
%  snoise = noisestd(in,mask)
%
% Assumption: White Gaussian noise (since use neighbour differences)
% Limitation: may fail if noise correlated (e.g. result of filtering)
%             and will fail under complex texture.
%
% PARAMETERS:
%  mask = binary regions over which noise is computed ([] means all)
%
% DEFAULTS:
%  mask = []
%
% EXAMPLE:
%  a = readim
%  b = noise(a,'gaussian',20)
%  c = hybridf(b)             % better than Lee80
%  d = dip_image(wiener2(double(b),[3 3],noisestd(b)^2))    % Lee80
%  [noisestd(a) noisestd(b) noisestd(c) noisestd(d)]
%
% DIPlib:
%  This function calls the DIPlib function dip::EstimateNoiseVariance.
%
% LITERATURE:
%  John Immerkaer. "Fast Noise Variance Estimation",
%  Computer Vision and Image Understanding, Vol. 64, No. 2, p. 300-302, 1996.

% (c)2017, Cris Luengo.
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

function out = noisestd(varargin)
out = dip_math('noisestd',varargin{:});
