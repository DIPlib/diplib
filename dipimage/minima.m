%MINIMA   Detect local minima
%
% MINIMA returns a binary image with pixels set that belong to local minima
% of the input.
%
% SYNOPSIS:
%  image_out = minima(image_in,connectivity,flag)
%
% PARAMETERS:
%  connectivity: defines which pixels are considered neighbours: up to
%     'connectivity' coordinates can differ by maximally 1. Thus:
%     * A connectivity of 1 indicates 4-connected neighbours in a 2D image
%       and 6-connected in 3D.
%     * A connectivity of 2 indicates 8-connected neighbourhood in 2D, and
%       18 in 3D.
%     * A connectivity of 3 indicates a 26-connected neighbourhood in 3D.
%     Connectivity can never be larger than the image dimensionality.
%  flag: set to 'labels' to return a labelled output image.
%
% DEFAULTS:
%  connectivity = 1
%  flag = 'binary'
%
% NOTE:
%     a = minima(in,2,'labels')
%  is equivalent to
%     a = label(minima(in,2),2)
%  except that labels are not necessarily contiguous (that is, not all values
%  between 1 and the maximum label value are used).
%
% SEE ALSO:
%  maxima
%
% DIPlib:
%  This function calls the DIPlib function dip::Minima.

% (c)2017, Cris Luengo.
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
