%FINDMINIMA   Find local minima, with sub-pixel precision
%
% FINDMINIMA returns a list of coordinates of local minima in the input
% image, with sub-pixel precision.
%
% SYNOPSIS:
%  [coords,vals] = findminima(image_in,mask,method)
%
% PARAMETERS:
%  mask: optional, limits the locations in the image where minima are found.
%  method: determines the sub-pixel location method to use, can be one of:
%     'linear', 'parabolic', 'gaussian', 'parabolic nonseparable',
%     or 'gaussian nonseparable'.
%
% OUTPUTS:
%  coords: array of size N by NDIMS(IMAGE_IN), where N is the number
%     of detected local minima, with the sub-pixel locations.
%  vals: optional output array of size N by 1, with the interpolated
%     values of IMAGE_IN at COORDS.
%
% DEFAULTS:
%  method = 'parabolic';
%
% SEE ALSO:
%  minima, findmaxima, subpixlocation
%
% DIPlib:
%  This function calls the DIPlib function dip::SubpixelMinima (but note that
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

function varargout = findminima(varargin)
varargout = cell(1,max(nargout,1));
[varargout{:}] = dip_segmentation('findminima',varargin{:});
