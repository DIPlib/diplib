%GET_SUBPIXEL   Retrieves subpixel values in an image by interpolation
%
% USAGE:
%  out = get_subpixel(in, coord, method)
%
% PARAMETERS:
%  coord:  Image coordinates of interest. Can be a list of coordinates:
%          coord = [x(:) y(:)]
%  method: 'linear', '3-cubic', 'nearest'
%  out:    array containing the interpolated values
%
% DEFAULTS:
%  method = 'linear'
%
% NOTES:
%  If COORD is an MxN matrix, where M is the number of points and N is
%  equal to the image dimensionality, then OUT is an MxT matrix, where
%  T is the number of tensor elements.
%
%  For backwards compatibility, method=='spline' will be interpreted
%  as 'cubic', since 'spline' is no longer supported.
%
%  `'cubic'` is an alias for `'3-cubic'`, for backwards compatibility.
%
% DIPlib:
%  This function calls the DIPlib function dip::ResampleAt.

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

function out = get_subpixel(varargin)
out = dip_geometry('get_subpixel',varargin{:});
