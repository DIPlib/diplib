%COORD2IMAGE   Generate binary image with ones at coordinates
%
% SYNOPSIS:
%  image_out = coord2image(coordinates,sz)
%
% PARAMETERS:
%  coordinates: NxD coordinates array, with D = ndims(image_out).
%  sz:          size of the image, defaults to max(coordinates)+1.
%               If an image is given, its size is used.
%
% EXAMPLE:
%  co = round(reshape(rand(10),50,2).*255);
%  coord2image(co,[256 256])
%
% SEE ALSO:
%  dip_image.findcoord, dip_image.sub2ind

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

function out = coord2image(t,sz)
if ~isnumeric(t)
   error('COORDINATES must be numeric');
end
if ndims(t) ~= 2
   error('COORDINATES must be a NxD 2D array');
end
D = size(t,2);
N = size(t,1);
if nargin<2
   sz = max(t) + 1;
elseif isa(sz,'dip_image')
   sz = imsize(sz);
else
   sz = sz(:).';
end
if numel(sz)~=D || ~isnumeric(sz)
   error('SZ must be an array with D elements');
end
t = round(t);
out = newim(sz,'bin');
% Remove coordinates out of bounds
I = any(bsxfun(@gt,t,sz),2) | any(t<0,2);
t(I,:) = [];
out(sub2ind(out,t)) = 1;
