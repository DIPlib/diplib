%PARAMTYPE_IMAGE   Called by PARAMTYPE.

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

function varargout = paramtype_image(command,param,varargin)

switch command
   case 'control_create'
      [dim,dt,at] = parse_constraint(param.constraint);
      fig = varargin{1};   % figure handle
      cm = uicontextmenu('Parent',fig);
      h = uicontrol(fig,...
                    'Style','edit',...
                    'String',param.default,...
                    'Visible','off',...
                    'HorizontalAlignment','left',...
                    'BackgroundColor',[1,1,1],...
                    'UIContextMenu',cm,...
                    'ButtonDownFcn',{@do_contextmenu,cm,fig,dt,at,dim});
      varargout{1} = h;
   case 'control_value'
      varargout{2} = get(varargin{1},'String');
      if isempty(varargout{2})
         varargout{2} = '[]';
      end
      varargout{1} = evalin('base',varargout{2});
      varargout{1} = dip_image(varargout{1});
   case 'definition_test'
      varargout{1} = '';
      if isempty(parse_constraint(param.constraint))
         varargout{1} = 'illegal CONSTRAINT value';
         return
      end
      if ~ischar(param.default)
         varargout{1} = 'DEFAULT image must be a string';
         return
      end
end

% Parse the CONSTRAINT parameter, isempty(dim) => error.
function [dim,dt,at] = parse_constraint(range)
alltypes = {'scalar','array','tensor','vector','color'};
dim = []; % error state!
dt = {'any'}; % default
at = {'tensor'}; % default
if ~isempty(range) && ~iscell(range)
   range = {range};
end
if ~isempty(range)
   range = range(:)'; % force row vector
   if isnumeric(range{1})
      dim = range{1};
      range(1) = [];
   end
   if ~isempty(range)
      if ~iscellstr(range)
         dim = []; % signals error!
         return
      end
      I = [];
      for ii=1:length(range)
         if any(strcmp(range{ii},alltypes))
            I = [I,ii];
         end
      end
      if ~isempty(I)
         at = range(I);
         range(I) = [];
      end
      if isempty(range)
         dt = {'any'}; % default
      else
         dt = range;
      end
   end
end
% PARSE DIMENSIONALITY
if isempty(dim)
   dim = [0,Inf];
elseif isequal(dim,0)
   dim = [0,Inf];
elseif numel(dim)==1
   dim = [dim,dim];
elseif numel(dim)~=2 || any(dim<0)
   dim = []; % signals error!
   return
end
dim = sort(dim(:))';
% PARSE DATA TYPE
dt = unique(dt);
% 'any' = 'complex' + 'bin'
I = find(strcmp(dt,'any'));
if ~isempty(I)
   dt(I) = [];
   dt = unique([dt,{'complex','bin'}]);
end
% 'complex' = 'scomplex' + 'dcomplex' + 'real'
I = find(strcmp(dt,'complex'));
if ~isempty(I)
   dt(I) = [];
   dt = unique([dt,{'scomplex','dcomplex','real'}]);
end
% 'noncomplex' = 'real' + 'bin'
I = find(strcmp(dt,'noncomplex'));
if ~isempty(I)
   dt(I) = [];
   dt = unique([dt,{'real','bin'}]);
end
% 'real' = 'float' + 'integer'
I = find(strcmp(dt,'real'));
if ~isempty(I)
   dt(I) = [];
   dt = unique([dt,{'float','integer'}]);
end
% 'integer' | 'int' = 'signed' + 'unsigned'
I = find( strcmp(dt,'integer') | strcmp(dt,'int') );
if ~isempty(I)
   dt(I) = [];
   dt = unique([dt,{'signed','unsigned'}]);
end
% 'float' = 'sfloat' + 'dfloat'
I = find(strcmp(dt,'float'));
if ~isempty(I)
   dt(I) = [];
   dt = unique([dt,{'sfloat','dfloat'}]);
end
% 'signed' | 'sint' = 'sint8' + 'sint16' + 'sint32' + 'sint64'
I = find( strcmp(dt,'signed') | strcmp(dt,'sint') );
if ~isempty(I)
   dt(I) = [];
   dt = unique([dt,{'sint8','sint16','sint32','sint64'}]);
end
% 'unsigned' | 'uint' = 'uint8' + 'uint16' + 'uint32' + 'uint64'
I = find( strcmp(dt,'unsigned') | strcmp(dt,'uint') );
if ~isempty(I)
   dt(I) = [];
   dt = unique([dt,{'uint8','uint16','uint32','uint64'}]);
end
% 'bin' = 'binary'
I = find(strcmp(dt,'bin'));
if ~isempty(I)
   dt(I) = {'binary'};
end

% Check image type and dimensionality against constraints
function res = check_image(name,dt,at,dim)
%#function isscalar isvector iscolor datatype ndims
if isempty(at)
   res = true;
else
   res = false;
   if any(strcmp(at,'scalar'))
      if evalin('base',['isscalar(',name,')'])
         res = true;
      end
   end
   if any(strcmp(at,'vector'))
      if evalin('base',['isvector(',name,')'])
         res = true;
      end
   end
   if any(strcmp(at,'color'))
      if evalin('base',['iscolor(',name,')'])
         res = true;
      end
   end
   if ~res
      return
   end
end
d = evalin('base',['datatype(',name,')']);
if ~any(strcmp(dt,d))
   res = false;
   return
end
d = evalin('base',['ndims(',name,')']);
if d<dim(1) || d>dim(2)
   res = false;
   return
end

% Callback function
function do_contextmenu(cbo,~,cm,fig,dt,at,dim)
%#function whos
list = evalin('base','whos');
I = strcmp({list.class},'dip_image');
if any(I)
   delete(get(cm,'Children'));
   list = {list(I).name};
   for item = list
      name = item{1}; % why is the item not extracted from the cell???
      if check_image(name,dt,at,dim)
         uimenu(cm,'Label',name,'Callback',{@contextmenu_callback,cbo,name});
      end
   end
   set(cm,'position',get(fig,'CurrentPoint')); % Not sure why this is necessary. Because of dynamic creation?
   set(cm,'Visible','on');
end

function contextmenu_callback(~,~,editbox,item)
set(editbox,'String',item);
