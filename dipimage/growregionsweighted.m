%GROWREGIONSWEIGHTED   Grow labeled regions with a speed function given by a grey-value image
%
% SYNOPSIS:
%  image_out = growregionsweighted(label_image,gray_image,mask,chamfer,distance)
%
% PARAMETERS:
%  label_image: a labeled image (of unsigned integer type).
%  gray_image:  a real-valued gray-scale image, or [].
%  mask:        binary image to constrain the operation, or [].
%  chamfer:     ignored (kept for backwards compatibility)
%  distance:    how far regions are grown.
%
% DEFAULTS:
%  mask = []
%  chamfer = 5
%  distance = Inf
%
% NOTES:
%  Computes the GDT of the background of LABEL_IMAGE, then applies
%  WATERSEED with LABEL_IMAGE as seeds. MASK limits the region growing.
%
%  If GRAY_IMAGE is [], then all pixels are assumed to have a weight
%  of 1. If both GRAY_IMAGE and MASK are [], the DT is computed.
%
%  Takes the pixel size of GRAY_IMAGE into account when computing the
%  distance regions are grown. This also allows for anisotropic
%  sampling densities. IF GRAY_IMAGE doesn't have a pixel size,
%  it will use the pixel size of LABEL_IMAGE instead.
%
% SEE ALSO:
%  growregions, gdt, dt, waterseed
%
% DIPlib:
%  This function calls the DIPlib function <a href="https://diplib.org/diplib-docs/regions.html#dip-GrowRegionsWeighted-Image-CL-Image-CL-Image-CL-Image-L-dfloat-">dip::GrowRegionsWeighted</a>.

% (c)2020-2024, Cris Luengo.
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
