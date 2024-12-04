%ANISO   Robust anisotropic diffusion using Tukey error norm
%
% SYNOPSIS:
%  out = aniso(in,sigma,iterations,lambda)
%
% PARAMETERS:
%  in:     Image with equal sampling rates in all dimensions
%  sigma:  Scale parameter to Tukey's biweight
%  iterations: The number of iterations to run
%  lambda: Diffusion step size (<1, smaller + more iterations = more accurate)
%
% DEFAULTS:
%  sigma = 20
%  iterations = 10
%  lambda = 0.25
%
% LITERATURE:
%  Black et al., "Robust Anisotropic Diffusion", IEEE TIP 7:421-432, 1998.
%
% SEE ALSO:
%  pmd, pmd_gaussian, ced
%
% DIPlib:
%  This function calls the DIPlib function <a href="https://diplib.org/diplib-docs/nonlinear.html#dip-RobustAnisotropicDiffusion-Image-CL-Image-L-dip-uint--dfloat--dfloat-">dip::RobustAnisotropicDiffusion</a>.

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

function out = aniso(in,sigma,iterations,lambda)
if nargin<4
   lambda = 0.25;
end
if nargin<3
   iterations = 10;
end
if nargin<2
   sigma = 20;
end
out = pmd(in,iterations,sigma,lambda,'Tukey');
