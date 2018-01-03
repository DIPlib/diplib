%CUT   Cuts/crops an image symmetrically around the center
%
% SYNOPSIS:
%  out = cut(in,newsize,location)
%
% PARAMETERS:
%  in:       Input image
%  newsize:  New image size. Must have NDIMS(IN) elements.
%  location: The location of the crop regionwithin the input image.
%            Can be one of the following strings: 'symmetric',
%            'top left', 'top right', 'bottom left', 'bottom right'.
%            Can also be a numeric array with coordinates for the top-left
%            pixel of OUT in IN. The empty array is equivalent to
%            'symmetric'.
%
% DEFAULTS:
%  location: 'symmetric'
%
% EXAMPLE:
%  a = readim;
%  b = extend(a,[500,500]);
%  c = cut(b,imsize(a));
%  a-c
%
% NOTE:
%  'top right' and 'bottom left' are only defined for 2D.
%
%  'top left' is the same as [0,0,0,...], and 'bottom right' is the same as
%  IMSIZE(IN)-NEWSIZE.
%
% SEE ALSO:
%  extend, extendregion

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

function out = cut(in,newsz,location)

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
newsz = min(sz,newsz);
if all(newsz==sz)
   out = in;
   return
end

% Find origin for new image within old image
if ischar(location)
   switch location
      case 'symmetric'
         start = (sz-newsz)./2;  % start coord
         I = mod(sz,2)~=0;       % odd size input
         start(I) = floor(start(I));
         start(~I) = ceil(start(~I));
      case 'top left'
         start = zeros(1,N);
      case 'bottom left'
         if N>2
            error('bottomleft option only up to 2D')
         end
         start = [0,sz(2)-newsz(2)];
      case 'top right'
         if N>2
            error('topright option only up to 2D')
         end
         start = [sz(1)-newsz(1),0];
      case 'bottom right'
         start = sz - newsz;
      otherwise
         error('Illegal LOCATION value')
   end
else
   if ~isnumeric(location) || ~isvector(location) || length(location)~=N
      error('Illegal LOCATION value')
   end
   start = round(location(:)');
end

s = cell(1,N);
for ii = 1:N
   s{ii} = start(ii)+(0:newsz(ii)-1);
   if s{ii}(end) >= sz(ii)
      error('Selected region out of bounds')
   end
end
out = in(s{:});


%%% TEST CODE:
%{
x = deltaim([4,4,5,5]);
y = extend(x,[5,6,7,8]);
if any(y~=deltaim([5,6,7,8])), disp('Error in extend!!!'), end
z = cut(y,[4,4,5,5]);
if any(z~=x), disp('Error in cut!!!'), end
%}
