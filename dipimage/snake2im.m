%SNAKE2IM   Creates a binary image based on a snake
%
% SYNOPSIS:
%  image = snake2im(snake,imagesize)
%
% PARAMETERS:
%  snake :     snake, Nx2 double array
%  imagesize : size of output image (1 or 2 values)
%
% DEFAULTS:
%  imagesize = ceil(max(snake))+1
%
% SEE ALSO:
%  im2snake, snakeminimize, snakedraw

% (c)2009, 2018, Cris Luengo.
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

function o = snake2im(s,imsz)

if ~isnumeric(s) || size(s,2)~=2
   error('Input array not valid as snake.');
end
s = round(s); % drawpolygon can use float-valued inputs, but not when they're too close together
[~,indx] = unique(s,'rows');
indx = sort(indx); % indx is the set of unique points
x = s(indx,1);
y = s(indx,2);

if nargin < 2 || isempty(imsz)
   imsz = ceil(max(s))+1;
elseif ~isnumeric(imsz) || numel(imsz)>2
   error('IMAGESIZE must have 1 or 2 numbers')
else
   if isscalar(imsz)
      imsz(2) = imsz;
   end
   x(x>imsz(1)-1) = imsz(1)-1;
   y(y>imsz(2)-1) = imsz(2)-1;
end
x(x<0) = 0;
y(y<0) = 0;

o = newim(imsz,'bin');
o = drawpolygon(o,s,1,'filled');
