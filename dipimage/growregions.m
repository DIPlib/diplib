%GROWREGIONS   Grow (dilate) labeled regions uniformly
%
% SYNOPSIS:
%  image_out = growregions(label_image,mask,connectivity,iterations)
%
% PARAMETERS:
%  label_image: a labeled image (of unsigned integer type).
%  mask: binary image to constrain the operation, or [].
%  connectivity: defines the neighborhood:
%     * 1 indicates 4-connected neighbors in 2D or 6-connected in 3D
%     * 2 indicates 8-connected neighbors in 2D
%     * 3 indicates 28-connected neighbors in 3D
%     * -1 and -2 indicate alternating values leading to a more isotropic
%       operation
%  iterations: the number of steps taken, defines the size of the
%     structuring element (0 is the same as Inf, meaning repeat until no
%     further changes occur).
%
% DEFAULTS:
%  mask = []
%  connectivity = -1
%  iterations = 0
%
% NOTE:
%  Pixels in GREY_IMAGE with a value of +INF are not processed, and will be
%  marked as watershed pixels. Use this to mask out parts of the image you
%  don't need processed. (If 'high first' is given, use -INF instead).
%
% NOTES:
%  If a MASK is given, this is the labeled equivalent to BPROPAGATION.
%  Otherwise it works as BDILATION on each label. However, the growing stops
%  as soon as two labels meet.
%
%  If isotropy in the dilation is very important, compute the distance
%  transform of the background component with DT, then apply WATERSEED.
%
% SEE ALSO:
%  growregionsweighted, waterseed, bpropagation
%
% DIPlib:
%  This function calls the DIPlib function <a href="https://diplib.org/diplib-docs/regions.html#dip-GrowRegions-Image-CL-Image-CL-Image-L-dip-sint--dip-uint-">dip::GrowRegions</a>.

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

function out = growregions(varargin)
out = dip_segmentation('growregions',varargin{:});
