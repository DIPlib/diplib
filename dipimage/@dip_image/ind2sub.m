%IND2SUB   Coordinates from linear indices.
%   C = IND2SUB(A,I) returns the coordinates in the image A that
%   correspond to linear indices I. That is,
%      A(C(N,1),C(N,2)) == A(I(N))
%
%   Note that IND2SUB(SIZE(A),I) produces different results!
%
%   See also DIP_IMAGE/SUB2IND, COORD2IMAGE, DIP_IMAGE/FIND, DIP_IMAGE/FINDCOORD.

% (c)2010, 2017, Cris Luengo.
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

function C = ind2sub(in,I)
if isempty(in), error('Input image is empty'), end
sz = imsize(in);
I = I(:);
m = prod(sz)-1;
if any(I>m) || any(I<0), error('Found indices outside range.'), end
if length(sz)<=1
   C = I;
else
   C = zeros(length(I),length(sz));
   k = [1,cumprod(sz([2,1,3:end-1]))];
   for ii=length(sz):-1:2
      C(:,ii) = floor(I/k(ii));
      I = I-(C(:,ii)*k(ii));
   end
   C(:,1) = C(:,2); % Switch the X and Y coordinates!
   C(:,2) = I;
end
