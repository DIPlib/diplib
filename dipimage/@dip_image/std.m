%STD   Standard deviation of all pixels in an image.
%   VALUE = STD(B) returns the standard deviation of all pixels in
%   image B. It works independently on each tensor element.
%
%   VALUE = STD(B,M), with M a binary image, is the same as STD(B(M)).
%
%   VALUE = STD(B,M,DIM) performs the computation over the dimensions
%   specified in DIM. DIM can be an array with any number of
%   dimensions. M can be [].
%
%   VALUE = STD(...,'stable') uses a stable algorithm that prevents
%   catastrophic cancellation.
%   VALUE = STD(...,'directional') uses directional statistics.

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

function out = std(varargin)
out = dip_projection('std',varargin{:});
