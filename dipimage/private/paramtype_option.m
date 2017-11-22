%PARAMTYPE_OPTION   Called by PARAMTYPE.

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

function varargout = paramtype_option(command,param,varargin)

switch command
   case 'control_create'
      fig = varargin{1};   % figure handle
      % Create a list of possible values and a list for display
      data = param.constraint;
      if isstruct(data)
         h = uicontrol(fig,'Style','text','String',' ','Visible','off');
         extent = get(h,'Extent');
         spw = extent(3);
         set(h,'String','  ');
         extent = get(h,'Extent');
         spw = extent(3) - spw; % This is the width added by a single space
         displist = {data.name};
         set(h,'String',displist);
         extent = get(h,'Extent');
         namew = extent(3) + spw; % This is the width of the widest name
         for ii=1:length(data)
            set(h,'String',data(ii).name);
            extent = get(h,'Extent');
            nspaces = round((namew - extent(3))/spw);
            displist{ii} = [data(ii).name,repmat(' ',1,nspaces),'- ',data(ii).description];
         end
         delete(h);
         data = {data.name};
      else
         displist = data;
      end
      % Find the index for the default value
      default = param.default;
      if (isempty(default))
         default = 1;
      else
         if ischar(default)
            default = find(strcmpi(data,default));
         else
            default = find([data{:}]==default);
            for ii=1:length(data)
               data{ii} = mat2str(data{ii});
            end
         end
      end
      h = uicontrol(fig,...
                    'Style','popupmenu',...
                    'String',displist,...
                    'Value',default,...
                    'Visible','off',...
                    'HorizontalAlignment','left',...
                    'BackgroundColor',[1,1,1]);
      varargout{1} = h;
   case 'control_value'
      indx = get(varargin{1},'Value');
      if iscell(param.constraint)
         varargout{1} = param.constraint{indx};
      else %isstruct(param.constraint)
         varargout{1} = param.constraint(indx).name;
      end
      varargout{2} = mat2str(varargout{1});
   case 'definition_test'
      msg = '';
      if isempty(param.constraint)
         msg = 'CONSTRAINT can not be empty for option';
      elseif iscell(param.constraint)
         options = param.constraint;
      elseif isstruct(param.constraint)
         if isfield(param.constraint,'name') && isfield(param.constraint,'description')
            options = {param.constraint.name};
         else
            msg = 'CONSTRAINT must contain a ''name'' and a ''description'' field';
         end
      else
         msg = 'CONSTRAINT must be a cell for option';
      end
      if isempty(msg)
         default = param.default;
         if ischar(default)
            if ~iscellstr(options)
               msg = 'DEFAULT and CONSTRAINT do not match';
            else
               N = find(strcmpi(options,default));
            end
         else
            if ~isnumeric(default)
               msg = 'options must be either strings or numerical values';
            elseif ~all(cellfun('isclass',options,'double'))
               msg = 'DEFAULT and CONSTRAINT do not match';
            else
               N = find([options{:}] == default);
            end
         end
         if isempty(msg) && isempty(N)
            msg = 'option not in list';
         end
      end
      varargout{1} = msg;
end
