%WATERSEED   Watershed initialized with a seed image
%
% WATERSEED performs a watershed on the image GREY_IMAGE, starting with
% the seeds in the labeled SEED_IMAGE. The labeled regions are grown
% by addressing their neighbors (defined by CONNECTIVITY) in the order
% of the grey-values in GREY_IMAGE.
%
% SYNOPSIS:
%  image_out = waterseed(seed_image,grey_image,connectivity,max_depth,max_size,flags)
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
%     the H-minima transform before the watershed. A negative 'max_depth'
%     disables all merging.
%  flags: a cell array of strings, choose from:
%     - 'high first': reverse the sorting order.
%     - 'labels': output a labeled image rather than a binary image.
%     - 'no gaps': reginos are grown until they tough each other, no watershed
%       lines are kept. Makes most sense together with 'labels'.
%     - 'uphill only': regions are grown only uphill (or downhill if 'high first'
%       is also given), so that each seed only grows within its own catchment
%       basin.
%
% DEFAULTS:
%  connectivity = 1
%  max_depth = 0 (only merging within plateaus)
%  max_size = 0 (any size)
%  flags = {}
%
% NOTE:
%  Pixels in GREY_IMAGE with a value of +INF are not processed, and will be
%  marked as watershed pixels. Use this to mask out parts of the image you
%  don't need processed. (If 'high first' is given, use -INF instead).
%
% NOTE:
%  See the user guide for the definition of connectivity in DIPimage.
%
% EXAMPLE:
%  a = readim('cermet');
%  b = minima(gaussf(a,10),2,'labels');
%  c = waterseed(b,a,1);
%  overlay(a,c)
%
% SEE ALSO:
%  watershed, maxima, minima
%
% DIPlib:
%  This function calls the DIPlib function dip::SeededWatershed.

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

function out = waterseed(varargin)
out = dip_morphology('waterseed',varargin{:});
