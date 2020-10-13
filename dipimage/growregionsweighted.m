%GROWREGIONSWEIGHTED   Grow labeled regions with a speed function given by a grey-value image
%
% SYNOPSIS:
%  image_out = growregionsweighted(label_image,gray_image,mask,chamfer)
%
% PARAMETERS:
%  label_image: a labeled image (of unsigned integer type).
%  gray_image: a real-valued gray-scale image.
%  mask: binary image to constrain the operation, or [].
%  chamfer:  Chamfer distance of 1, 3 or 5.
%            1: Only the 6 (8) direct neighbors in a 3x3 (3x3x3) neighbourhood
%               are used.
%            3: All neighbors in in a 3x3 (3x3x3) neighbouhood are used.
%            5: A neighborhood of (5x5) or (5x5x5) is used.
%
% DEFAULTS:
%  mask = []
%  chamfer = 5
%
% NOTE:
%  Computes the GDT of the background of LABEL_IMAGE, then applies
%  WATERSEED with LABEL_IMAGE as seeds. MASK limits the region growing.
%
% SEE ALSO:
%  growregions, gdt, waterseed
%
% DIPlib:
%  This function calls the DIPlib function dip::GrowRegionsWeighted.

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

function out = growregionsweighted(varargin)
out = dip_segmentation('growregionsweighted',varargin{:});
