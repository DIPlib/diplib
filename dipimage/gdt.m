%GDT   Grey-weighted distance transform
%  Finds a path from each set pixel in IMAGE_IN to the background, such that the
%  integral of IMAGE_WEIGHT over the path is minimal. OUT is the value of this
%  integral. Optionally, paths can be constrained to a MASK image.
%
% SYNOPSIS:
%  out = gdt(image_in,image_weight,chamfer)
%  out = gdt(image_in,image_weight,mask,chamfer)
%
% PARAMETERS:
%  image_in: Binary image.
%  image_weight: Grey-value image. If [], a constant value of 1 is assumed.
%  mask:     Binary mask image, paths are constrained to the mask.
%  chamfer:  Chamfer distance of 1, 3 or 5.
%            0: Uses the fast marching algorithm, rather than chamfer metric.
%            1: Only the 6 (8) direct neighbors in a 3x3 (3x3x3) neighbourhood
%               are used.
%            3: All neighbors in in a 3x3 (3x3x3) neighbouhood are used.
%            5: A neighborhood of (5x5) or (5x5x5) is used.
%  out:      Integrated grey-value over least-resistance path from each
%            foreground pixel in IMAGE_IN to the nearest background pixel.
%
% DEFAULTS:
%  chamfer = 0
%
% NOTES:
%  When CHAMFER is set to 0, the fast marching algorithm is used. It
%  approximates Euclidean distances, but has a bias.
%
%  For other values of CHAMFER, a simpler (slighly faster) algorithm is used
%  that propagates distances from one pixel to the next. The neighborhood here
%  dictates the shape of the unit sphere: diamond (1), octagon (3) of dodecagon
%  (5). For 3 and 5, the distances are unbiased. Increasing the neighborhood
%  size improves the precision.
%
% DIPlib:
%  This function calls the DIPlib function <a href="https://diplib.org/diplib-docs/distance.html#dip-GreyWeightedDistanceTransform-Image-CL-Image-CL-Image-CL-Image-L-Metric--String-CL">dip::GreyWeightedDistanceTransform</a>

% (c)2017-2019, Cris Luengo.
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

function out = gdt(varargin)
out = dip_analysis('gdt',varargin{:});
