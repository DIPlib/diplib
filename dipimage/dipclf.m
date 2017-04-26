%DIPCLF   Clears figure windows created by DIPSHOW
%   DIPCLF clears all figure windows created by DIPSHOW, without
%   changing their position or size.
%
%   DIPCLF(H) clears the figure window with handle H. If H is zero,
%   clears all figure windows. H can be a numeric array [H1,H2,H3]
%   or a cell array {H1,H2,'varname1','varname2'} or just a variable
%   name. The window associated with that name is cleared.

% (c)2017, Cris Luengo.
% (c)1999-2014, Delft University of Technology.
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

function dipclf(h)

if nargin < 1 || isequal(h,0)
   h = findobj('Type','figure');
else
   if isempty(h)
      return;
   end
   if ~iscell(h)
      if isnumeric(h) || ishandle(h)
         h = num2cell(h);
      elseif ischar(h)
         h = {h};
      else
         error('Array with figure handles expected')
      end
   end
   I = false(size(h));
   for ii=1:numel(I)
      if ischar(h{ii})
         h{ii} = dipfig('-get',h{ii});
      end
      if isfigh(h{ii})
         I(ii) = 1;
      end
   end
   h = [h{I}];
end
h = h(strncmp(get(h,'Tag'),'DIP_Image',9)); % They should be created using DIPshow.
for ii=1:length(h)
   dipshow(h(ii),[],'name','');
end
