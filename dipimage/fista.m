%FISTA  Fast Iterative Shrinkage-Thresholding deconvolution
%
% SYNOPSIS:
%  image_out = fista(image_in,psf,reg,tol,maxiter,nscales)
%
% PARMETERS:
%  psf: Point Spread Function
%  reg: Regularization parameter
%  tol: Tolerance for determining if the algorithm has converged
%  maxiter: Maximum number of iterations to apply
%  nscales: Number of scales used in the Haar wavelet
%
% DEFAULTS:
%  reg = 0.1
%  tol = 1e-6
%  maxiter = 30
%  nscales = 3
%
% SEE ALSO:
%  wiener, tikhonovmiller, richardsonlucy, ictm
%
% DIPlib:
%  This function calls the DIPlib functions <a href="https://diplib.org/diplib-docs/deconvolution.html#dip-FastIterativeShrinkageThresholding-Image-CL-Image-CL-Image-L-dfloat--dfloat--dip-uint--dip-uint--StringSet-CL">dip::FastIterativeShrinkageThresholding</a>.

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

function out = fista(varargin)
out = dip_microscopy('fista',varargin{:});
