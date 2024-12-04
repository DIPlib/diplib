%APPLY_COLORMAP   Apply a color map to a gray-scale image
%
% SYNOPSIS:
%  image_out = apply_colormap(image_in,colormap)
%
% PARAMETERS:
%  image_in: a grey-scale image
%  colormap: a string, one of:
%    - 'grey':       A 1:1 mapping of intensities to the RGB value for the same intensity.
%    - 'saturation': Like 'grey', except 0 and 255 are blue and red respectively.
%    - 'linear':     A blue-magenta-yellow highly saturated, perceptually linear color map.
%    - 'diverging':  A blue-grey-yellow diverging, perceptually linear color map.
%    - 'cyclic':     A magenta-yellow-green-blue cyclic, perceptually linear color map.
%    - 'label':      Each grey value gets a color that can easily be distinguished from that
%                    of nearby grey values. Has 16 different colors, plus black for 0.
%
% NOTES:
%  IMAGE_IN is expected to be in the range [0, 255], values outside that range will be
%  clamped to the range.
%
%  IMAGE_OUT will be an image of the same size, of type uint8, with three channels and in
%  sRGB color space.
%
%  'linear', 'diverging' and 'cyclic' are from colorcet by Peter Kovesi,
%  see https://colorcet.com
%  (CET-L08, CET-D07 and CET-C2 respectively).
%
% SEE ALSO:
%  overlay, lut.
%
% DIPlib:
%  This function calls the DIPlib function <a href="https://diplib.org/diplib-docs/display.html#dip-ApplyColorMap-Image-CL-Image-L-String-CL">dip::ApplyColorMap</a>.

% (c)2023, Cris Luengo.
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

function out = overlay(varargin)
out = dip_math('apply_colormap',varargin{:});
