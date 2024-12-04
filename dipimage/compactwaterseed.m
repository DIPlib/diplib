%COMPACTWATERSEED   Compact watershed initialized with a seed image
%
% COMPACTWATERSEED performs a watershed on the image GREY_IMAGE, starting with
% the seeds in the labeled SEED_IMAGE. The labeled regions are grown
% by addressing their neighbors (defined by CONNECTIVITY) in the order
% of the grey-values in GREY_IMAGE as well as the distance to the seed.
%
% SYNOPSIS:
%  image_out = compactwaterseed(seed_image,grey_image,connectivity,compactness,flags)
%
% PARAMETERS:
%  connectivity: defines the metric, that is, the shape of the structuring
%     element.
%     * 1 indicates city-block metric, or a diamond-shaped S.E in 2D.
%     * 2 indicates chessboard metric, or a square structuring element in 2D.
%     For 3D images use 1, 2 or 3.
%  compactness: determines compactness of output regions.
%     Pixels are addressed in the order given by
%     GREY_VALUE + COMPACTNESS * DISTANCE, where DISTANCE is the distance to
%     the seed the current pixel was reached from. Compactness of 0 leads to
%     a normal seeded watershed, and a very large compactness leads to a
%     voronoi diagram.
%  flags: a cell array of strings, choose from:
%     - 'high first': reverse the sorting order.
%     - 'labels': output a labeled image rather than a binary image.
%     - 'no gaps': regions are grown until they touch each other, no watershed
%       lines are kept. Makes most sense together with 'labels'.
%
% DEFAULTS:
%  connectivity = 1
%  compactness = 1
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
% SEE ALSO:
%  waterseed, watershed
%
% DIPlib:
%  This function calls the DIPlib function <a href="https://diplib.org/diplib-docs/segmentation.html#dip-CompactWatershed-Image-CL-Image-CL-Image-CL-Image-L-dip-uint--dfloat--StringSet-CL">dip::CompactWatershed</a>.

% (c)2019, Cris Luengo.
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

function out = compactwaterseed(varargin)
out = dip_morphology('compactwaterseed',varargin{:});
