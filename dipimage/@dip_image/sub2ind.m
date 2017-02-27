%SUB2IND   Linear indices from coordinates
%   I = SUB2IND(A,C) returns the linear indices in the image A that
%   correspond to coordinates C. That is,
%      A(I(N)) == A(C(N,1),C(N,2))
%
%   Note that SUB2IND(SIZE(A),C) produces different results!
%
%   See also DIP_IMAGE/IND2SUB, COORD2IMAGE, DIP_IMAGE/FIND, DIP_IMAGE/FINDCOORD.

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

function I = sub2ind(in,C)
if isempty(in), error('Input image is empty'), end
sz = imsize(in);
if size(C,2)~=length(sz), error('Array C does not match image dimensionality'), end
if any(any(C>=repmat(sz,size(C,1),1),1)) || any(any(C<0,1)), error('Found coordinates outside range.'), end
if length(sz)<=1
   I = C;
else
   strides = cumprod([1,sz([2,1,3:end])]);
   strides = strides([2,1,3:end-1])';
   I = C*strides;
end
