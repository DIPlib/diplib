%PARAMTYPE_BOOLEAN   Called by PARAMTYPE.

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

function varargout = paramtype_boolean(command,param,varargin)

switch command
   case 'control_create'
      fig = varargin{1};   % figure handle
      default = (+evalbool(param.default))+1; % 1 or 2
      h = uicontrol(fig,...
                    'Style','popupmenu',...
                    'String',{'no','yes'},...
                    'Value',default,...
                    'Visible','off',...
                    'HorizontalAlignment','left',...
                    'BackgroundColor',[1,1,1]);
      varargout{1} = h;
   case 'control_value'
      indx = get(varargin{1},'Value');
      varargout{1} = indx-1;         % first element is NO, second YES.
      varargout{2} = num2str(varargout{1});
   case 'definition_test'
      varargout{1} = '';
      try
         evalbool(param.default);
      catch
         varargout{1} = 'DEFAULT boolean should be ''yes'',''no'', 1 or 0';
      end
end

function bool = evalbool(string)
if ischar(string)
   switch lower(string)
      case {'y','yes','t','true'}
         bool = true;
      case {'n','no','f','false'}
         bool  = false;
      otherwise
         error('Boolean value expected.')
   end
elseif ( isnumeric(string) || islogical(string) ) && numel(string)==1
   if string
      bool = true;
   else
      bool = false;
   end
else
   error('Boolean value expected.')
end
