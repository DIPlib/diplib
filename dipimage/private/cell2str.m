%CELL2STR Convert 2-D cell array into evaluable string.
%   B = CELL2STR(C) returns a B such that C = EVAL(B), under the
%   following contraits:
%   - C contains only numeric, logic, string or cell arrays, not
%     structs nor objects.
%   - NDIMS(C) = 2.
%
%   B = CELL2STR(C,N) uses N digits of precision for numeric
%   arrays. N defaults to 15.
%
%   B = CELL2STR(C,'class') and B = CELL2STR(C,N,'class') also
%   include the class string for the numeric arrays
%
%   See also MAT2STR, EVAL

% (c)2010-2012, Cris Luengo.
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

function str = cell2str(c,n,mode)

if nargin==2
   if isnumeric(n)
      mode = '';
   else
      mode = n;
      n = 15;
   end
elseif nargin==1
   mode = '';
   n = 15;
end

if ~isnumeric(n) || numel(n)~=1
   error('Input argument N should be a scalar.')
end
if ~ischar(mode) || ~any(strcmp(mode,{'','class'}))
   error('Mode string can only be ''class''.')
end

if iscell(c)
   N = size(c);
   if length(N)~=2
      error('Cell array must be 2-dimensional.')
   end
   str = '{';
   for ii=1:N(1)
      if ii>1
         str = [str,';'];
      end
      for jj=1:N(2)
         if jj>1
            str = [str,','];
         end
         str = [str,cell2str(c{ii,jj})];
      end
   end
   str = [str,'}'];
elseif isnumeric(c)
   if ~isempty(mode)
      str = mat2str(c,n,'class');
   else
      str = mat2str(c,n);
   end
elseif ischar(c) || islogical(c)
   str = mat2str(c);
else
   error('Illegal array in input.')
end
