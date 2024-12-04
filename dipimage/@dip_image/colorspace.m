%COLORSPACE   Gets/sets/changes the color space.
%    IN = COLORSPACE(IN,COL), with IN a color image, changes the color
%    space of image IN to COL, converting the pixel values as required.
%    If COL is 'grey', a scalar image is returned with the luminosity (Y).
%    If COL is '', no transformation is made, the color space information
%    is simply removed.
%
%    IN = COLORSPACE(IN,COL), with IN a tensor image, sets the color
%    space of image IN to COL. IN must have the right number of tensor
%    elements expected for colorspace COL.
%
%    COL = COLORSPACE(IN) returns the name of the color space of the
%    image IN.
%
%    Converting to a color-space-less tensor image is done by specifying
%    the empty string as a color space. This action only changes the color
%    space information, and does not change any pixel values. Thus, to
%    change from one color space to another without converting the pixel
%    values themselves, change first to a color-space-less tensor image,
%    and then to the final color space.
%
%    A color space is any string recognized by the system. It is possible
%    to specify any other string as color space, but no conversion of pixel
%    values can be made, since the system doesn't know about that color
%    space. Color space names are not case sensitive. Recognized color
%    spaces are:
%
%       grey (or gray)
%       RGB
%       sRGB
%       CMY
%       CMYK
%       HSI
%       ICH
%       ISH
%       HCV
%       HSV
%       XYZ
%       Yxy
%       Lab (or L*a*b*, CIELAB)
%       Luv (or L*u*v*, "CIELUV")
%       LCH (or L*C*H*)
%       Oklab
%       Oklch
%       wavelength
%
%    See the DIPlib documentation for <a href="https://diplib.org/diplib-docs/dip-ColorSpaceManager.html">dip::ColorSpaceManager</a> for more
%    information on the color spaces.
%
%    See also: dip_image/iscolor

% (c)2017-2019, Cris Luengo.
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

function obj = colorspace(obj,col)
if nargin==1
   obj = obj.ColorSpace;
elseif isempty(col)
   obj.ColorSpace = '';
else
   obj = colorspacemanager(obj,col);
end
