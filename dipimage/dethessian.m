%DETHESSIAN   Det(Hessian) operator
%
% SYNOPSIS:
%  image_out = dethessian(image_in,sigma,method,boundary_condition,dim,truncation)
%
%  IMAGE_IN is a scalar image with N dimensions.
%  IMAGE_OUT is a scalar image, computed by DET(HESSIAN(IMAGE_IN,...).
%
%  DIM determines along which dimensions to apply the operation.
%  It must be an array of integers in the range 1 to NDIMS(IMAGE_IN).
%  The empty array indicates that all dimensions should be processed.
%
%  See DERIVATIVE for a description of the parameters and the defaults.

% (c)2017-2018, Cris Luengo.
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

function image_out = dethessian(image_in,varargin)
image_out = det(hessian(image_in,varargin{:}));
