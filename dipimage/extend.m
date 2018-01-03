%EXTEND   Extends/pads an image with values
%
% SYNOPSIS:
%  out = extend(in,newsize,location,value,clip)
%
% PARAMETERS:
%  in:       Input image
%  newsize:  New image size. Must have NDIMS(IN) elements.
%  location: The location of the old image within the extended image.
%            Can be one of the following strings: 'symmetric',
%            'top left', 'top right', 'bottom left', 'bottom right'.
%            Can also be a numeric array with coordinates for the top-left
%            pixel of IN in OUT. The empty array is equivalent to
%            'symmetric'.
%  value:    Value with which to pad the image. Can also be a string
%            or a cell array with strings specifying one of the
%            boundary extension algorithms, see BOUNDARY_CONDITION.
%  clip:     Clip images if newsize < oldsize ('no','yes')
%
% DEFAULTS:
%  location: 'symmetric'
%  value: 0
%  clip: 'no' (if newsize smaller than input, do nothing)
%
% EXAMPLE:
%  extend(readim,[300,300])
%  extend(readim,[300,300],'symmetric','periodic')
%  extend(readim('chromo3d'),[100,100,50]'symmetric',30,1)
%
% NOTE:
%  'top right' and 'bottom left' are only defined for 2D.
%
%  'top left' is the same as [0,0,0,...], and 'bottom right' is the same as
%  NEWSIZE-IMSIZE(IN).
%
% SEE ALSO:
%  extendregion, cut

% (c)2017-2018, Cris Luengo.
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

function out = extend(in,newsz,location,value,clip)

borderextension = 'value';

if nargin < 5
   clip = false;
elseif ischar(clip)
	clip = any(strcmp(clip,{'yes','y'}));
end
if nargin < 4
   value = 0;
elseif ischar(value) || iscellstr(value)
   % Optional string as 'value' argument
   borderextension = value;
   value = 0;
elseif ~isnumeric(value) || ( numel(value)~=numtensorel(in) && numel(value)~=1 )
   error('VALUE must be a numeric array with one value per tensor element')
end
if nargin < 3
   location = 'symmetric';
elseif ischar(location)
   % Aliases for 'location' string (for backwards compatability):
   switch location
      case ''
         location = 'symmetric';
      case {'leftup','topleft'}
         location = 'top left';
      case {'rightup','topright'}
         location = 'top right';
      case {'leftlow','leftdown','bottomleft'}
         location = 'bottom left';
      case {'rightlow','rightdown','bottomright'}
         location = 'bottom right';
   end
end

sz = imsize(in);
N = length(sz);
if ~isnumeric(newsz) || ~isvector(newsz)
   error('NEWSIZE must be a numeric vector')
end
if isscalar(newsz)
   newsz = repmat(newsz,1,N);
elseif length(newsz)~=N
   error('NEWSIZE has wrong number of elements')
end
newsz = newsz(:)';
if ~clip
   newsz = max(sz,newsz);
end

% Find origin for old image within new image
if ischar(location)
   switch location
      case 'symmetric'
         start = (newsz-sz)./2;  % start coord
         I = mod(sz,2)~=0;       % odd size input
         start(I) = ceil(start(I));
         start(~I) = floor(start(~I));
      case 'top left'
         start = zeros(1,N);
      case 'bottom left'
         if N>2
            error('bottomleft option only up to 2D')
         end
         start = [0,newsz(2)-sz(2)];
      case 'top right'
         if N>2
            error('topright option only up to 2D')
         end
         start = [newsz(1)-sz(1),0];
      case 'bottom right'
         start = newsz - sz;
      otherwise
         error('Illegal LOCATION value')
   end
else
   if ~isnumeric(location) || ~isvector(location) || length(location)~=N
      error('Illegal LOCATION value')
   end
   start = round(location(:)');
end

% First crop dimensions that we want smaller
k = sz>newsz;
if any(k)
   tmp = min(sz,newsz);
   orig = -start;
   orig(~k) = 0;
   in = cut(in,tmp,orig);
   sz = imsize(in);
   start(k) = 0;
end
if all(newsz<=sz)
   out = in;
   return
end

% Next extend the other dimensions
out = clone(in,'imsize',newsz);
if any(value~=0)
   out(:) = dip_image(value(:),numel(value));
end

% Copy input into output
s = cell(1,N);
for ii = 1:N
   s{ii} = start(ii)+(0:sz(ii)-1);
   if s{ii}(end) >= newsz(ii)
      error('Selected region out of bounds')
   end
end
out(s{:}) = in;

% Boundary extension
if ~isequal(borderextension,'value')
   out = extendregion(out,start,sz,borderextension);
end
