%SUBPIXLOCATION   Find sub-pixel location of extrema
%
% Returns a list of coordinates of local maxima or minima in the
% input image, with sub-pixel precision.
%
% SYNOPSIS:
%  [coords,vals] = subpixlocation(image_in,intcoords,method,polarity)
%
% PARAMETERS:
%  intcoords: array of size N by NDIMS(IMAGE_IN), of N integer locations
%       around which to determine sub-pixel locations.
%  method: determines the sub-pixel location method to use, can be one of:
%     'linear', 'parabolic', 'gaussian', 'parabolic nonseparable',
%     or 'gaussian nonseparable'.
%  polarity: 'maximum' or 'minimum'.
%
% OUTPUTS:
%  coords: array of size N by NDIMS(IMAGE_IN), with sub-pixel locations
%     for the maxima in INTCOORDS.
%  vals: optional output array of size N by 1, with the interpolated
%     values of IMAGE_IN at COORDS.
%
% DEFAULTS:
%  method = 'parabolic';
%  polarity = 'maximum';
%
% EXAMPLE:
%  t = findcoord(maxima(a));
%  t = subpixlocation(a,t);
%  % Note that this is the same as:
%  t = findmaxima(a);
%
% SEE ALSO:
%  findmaxima, findminima, maxima, minima
%
% DIPlib:
%  This function calls the DIPlib function dip::SubpixelLocation (but note that
%  the names for separable/non-separable methods are different).

% (c)2010, 2017, Cris Luengo.
% Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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

function varargout = subpixlocation(varargin)
varargout = cell(1,max(nargout,1));
[varargout{:}] = dip_geometry('subpixlocation',varargin{:});
