%PARAMTYPE_DATASET   Called by PARAMTYPE.

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

function varargout = paramtype_dataset(command,param,varargin)

switch command
   case 'control_create'
      fig = varargin{1};   % figure handle
      cm = uicontextmenu('Parent',fig);
      h = uicontrol(fig,...
                    'Style','edit',...
                    'String',param.default,...
                    'Visible','off',...
                    'HorizontalAlignment','left',...
                    'BackgroundColor',[1,1,1],...
                    'UIContextMenu',cm,...
                    'ButtonDownFcn',{@do_contextmenu,cm,fig});
      varargout{1} = h;
   case 'control_value'
      varargout{2} = get(varargin{1},'String');
      if isempty(varargout{2})
         varargout{2} = '[]';
      end
      varargout{1} = evalin('base',varargout{2});
      if ~isa(varargout{1},'dataset')
         varargout{1} = dataset(varargout{1});
      end
   case 'definition_test'
      varargout{1} = '';
      if ~ischar(param.default)
         varargout{1} = 'DEFAULT dataset must be a string';
      end
end

% Callback function
function do_contextmenu(cbo,~,cm,fig)
%#function whos
list = evalin('base','whos');
I = find(strcmp({list.class},'dataset'));
list = {list(unique(I)).name};
if ~any(cellfun('isempty',list))
   delete(get(cm,'Children'));
   for ii=1:length(list)
      uimenu(cm,'Label',list{ii},'Callback',{@contextmenu_callback,cbo,list{ii}});
   end
   set(cm,'position',get(fig,'CurrentPoint')); % Not sure why this is necessary. Because of dynamic creation?
   set(cm,'Visible','on');
end

function contextmenu_callback(~,~,editbox,item)
set(editbox,'String',item);
