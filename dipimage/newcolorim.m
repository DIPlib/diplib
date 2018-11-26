%NEWCOLORIM   Creates a color image initialized to zero
%   NEWCOLORIM, by itself, creates an image of 256 by 256 pixels of
%   colorspace RGB.
%
%   NEWCOLORIM(N) is an 1D RGB image with N pixels.
%
%   NEWCOLORIM([N,M]) is an N-by-M RGB image.
%
%   NEWCOLORIM([N,M,P,...]) is an N-by-M-by-P-by-... RGB image.
%
%   NEWCOLORIM(B) creates an image with zeros with the same sizes and color
%   space as B. If B is a grey-scale image, the result is an RGB image.
%
%   NEWCOLORIM(B,COL) creates an empty image with the colorspace COL.
%
%   NEWCOLORIM([N,M,..],COL,TYPE) sets the data type of the new image to
%   TYPE. TYPE can be any of the type parameters allowed by DIP_IMAGE. The
%   default data type is 'single'.
%
%   See DIP_IMAGE/COLORSPACE for known color spaces
%
%   SEE ALSO: newim, newtensorim, dip_image, dip_image.colorspace
%   dip_image.clone

% (c)2017, Cris Luengo.
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

function out = newcolorim(varargin)
sz = [256,256];
col = 'RGB';
dt = 'single';
psize = [];
if nargin >= 1
   sz = varargin{1};
   if isa(sz,'dip_image')
      psize = sz.PixelSize;
      if iscolor(sz)
         col = colorspace(sz);
      end
      sz = imsize(sz);
   elseif ~isnumeric(sz)
      error('Size vector must be a row vector with integer elements')
   elseif ~isvector(sz)
      % Treat n as an image
      sz = imsize(dip_image(sz));
   elseif ~isempty(sz) && any(sz)==0
      error('One of the dimensions is zero');
   end
   if nargin >= 2
      col = varargin{2};
      if nargin >= 3
         dt = varargin{3};
         if nargin >3
            error('Too many arguments in call to NEWCOLORIM')
         end
      end
   end
end
n = dip_image.numberchannels(col);
out = newtensorim(n,sz,dt);
out = colorspace(out,col);
if ~isempty(psize)
   out.PixelSize = psize;
end
