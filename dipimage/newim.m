%NEWIM   Creates a scalar image initialized to zero
%   NEWIM, by itself, creates an image of 256 by 256 pixels.
%
%   NEWIM(N) is an 1D image with N pixels all set to zero.
%
%   NEWIM(N,M) or NEWIM([N,M]) is an N-by-M image.
%
%   NEWIM(N,M,P,...) or NEWIM([N,M,P,...]) is an
%   N-by-M-by-P-by-... image.
%
%   NEWIM(IMSIZE(B)) is an image with the same size as the dip_image
%   B. This does not work if B is not a dip_image.
%   In that case, do DIP_IMAGE(ZEROS(SIZE(B))).
%
%   NEWIM(B) does the same thing, but also copies over the pixel
%   size information.
%
%   NEWIM(...,TYPE) sets the data type of the new image to TYPE.
%   TYPE can be any of the type parameters allowed by DIP_IMAGE. The
%   default data type is 'single'.
%
%   NEWIM(B,DATATYPE(B)) creates an empty image with the same size
%   and data type as B.
%
%   SEE ALSO: newtensorim, newcolorim, dip_image, dip_image.clone

% (c)2017, Cris Luengo.
% (c)1999-2014, Delft University of Technology.
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

function out = newim(varargin)
dt = 'single';
n = [256,256];
N = nargin;
psize = [];
if N ~= 0
   if ischar(varargin{N})
      dt = varargin{N};
      N = N-1;
   end
   if N > 1
      for ii=1:N
         if ~isnumeric(varargin{ii}) || numel(varargin{ii}) ~= 1 || mod(varargin{ii},1)
            error('Input arguments must be scalar integers')
         end
      end
      n = cat(2,varargin{1:N});
   elseif N == 1
      n = varargin{1};
      if isa(n,'dip_image')
         psize = n.PixelSize;
         n = imsize(n);
      elseif ~isnumeric(n)
         error('Size vector must be a row vector with integer elements')
      elseif ~isvector(n)
         % Treat n as an image
         n = imsize(dip_image(n));
      elseif ~isempty(n) && any(n)==0
         error('One of the dimensions is zero');
      end
   end
end
out = dip_image(0,dt);
out = repmat(out,n);
if ~isempty(psize)
   out.PixelSize = psize;
end
