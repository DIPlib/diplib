%PMD   Perona-Malik anisotropic diffusion
%
% SYNOPSIS:
%  outimage = pmd(in,iterations,K,lambda,g)
%
% PARAMETERS:
%  in:         Image with equal sampling rates in all dimensions
%  iterations: The number of iterations to run
%  K:          Edge-stopping parameter (4% of the image's range is a good start)
%  lambda:     Diffusion step size (<1, smaller + more iterations = more accurate)
%  g:          Name of function used to compute diffusion value at each pixel.
%              Can be one of: 'Gauss', 'quadratic', 'exponential'.
%
% DEFAULTS:
%  iterations = 5
%  K = 10
%  lambda = 0.25
%  g = 'Gauss'
%
% LITERATURE:
%  P. Perona and J. Malik, "Scale-Space and Edge Detection Using Anisotropic
%  Diffusion", IEEE Transactions on Pattern Analysis and Machine Intelligence
%  12(7):629:639, 1990.
%
% SEE ALSO:
%  pmd_gaussian, aniso, ced
%
% DIPlib:
%  This function calls the DIPlib function dip::PeronaMalikDiffusion.

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

function out = pmd(varargin)
out = dip_filtering('pmd',varargin{:});
