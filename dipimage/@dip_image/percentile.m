%PERCENTILE   Get the percentile of an image.
%   VALUE = PERCENTILE(B,P) gets the value of the P percentile of all
%   pixels in the image B. P must be between 0 and 100. It works
%   independently on each tensor element.
%
%   Note that:
%   PERCENTILE(B,0) is the same as MIN(B)
%   PERCENTILE(B,50) is the same as MEDIAN(B)
%   PERCENTILE(B,100) is the same as MAX(B)
%
%   VALUE = PERCENTILE(B,P,M), with M a binary image, is the same as
%   PERCENTILE(B(M),P).
%
%   VALUE = PERCENTILE(B,P,M,DIM) computes the P percentile over the
%   dimensions specified in DIM. For example, if B is a 3D image,
%   PERCENTILE(B,10,[],3) returns an image with 2 dimensions, containing
%   the 10th percentile over the pixel values along the third dimension (z).
%   DIM can be an array with any number of dimensions. M can be [].
%
%   [VALUE,POSITION] = PERCENTILE(B,P,M,DIM) returns the position of the found
%   values as well. With this syntax, DIM must specify just one dimension.
%
%   VALUE = PERCENTILE(B,P,'tensor') works over the tensor elements, returning
%   a scalar image of the same size as B.
%
%   COMPATIBILITY NOTE:
%   The behavior of PERCENTILE(B,P), with B a tensor image, has changed since
%   DIPimage 2. Previously, it operated on the tensor elements, which
%   is currently accomplished with PERCENTILE(B,P,'tensor').

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

function varargout = percentile(varargin)
varargout = cell(1,max(nargout,1));
[varargout{:}] = dip_projection('percentile',varargin{:});
