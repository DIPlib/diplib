%DRAWTEXT   Draws text in an image
%
% SYNOPSIS:
%  image_out = drawtext(image_in, text, font, size, origin, color, orientation, align)
%  image_out = drawtext(image_in, text, origin, color, orientation, align)
%  image_out = drawtext(text, font, size, orientation)
%  image_out = drawtext(text, orientation)
%
%  In the first two forms, adds text to an existing image. In the latter two
%  forms, returns an image tightly cropped around the rendered text, with an
%  intensity of 1 on a background of intensity 0.
%
%  If FONT is given, will use FreeType to load the font from file and render
%  the text with it. DIPlib must be built with FreeType support, otherwise an
%  error message will be given. When no FONT is given, uses the built-in font
%  to render the text.
%
%  font:        name of an TTF or OTF file, including absolute or relative path.
%  size:        size, in pixels, of the font; interpretation depends on font.
%  origin:      coordinates of the baseline of the text, see ALIGN.
%  color:       color used to render the text (one value per image channel).
%  orientation: orientation of the baseline in radian.
%  align:       horizontal alignment of the text around ORIGIN. One of:
%               'left', 'center', 'right'.
%
% DEFAULTS:
%  size = 12
%  origin = [0, size(image_in, 2)-1]
%  color = [255]
%  orientation = 0
%  align = 'left'
%
% EXAMPLES:
%  drawtext('foobar',pi/4)
%
%  a = readim('trui');
%  drawtext(a,'trui',[53,120],255,0,'center')
%
% EXAMPLES (requiring FreeType):
%  drawtext('foobar','/Library/Fonts/Montserrat-Regular.ttf',40,pi/4)
%
%  a = readim('DIP');
%  drawtext(a,'DIP','/Library/Fonts/Montserrat-Regular.ttf',40,[128,83],[255,30,100],0,'center')
%
% DIPlib:
%  This function calls the DIPlib functions <a href="https://diplib.org/diplib-docs/generation_drawing.html#dip-DrawText-Image-L-String-CL-FloatArray--Image-Pixel-CL-dfloat--String-CL">dip::DrawText</a> and
%  <a href="https://diplib.org/diplib-docs/dip-FreeTypeTool.html#dip-FreeTypeTool-DrawText-Image-L-String-CL-FloatArray--Image-Pixel-CL-dfloat--String-CL">dip::FreeTypeTool::DrawText</a>.

% (c)2022, Cris Luengo.
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

function out = drawtext(varargin)
out = dip_generation('drawtext',varargin{:});
