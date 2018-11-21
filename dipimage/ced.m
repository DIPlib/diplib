%CED   Coherence enhancing (anisotropic) diffusion
%
% SYNOPSIS:
%  out = ced(in,sg,st,iterations,coef,flavor,resample)
%
% PARAMETERS:
%  in:         Image with equal sampling rates in all dimensions
%  sg:         Sigma for gradient in structure tensor
%  st:         Sigma for regularization in structure tensor
%  iterations: The number of iterations to run
%  coef:       Determines whether the diffusion coefficient is considered
%              a constant or not in the computation of the divergence. One
%              of two strings: 'const' or 'variable'. 'variable' reduces
%              the computational cost a little bit.
%  flavor:     String, either 'all' or 'first'. 'all' is valid for images
%              of any dimensionality. 'first' is only available for 2D
%              images, and produces better results than 'all'.
%  resample:   A boolean ('yes' or 'no'): If 'yes', OUT is resampled to the
%              same size as IN. Otherwise, OUT will be twice as large in
%              each dimension, this is the size at which the computation is
%              performed.
%
% DEFAULTS:
%  sg = 1
%  st = 3
%  iterations = 5
%  coef = 'variable'
%  flavor = 'first'
%  resample = 'yes'
%
% LITERATURE:
%  J. Weickert, Anisotropic diffusion in image processing, Teubner (Stuttgart), 1998.
%  page 95 and 127
%
% SEE ALSO:
%  pmd, pmd_gaussian, aniso
%
% DIPlib:
%  This function calls the DIPlib function dip::CoherenceEnhancingDiffusion.

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

function out = ced(varargin)
out = dip_filtering('ced',varargin{:});
