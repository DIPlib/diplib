%PARAMTYPE_ARRAY   Called by PARAMTYPE.

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

function varargout = paramtype_array(command,param,varargin)

switch command
   case 'control_create'
      fig = varargin{1};   % figure handle
      default = param.default;
      if ~ischar(default)
         if numel(default) > 1
            default = mat2str(default);
         end
      end
      h = uicontrol(fig,...
                    'Style','edit',...
                    'String',default,...
                    'Visible','off',...
                    'HorizontalAlignment','left',...
                    'BackgroundColor',[1,1,1]);
      varargout{1} = h;
   case 'control_value'
      varargout{2} = get(varargin{1},'String');
      if isempty(varargout{2})
         varargout{2} = '[]';
      end
      varargout{1} = double(evalin('base',varargout{2}));
   case 'definition_test'
      varargout{1} = '';
      default = param.default;
      if ischar(default)
         try
            default = evalin('base',default);
         catch
            varargout{1} = 'DEFAULT array could not be evaluated';
            return
         end
      end
      if ~isnumeric(default)
         varargout{1} = 'DEFAULT is not a numeric array';
      end
end
