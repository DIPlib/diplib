%RICHARDSONLUCY  Richardson-Lucy iterative deconvolution
%
% This algorithm is also known as expectation maximization (EM)
%
% SYNOPSIS:
%  image_out = richardsonlucy(image_in,psf,reg,niter)
%
% PARMETERS:
%  psf: Point Spread Function
%  reg: Regularization parameter. If positive, uses total variation (TV)
%       regularization according to Dey et al. (2006).
%  niter: Number of iterations. Functions as the regularization parameter.
%
% DEFAULTS:
%  reg = 0
%  niter = 30
%
% SEE ALSO:
%  wiener, tikhonovmiller, ictm, fista
%
% DIPlib:
%  This function calls the DIPlib functions <a href="https://diplib.org/diplib-docs/deconvolution.html#dip-RichardsonLucy-Image-CL-Image-CL-Image-L-dfloat--dip-uint--StringSet-CL">dip::RichardsonLucy</a>.

% (c)2022, Cris Luengo.
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

function out = richardsonlucy(varargin)
out = dip_microscopy('richardsonlucy',varargin{:});
