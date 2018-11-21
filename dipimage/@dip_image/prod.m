%PROD Product of elements.
%   VALUE = PROD(B) gets the value of the product of all pixels in an image.
%   It works independently on each tensor element.
%
%   VALUE = PROD(B,M) only computes the product of the pixels within the
%   mask specified by the binary image M, and is equivalent to PROD(B(M)).
%
%   VALUE = PROD(B,M,DIM) computes the product over the dimensions specified
%   in DIM. For example, if B is a 3D image, PROD(B,[],3) returns an image
%   with 2 dimensions, containing the product over the pixel values along
%   the third dimension (z). DIM can be an array with any number of
%   dimensions. M can be [].
%
%   VALUE = PROD(B,'tensor') works over the tensor elements, returning
%   a scalar image of the same size as B.
%
%   COMPATIBILITY NOTE:
%   The behavior of PROD(B), with B a tensor image, has changed since
%   DIPimage 2. Previously, it operated on the tensor elements, which
%   is currently accomplished with PROD(B,'tensor').

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

function out = prod(varargin)
out = dip_projection('prod',varargin{:});
