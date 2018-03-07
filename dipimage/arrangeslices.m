%ARRANGESLICES   Converts an n-D image into a 2D image by arringing the slices
%
% SYNOPSIS:
%  image_out = arrangeslices(image_in,ncolumns)
%
% PARAMETERS:
%  ncolumns:  number of columns in which the slices are arranged.
%
% DEFAULTS:
%  ncolumns = ceil(sqrt(N)), with N = prod(sz(3:end)), sz = imsize(image_in)
%
% NOTES:
%  If you want to show only part of the slices, use (for example):
%     arrangeslices(image(:,:,0:10:end))
%
%  For a 4D image you might want to set ncolums=size(image_in,3).
%
% SEE ALSO:
%   tile, detile

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

function img = arrangeslices(img,ncolumns)
% check input arguments
if nargin > 1 && ~isempty(ncolumns)
   if length(ncolumns)~=1 || ~isnumeric(ncolumns) || ncolumns < 1 || mod(ncolumns,1)
      error('NCOLUMNS must be a positive integer scalar')
   end
else
   ncolumns = [];
end
% some checks
if ~isa(img,'dip_image')
   img = dip_image(img);
end
sz = imsize(img);
if length(sz)<3
   return
end
if length(sz)>3
   sz = [sz(1:2),prod(sz(3:end))];
   img = reshape(img,sz);
end
% compute table size
if isempty(ncolumns)
   ncolumns = ceil(sqrt(sz(3)));
end
nrows = ceil(sz(3)/ncolumns);
% add zeros to image for empty table elements
if nrows*ncolumns > sz(3)
   img = cat(3,img,newim([sz(1:2),nrows*ncolumns-sz(3)],datatype(img)));
end
% two reshapes and one permute do the trick (note that only the permute
% causes copying of the data)
img = reshape(img,[sz(1)*ncolumns,sz(2),nrows]);
img = permute(img,[3,2,1]);
img = reshape(img,[sz(1)*ncolumns,sz(2)*nrows]);
