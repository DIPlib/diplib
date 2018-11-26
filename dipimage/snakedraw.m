%SNAKEDRAW   Draws a snake over an image
%
% SYNOPSIS:
%  handle_out = snakedraw(snake,handle_in)
%
% PARAMETERS:
%  snake :      snake, Nx2 double array
%  handle_in :  handle of image figure, or handle of line to replace
%
% OUTPUT:
%  handle_out : handle of line created, use as input to move the snake
%
% DEFAULTS:
%  handle = gcf
%
% EXAMPLE:
%  See the example in SNAKEMINIMIZE
%
% SEE ALSO:
%  snakeminimize, snake2im

% (c)2009, 2018, Cris Luengo.
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

function oh = snakedraw(s,h)

if ~isnumeric(s) || size(s,2)~=2
   error('Input array not valid as snake.');
end
x = s(:,1);
y = s(:,2);

if nargin<2
   h = gcf;
end
if ~ishandle(h)
   error('Figure does not exist!')
end
if strcmp(get(h,'type'),'line')
   lh = h;
   set(lh,'xdata',[x;x(1)],'ydata',[y;y(1)]);
else
   if ~strcmp(get(h,'type'),'image')
      h = findobj(h,'type','image');
      if length(h)~=1
         error('Cannot find an image in the figure.')
      end
   end
   h = get(h,'parent'); % axes handle
   os = get(h,'nextplot');
   set(h,'nextplot','add');
   lh = line([x;x(1)],[y;y(1)],'color',[0,0.8,0],'parent',h,'linewidth',1);
   set(h,'nextplot',os);
end
drawnow
if nargout>0
   oh = lh;
end
