%PARAMTYPE_MEASUREID   Called by PARAMTYPE.

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

function varargout = paramtype_measureid(command,param,varargin)

switch command
   case 'control_create'
      fig = varargin{1};   % figure handle
      h = uicontrol(fig,...
                    'Style','popupmenu',...
                    'String',{''},...
                    'Value',1,...
                    'Visible','off',...
                    'HorizontalAlignment','left',...
                    'BackgroundColor',[1,1,1]);
      mh = findobj(fig,'Type','uicontrol','Tag',['control',num2str(param.constraint)]);
      set(mh,'Callback',{@do_reloadcontrol,mh,h});
      do_reloadcontrol(mh,[],mh,h);
      varargout{1} = h;
   case 'control_value'
      options = get(varargin{1},'String');
      indx = get(varargin{1},'Value');
      varargout{1} = options{indx};
      varargout{2} = mat2str(varargout{1});
   case 'definition_test'
      varargout{1} = [];
      params = varargin{1};
      if numel(param.constraint) ~= 1 || ~isnumeric(param.constraint) || mod(param.constraint,1)
         varargout{1} = 'CONSTRAINT should be a scalar integer';
      elseif param.constraint >= length(params) || param.constraint < 1
         varargout{1} = 'CONSTRAINT points to a non-existent parameter';
      elseif ~strcmpi(params(param.constraint).type,'measurement')
         varargout{1} = 'CONSTRAINT should point to a measurement parameter';
      end
end

function do_reloadcontrol(~,~,cbo,popupmenu)
% Sometimes the callback object is a menu in the measurement data edit box
%if strcmp(get(cbo,'type'),'uimenu')
%   h = get(get(cbo,'Parent'),'UserData');
%end
meas = get(cbo,'String');
try
   meas = evalin('base',meas);
catch
end
if isa(meas,'dip_measurement')
   strings = fieldnames(meas);
else
   strings = {''};
end
set(popupmenu,'String',strings,'Value',1);
