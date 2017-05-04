%WATERSEED   Watershed initialized with a seed image
%
% WATERSEED performs a watershed on the image GREY_IMAGE, starting with
% the seeds in the labelled SEED_IMAGE. The labelled regions are grown
% by addressing their neighbors (defined by CONNECTIVITY) in the order
% of the grey-values in GREY_IMAGE.
%
% SYNOPSIS:
%  image_out = waterseed(seed_image,grey_image,connectivity,max_depth,max_size,flags)
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
%  max_depth, max_size: determine merging of regions.
%     A region up to 'max_size' pixels and up to 'max_depth' grey-value
%     difference will be merged. Set 'max_size' to 0 to not include size
%     in the constraint. 'max_depth'-only merging is equivalent to applying
%     the H-minima transform before the watershed.
%  flags: a cell array of strings, choose from:
%     - 'high first': reverse the sorting order.
%     - 'labels': output a labelled image rather than a binary image.
%
% DEFAULTS:
%  connectivity = 1
%  max_depth = 0 (only merging within plateaus)
%  max_size = 0 (any size)
%  flags = {}
%
% NOTE 1:
%  Two seeds will always be merged if there is no "grey-value barrier"
%  between them. Simply adding a little bit of noise to the image will
%  avoid merging of seeds.
%
% NOTE 2:
%  Pixels in GREY_IMAGE with a value of +INF are not processed, and will be
%  marked as watershed pixels. Use this to mask out parts of the image you
%  don't need processed.
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
%  This function calls the DIPlib functions dip::SeededWatershed.

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
