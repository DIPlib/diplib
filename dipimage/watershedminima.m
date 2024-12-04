%WATERSHEDMINIMA   Detect significant local minima
%
% WATERSHEDMINIMA returns a binary image with pixels set that belong to local
% minima of the input.
%
% SYNOPSIS:
%  image_out = watershedminima(image_in,connectivity,max_depth,max_size,flag)
%
% PARAMETERS:
%  connectivity: defines the metric, that is, the shape of the structuring
%     element.
%     * 1 indicates city-block metric, or a diamond-shaped S.E in 2D.
%     * 2 indicates chessboard metric, or a square structuring element in 2D.
%     For 3D images use 1, 2 or 3.
%  max_depth, max_size: determine merging of regions.
%     A region up to 'max_size' pixels and up to 'max_depth' grey-value
%     difference will be merged. Set 'max_size' to 0 to not include size
%     in the constraint. 'max_depth'-only merging is equivalent to applying
%     the H-minima transform before the watershed.
%  flag: set to 'labels' to return a labeled output image.
%
% DEFAULTS:
%  connectivity = 1
%  max_depth = 0 (only merging within plateaus)
%  max_size = 0 (any size)
%  flag = 'binary'
%
% NOTES:
%  As opposed to the function MINIMA, local minima are defined here as the
%  lowest-valued pixels in each watershed basin, after merging the less
%  significant basins. This removes minima that are not significant.
%
%  There can be multiple disjoint groups of pixels having the same lowest
%  value within one basin. The 'labels' option will show all of these with
%  the same value.
%
%  See the user guide for the definition of connectivity in DIPimage.
%
% SEE ALSO:
%  watershedmaxima, minima, watershed
%
% DIPlib:
%  This function calls the DIPlib function <a href="https://diplib.org/diplib-docs/segmentation.html#dip-WatershedMinima-Image-CL-Image-CL-Image-L-dip-uint--dfloat--dip-uint--String-CL">dip::WatershedMinima</a>.

% (c)2020, Cris Luengo.
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

function out = watershedminima(varargin)
out = dip_morphology('watershedminima',varargin{:});
