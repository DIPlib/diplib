%PARAMTYPE_HANDLE   Called by PARAMTYPE.

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

function varargout = paramtype_handle(command,param,varargin)

switch command
   case 'control_create'
      fig = varargin{1};   % figure handle
      % Get a list with figure window names
      [figh,strings] = handlelist(param.constraint);
      if isempty(figh)
         strings = {''};
         figh = 0;
      end
      cmenu = uicontextmenu('Parent',fig);
      h = uicontrol(fig,...
                    'Style','popupmenu',...
                    'String',strings,...
                    'Value',1,...
                    'Visible','off',...
                    'HorizontalAlignment','left',...
                    'BackgroundColor',[1,1,1],...
                    'UIContextMenu',cmenu,...
                    'UserData',struct('figh',figh));
      uimenu(cmenu,'Label','Reload','Callback',{@do_reloadcontrol,h,param.constraint});                         
      varargout{1} = h;
   case 'control_value'
      figh = get(varargin{1},'UserData');
      figh = figh.figh;
      indx = get(varargin{1},'Value');
      varargout{1} = figh(indx);
      varargout{2} = num2str(varargout{1});
   case 'definition_test'
      varargout{1} = '';
      if ~isempty(param.constraint) && ~ischar(param.constraint) && ~iscellstr(param.constraint)
         varargout{1} = 'CONSTRAINT must be a cell array of strings for handle';
      end
end

%
% Context menu for 'handle' control: update the handle list
%
function do_reloadcontrol(~,~,popupmenu,selection)
[figh,strings] = handlelist(selection);
if isempty(figh)
   strings = {''};
   figh = 0;
end
set(popupmenu,'String',strings,'Value',1,'UserData',struct('figh',figh));
