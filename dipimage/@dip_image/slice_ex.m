%SLICE_EX   Extracts one slice from an image
%
% SYNOPSIS:
%  slice = slice_ex(in,plane,dim)
%
% PARAMETERS:
%  plane: Index along dimension DIM for the plane.
%  dim:   Dimension along which to slice. Negative dimensions start from the
%         end. Dimensions start counting at 1.
%
% NOTE:
%  The output SLICE has the same properties as IN, but has one fewer
%  dimension: it misses dimension DIM. However, as in DIP_IMAGE/RESHAPE
%  and DIP_IMAGE/SQUEEZE, if dimension 1 or 2 are removed, other dimensions
%  map in a non-obvious way. Given dimensions [1,2,3,4], if dimension 1 is
%  removed, the output dimensions will be ordered [3,2,4]. If dimension 2 is
%  removed, the output dimensions will be ordered [3,1,4]. This is because
%  of the way that DIPimage stores image data (swapping the first and second
%  dimensions).
%
% DEFAULTS:
%  dim = -1 (last dimension)
%
% EXAMPLE:
%  slice_ex(readim('chromo3d'),3)
%  slice_ex(readim('chromo3d'),0,1)
%  slice_ex(readim,3,2)
%
%  SEE ALSO:
%   dip_image/slice_in, dip_image/slice_op, im2array, array2im.

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
