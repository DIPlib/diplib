%GRADIENT   Gradient vector of an image.
%   GRADIENT(A,SIGMA) returns the gradient vector of a scalar image.
%   SIGMA is the Gaussian smoothing. The returned type is an N-by-1
%   tensor image, where N is the dimensionality of A.
%   SIGMA defaults to 1.
%
%   GRADIENT(A,SIGMA,METHOD,BOUNDARY_CONDITION,PROCESS,TRUNCATION) defines
%   how the gradient is computed. See DERIVATIVE for a description of these
%   parameters and their defaults.
%
%   PROCESS determines along which dimensions to take the derivative.
%   For the N-dimensional image above, if PROCESS==1, then the output
%   is a scalar image with only the derivative along the first
%   dimension.

% (c)2017-2018, Cris Luengo.
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

function out = gradient(varargin)
out = gradientvector(varargin{:});
