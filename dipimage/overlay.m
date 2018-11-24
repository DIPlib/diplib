%OVERLAY   Overlay a grey-value or color image with a binary or label image
%
% SYNOPSIS:
%  image_out = overlay(grey,binary,color)
%
% PARAMETERS:
%  grey:   a grey-value or RGB image.
%  binary: an unsigned integer image or a binary image.
%  color:  the color to use to paint the binary image into grey. Either a
%          scalar value or a 3-element vector representing RGB values on
%          a scale of 0 to 255.
%
% DEFAULTS:
%  color = [255,0,0]
%
% NOTES:
%  If BINARY is an unsigned integer image, it is overlayed on GREY using the
%  same color scheme as the labeled image display of DIPSHOW. COLOR is ignored.
%
%  If COLOR is a scalar, and GREY is a scalar image, then the output is a scalar
%  image as well. In all other cases, the output is an RGB image.
%
% SEE ALSO:
%  colormap, lut.
%
% DIPlib:
%  This function calls the DIPlib function dip::Overlay.

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
