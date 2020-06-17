%SLICE_EX   Extracts one slice from an image.
%   SLICE_EX(IN,PLANE,DIM) returns the N-1 dimensional image obtained by
%   indexing into N-dimensional image IN along dimension DIM with index
%   PLANE. For example, if IN has 3 dimensions, and DIM is 3, it returns
%   IN(:,:,PLANE). Dimensions start counting at 1, negative dimensions
%   start from the end (that is, -1 is the last dimension).
%
%   SLICE_EX(IN,PLANE) indexes along the last dimension of IN.
%
%   The output SLICE has the same properties as IN, but has one fewer
%   dimension: it misses dimension DIM. However, as in DIP_IMAGE/SQUEEZE,
%   if dimension 1 or 2 are removed, other dimensions map in a non-obvious
%   way. Given dimensions [1,2,3,4], if dimension 1 is removed, the output
%   dimensions will be ordered [3,2,4]. If dimension 2 is removed, the
%   output dimensions will be ordered [3,1,4]. This is because of the way
%   that DIPimage stores image data (swapping the first and second
%   dimensions). Note that the 'CheapSqueeze' preference does not affect
%   how SLICE_EX works.
%
%   See also dip_image/slice_in, dip_image/slice_op, im2array, array2im.

% (c)2017-2019, Cris Luengo.
% Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
% Originally written by Bernd Rieger.
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

function out = slice_ex(in,plane,dim)

N = ndims(in);
if nargin < 3
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
out = in;
out.Data = out.Data(:,:,indx{:});
if length(out.PixelSize)>ddim % not >= ddim!
   out.PixelSize(ddim) = [];
end
if ddim < N % if it's the last dimension, the singleton is removed automatically
   sz = size(out.Data);
   sz(dim+2) = [];
   out.Data = reshape(out.Data,sz);
   % This works OK with trailing singleton dimensions
end
