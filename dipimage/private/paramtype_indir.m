%PARAMTYPE_INDIR   Called by PARAMTYPE.

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

function varargout = paramtype_indir(command,param,varargin)

switch command
   case 'control_create'
      fig = varargin{1};   % figure handle
      bh = uicontrol(fig,...
                     'Style','pushbutton',...
                     'String','Browse...',...
                     'Visible','off',...
                     'HorizontalAlignment','center',...
                     'BusyAction','cancel',...
                     'Interruptible','off');
      eh = uicontrol(fig,...
                     'Style','edit',...
                     'String',param.default,...
                     'Visible','off',...
                     'HorizontalAlignment','left',...
                     'BackgroundColor',[1,1,1]);
      set(bh,'Callback',{@do_browse,eh,param.description});
      varargout{1} = [eh,bh];
   case 'control_value'
      varargout{2} = get(varargin{1},'String');
      varargout{1} = varargout{2};
      varargout{2} = mat2str(varargout{2});
   case 'definition_test'
      varargout{1} = '';
      if ~ischar(param.default)
         varargout{1} = 'DEFAULT must be a string for indir';
      end
end

function do_browse(~,~,editbox,title)
   %tmp = getfile(mask,title);
   %tmp = putfile(mask,title);
tmp = getdir(title);
if ~isempty(tmp)
   set(editbox,'String',tmp);
end

