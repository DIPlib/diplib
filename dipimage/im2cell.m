%IM2CELL   Converts a dip_image to a cell array of images
%
% SYNOPSIS:
%  A = IM2CELL(B)
%
%  Coverts tensor image B into a cell array, where each tensor element
%  becomes a cell. The cell array has the same size and shape as the
%  tensor.
%
%  For symmetric tensors, this implies that some cells of A will contain
%  the same image (but MATLAB uses copy-on-write, meaning that these
%  identical copies will share data).
%
%  For diagonal and triangular matrix images, the zero components are
%  filled with zero-valued images of the same size as the other components.
%  To prevent this, reshape the tensor to a vector first: IM2CELL(B{:})
%
% EXAMPLES:
%  a = readim('flamingo');
%  b = im2cell(a);
%  c = cell2im(b,'rgb');   % c is identical to a.
%
%  a = readim('chromo3d');
%  a = gradientvector(a);
%  a = a * a.';
%  b = im2cell(a);      % b is a 3x3 cell, with b{2,1}==b{1,2}
%  c = im2cell(a{:});   % c is a 6x1 cell
%
% SEE ALSO:
%  cell2im, im2mat, dip_image.dip_array

% (c)2018, Cris Luengo.
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

function out = im2cell(in)
if ~isa(in,'dip_image')
   error('Input image expected');
end
sz = imarsize(in);
out = cell(sz);
switch in.TensorShape
   case 'diagonal matrix'
      zero = newim(in,datatype(in));
      for ii = 1:sz(1)
         out{ii,ii} = in{ii,ii};
         for jj = 1:sz(2)
            if ii~=jj
               out{ii,jj} = zero;
            end
         end
      end
   case 'symmetric matrix'
      for ii = 1:sz(1)
         out{ii,ii} = in{ii,ii};
         for jj = ii+1:sz(2)
            out{ii,jj} = in{ii,jj};
            out{jj,ii} = out{ii,jj};
         end
      end
   case 'upper triangular matrix'
      zero = newim(in,datatype(in));
      for ii = 1:sz(1)
         for jj = 1:ii-1
            out{ii,jj} = zero;
         end
         for jj = ii:sz(2)
            out{ii,jj} = in{ii,jj};
         end
      end
   case 'lower triangular matrix'
      zero = newim(in,datatype(in));
      for ii = 1:sz(1)
         for jj = 1:ii
            out{ii,jj} = in{ii,jj};
         end
         for jj = ii+1:sz(2)
            out{ii,jj} = zero;
         end
      end
   otherwise
      for ii = 1:sz(1)
         for jj = 1:sz(2)
            out{ii,jj} = in{ii,jj};
         end
      end
end
