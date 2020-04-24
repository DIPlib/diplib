%SLICE_IN   Inserts one slice in an image.
%   SLICE_IN(IN,SLICE,PLANE,DIM) assigns the N-1 dimensional image SLICE
%   into the N-dimensional image IN by indexing along dimension DIM with
%   index PLANE. For example, if IN has 3 dimensions, and DIM is 3, it
%   assigns IN(:,:,PLANE) = SLICE. Dimensions start counting at 1,
%   negative dimensions start from the end (that is, -1 is the last
%   dimension).
%
%   SLICE_IN(IN,SLICE,PLANE) indexes along the last dimension of IN.
%
%   The input SLICE should have the same properties (tensor elements
%   etc.) as IN, but with one fewer dimension. Because of the way data
%   is stored in DIPimage, if DIM is 1 or 2 the dimensions will be re-
%   ordered in a non-obvious way. The best way to find out what the
%   correct size is for SLICE is to extract a slice using SLICE_EX.
%
%   SLICE_EX, with the same value for the parameter DIM, gives an image
%   of correct size for SLICE_IN. See the example below.
%
%   Example:
%      a = readim('chromo3d');
%      b = gaussf(slice_ex(a,8));
%      a = slice_in(a,b,8);
%
%   See also dip_image/slice_ex, dip_image/slice_op, im2array, array2im.

% (c)2010, 2017-2019, Cris Luengo.
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

function in = slice_in(in,slice,plane,dim)

N = ndims(in);
if nargin < 4
   dim = N;
else
   if dim<0
      dim = N+dim+1;
   end
   if dim<1 || dim>N
      error('Dimension out of bounds');
   end
end
if plane<0 || plane>=imsize(in,dim)
   error('Plane out of bounds');
end

ddim = dim;
if dim==1
   dim = 2;
elseif dim==2
   dim = 1;
end
indx = repmat({':'},1,N);
indx{dim} = plane + 1;
in.Data(:,:,indx{:}) = slice.Data; % Will throw if wrong slice sizes
