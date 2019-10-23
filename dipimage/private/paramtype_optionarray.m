%PARAMTYPE_OPTIONARRAY   Called by PARAMTYPE.

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

function varargout = paramtype_optionarray(command,param,varargin)

switch command
   case 'control_create'
      fig = varargin{1};   % figure handle
      default = param.default;
      if iscell(default)
         default = cell2str(default);
      end
      bh = uicontrol(fig,...
                     'Style','pushbutton',...
                     'String','Select...',...
                     'Visible','off',...
                     'HorizontalAlignment','center',...
                     'BusyAction','cancel',...
                     'Interruptible','off');
      eh = uicontrol(fig,...
                     'Style','edit',...
                     'String',default,...
                     'Visible','off',...
                     'HorizontalAlignment','left',...
                     'BackgroundColor',[1,1,1]);
      set(bh,'Callback',{@do_select,eh,param.description,param.constraint});
      varargout{1} = [eh,bh];
   case 'control_value'
      varargout{2} = get(varargin{1},'String');
      if isempty(varargout{2})
         varargout{2} = '[]';
      end
      if (iscell(param.constraint) && ~ischar(param.constraint{1})) || ...
         (isstruct(param.constraint) && ~ischar(param.constraint(1).name))
         varargout{1} = evalin('base',varargout{2});
      else
         try
            varargout{1} = evalin('base',varargout{2});
         catch
            varargout{1} = varargout{2};
            varargout{2} = mat2str(varargout{2});
         end
      end
   case 'definition_test'
      msg = '';
      if isempty(param.constraint)
         msg = 'CONSTRAINT can not be empty for option';
      elseif iscell(param.constraint)
         options = param.constraint;
      else
         if isstruct(param.constraint)
            if isfield(param.constraint,'name') && isfield(param.constraint,'description')
               options = {param.constraint.name};
            else
               msg = 'CONSTRAINT must contain a ''name'' and a ''description'' field';
            end
         else
            msg = 'CONSTRAINT must be a cell for option';
         end
      end
      if isempty(msg)
         % default value can be cell array with 0 or more elements
         if ~iscell(param.default)
            defaultcell = {param.default};
         else
            defaultcell = param.default;
         end
      end
      if isempty(msg)
         for jj = 1:length(defaultcell)
            if ischar(defaultcell{jj})
               if ~iscellstr(options)
                  msg = 'DEFAULT and CONSTRAINT do not match';
               else
                  N = find(strcmpi(options,defaultcell{jj}));
               end
            else
               if ~isnumeric(defaultcell{jj})
                  msg = 'options must be either strings or numerical values';
               elseif ~all(cellfun('isclass',options,'double'))
                  msg = 'DEFAULT and CONSTRAINT do not match';
               else
                  N = find([options{:}] == defaultcell{jj});
               end
            end
            if isempty(msg) && isempty(N)
               msg = 'option not in list';
            end
            if ~isempty(msg)
               break;
            end
         end
      end
      varargout{1} = msg;
end

function do_select(~,~,editbox,title,displist)
% Create a list of possible values and a list for display
if isstruct(displist)
   items = {displist.name};
else
   items = displist;
end
% Find currently selected items
try
   c = eval(get(editbox,'String'));
catch
   c = {};
end
if isempty(c)
   selection = [];
else
   if iscell(c)
      N = length(c);
      selection = zeros(N,1);
      for ii=1:N
         if ischar(c{ii})
            selection(ii) = find(strcmp(items,c{ii}));
         else
            selection(ii) = find(strcmp(items,mat2str(c{ii})));
         end
      end
   elseif isnumeric(c)
      selection = find([items{:}]==c);
   elseif ischar(c)
      selection = find(strcmp(items,c));
   else
      selection = [];
   end
end
% Display dialog box
[selection,button] = multiselect(title,displist,selection);
% If user pressed OK, button==1 else button==0
if button
   % Put new selection in edit box;
   set(editbox,'String',cell2str(items(selection)));
end
