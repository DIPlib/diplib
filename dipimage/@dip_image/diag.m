%DIAG    Diagonal matrices and diagonals of a matrix.
%   DIAG(V) when V is a vector image with N tensor elements is a diagonal
%   tensor image of tensor size [N,N]. Note that no data is copied, and
%   the output still has only N tensor elements.
%
%   DIAG(X) when X is a matrix is a column vector formed from the elements
%   of the main diagonal of X. In this case, data is copied.

% (c)2017-2019, Cris Luengo.
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

function a = diag(a,k)
if nargin > 1 && k ~= 0
   error('Only main diagonal access supported')
end
if ~isscalar(a)
   if isvector(a)
      a.TensorShape = 'diagonal matrix';
   else
      sz = a.TensorSizeInternal;
      if sz(1) ~= sz(2)
         error('Image is not a square tensor');
      end
      N = sz(1);
      s = substruct('()',repmat({':'},1,ndims(a.Data)));
      if strcmp(a.TensorShapeInternal,'column-major matrix') || strcmp(a.TensorShapeInternal,'row-major matrix')
         s.subs{2} = 1:N+1:N*N;
      else % diagonal, symmetric and triangular matrices store diagonal elements first
         s.subs{2} = 1:N;
      end
      if numtensorel(a) ~= N
         a.ColorSpace = '';
      end
      a.Data = subsref(a.Data,s);
      a.TensorShape = 'column vector';
   end
end
