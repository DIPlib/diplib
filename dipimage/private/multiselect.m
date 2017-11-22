%MULTISELECT   Multiple selection dialog box.
%   [SELECTION,OK] = LISTDLG(PROMPTSTRING,LISTSTRING,INITIALVALUE)
%   creates a modal dialog box which allows you to select multiple
%   strings from a list.
%   SELECTION is a vector of indices of the selected strings. Can be
%   [], and will be [] when OK is 0.
%   OK is 1 if you push the OK button, or 0 if you push the Cancel
%   button or close the figure.
%   Double-clicking on an item or pressing <Enter> when multiple items
%   are selected has the same effect as clicking the OK button.
%
%   Input parameter:  Description:
%   PROMPTSTRING      String which appears as text above the list box.
%   LISTSTRING        Cell array of strings for the list box OR
%                     Struct array with l.name and l.description.
%   INITIALVALUE      Vector of indices of which items of the list box
%                     are initially selected.
%
%   LISTDLG(...,1) causes the LISTSTRING to be shown as fixed-width text.

% (c)2017, Cris Luengo.
% Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
% Blatantly based on LISTDLG:
%   T. Krauss, 12/7/95, P.N. Secakusuma, 6/10/97
%   Copyright (c) 1984-98 by The MathWorks, Inc.
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

function [selection,value] = multiselect(promptstring,liststring,initialvalue,fixedwidth)

if nargin<3
   error('Invalid input.');
elseif nargin<4
   fixedwidth = 0;
end

% Create dialog box
fig = figure('name','DIPimage dialog', ...
             'units','pixels', ...
             'resize','off', ...
             'numbertitle','off', ...
             'windowstyle','modal', ...
             'KeyPressFcn',@keypress_callback,...
             'createfcn','', ...
             'closerequestfcn',@(~,~)set(gcbf,'userdata','cancel'), ...
             'color',get(0,'defaultuicontrolbackgroundcolor'), ...
             'visible','off');

% These quantities are copied from DIPIMAGE:
leftmargin = 20;          % Space between left edge and controls
internalmargin = 10;      % Horizontal space between controls
rightmargin = 20;         % Space between right edge and controls
topmargin = 10;           % Space between top edge and controls
bottommargin = 10;        % Space between bottom edge and controls
vspacing = 6;             % Space between rows
vslack = 4;               % Extra space allotted to controls (around text)
hslack = 10;              % Extra space allotted to controls (around text)
mindlgsize = [300,200];

% Calculate sizes of uicontrol elements and create liststring from struct array.
h = uicontrol(fig,'Style','text','String','Select none','Visible','off');
extent = get(h,'Extent');
controlwidth = extent(3)+hslack;
textheight = extent(4);
controlheight = textheight+vslack;
set(h,'String',promptstring);
extent = get(h,'Extent');
listwidth = extent(3);
if fixedwidth
   set(h,'FontName','FixedWidth');
end
if isstruct(liststring)
   % struct array with .name and .description - create cell array of strings
   names = {liststring.name};
   descriptions = {liststring.description};
   set(h,'String',' ');
   extent = get(h,'Extent');
   spw = extent(3);
   set(h,'String','  ');
   extent = get(h,'Extent');
   spw = extent(3) - spw; % This is the width added by a single space
   liststring = names;
   set(h,'String',names);
   extent = get(h,'Extent');
   namew = extent(3) + spw; % This is the width of the widest name
   for ii=1:length(names)
      set(h,'String',names{ii});
      extent = get(h,'Extent');
      nspaces = round((namew - extent(3))/spw);
      liststring{ii} = [names{ii},repmat(' ',1,nspaces),'- ',descriptions{ii}];
   end
else
   % cell array of strings
   % all's correct.
end
set(h,'String',liststring);
extent = get(h,'Extent');
listwidth = max(listwidth,extent(3))+20+hslack;
listheight = extent(4)+vslack;
listwidth = max(2*controlwidth+internalmargin,listwidth);
listwidth = max(listwidth,mindlgsize(1)-leftmargin-rightmargin);
dlgheight = topmargin+textheight+3*vspacing+2*controlheight+bottommargin;
listheight = max(mindlgsize(2)-dlgheight,listheight);
dlgheight = dlgheight+listheight;
dlgwidth = leftmargin+rightmargin+listwidth;
delete(h);

% Position dialog
fp = get(fig,'position');
fp = [fp(1) fp(2)+fp(4)-dlgheight dlgwidth dlgheight];  % keep upper left corner fixed
set(fig,'position',fp);

% Create controls
uicontrol(fig,'style','text','string',promptstring,...
          'horizontalalignment','left','units','pixels',...
          'position',[leftmargin,fp(4)-topmargin-textheight,...
                      listwidth,controlheight]);
uicontrol(fig,'style','pushbutton','string','OK',...
          'position',[fp(3)-rightmargin-controlwidth,bottommargin,...
                      controlwidth,controlheight],...
          'callback',@(~,~)set(gcbf,'userdata','ok'));
uicontrol(fig,'style','pushbutton','string','Cancel',...
          'position',[fp(3)-rightmargin-2*controlwidth-internalmargin,...
                      bottommargin,controlwidth,controlheight],...
          'callback',@(~,~)set(gcbf,'userdata','cancel'));
selectall_btn = uicontrol(fig,'style','pushbutton','string','Select all',...
          'position',[fp(3)-rightmargin-controlwidth,...
                      bottommargin+controlheight+vspacing,controlwidth,controlheight]);
selectnone_btn = uicontrol(fig,'style','pushbutton','string','Select none',...
          'position',[fp(3)-rightmargin-2*controlwidth,...
                      bottommargin+controlheight+vspacing,controlwidth,controlheight]);
listbox = uicontrol(fig,'style','listbox','string',liststring,...
          'backgroundcolor','w','max',2,'value',initialvalue,...
          'position',[leftmargin,bottommargin+2*controlheight+2*vspacing,...
                      listwidth,listheight]);
if fixedwidth
   set(listbox,'FontName','FixedWidth');
end
N = length(liststring);
set(selectall_btn,'callback',{@selectall_btn_callback,listbox,N,selectnone_btn});
set(selectnone_btn,'callback',{@selectnone_btn_callback,listbox,selectall_btn});
set(listbox,'callback',{@listbox_callback,selectall_btn,selectnone_btn,N});
if length(initialvalue) == N
   set(selectall_btn,'enable','off')
elseif isempty(initialvalue)
   set(selectnone_btn,'enable','off')
end

% RUN!
set(fig,'visible','on');
drawnow;
uicontrol(listbox);
waitfor(fig,'userdata')
switch get(fig,'userdata')
   case 'ok'
      value = 1;
      selection = get(listbox,'value');
   otherwise
      value = 0;
      selection = [];
end
delete(fig)

function selectall_btn_callback(selectall_btn,~,listbox,N,selectnone_btn)
set(selectall_btn,'enable','off');
set(selectnone_btn,'enable','on');
set(listbox,'value',1:N);

function selectnone_btn_callback(selectnone_btn,~,listbox,selectall_btn)
set(selectnone_btn,'enable','off');
set(selectall_btn,'enable','on');
set(listbox,'value',[]);

function listbox_callback(listbox,~,selectall_btn,selectnone_btn,N)
if length(get(listbox,'value'))==N
   set(selectall_btn,'enable','off');
else
   set(selectall_btn,'enable','on');
end
if isempty(get(listbox,'value'))
   set(selectnone_btn,'enable','off');
else
   set(selectnone_btn,'enable','on');
end
if(strcmp(get(gcbf,'SelectionType'),'open'))
   set(gcbf,'userdata','ok');
end

function keypress_callback(~,~)
ch = double(get(gcbf,'CurrentCharacter'));
if ~isempty(ch)
   switch ch
      case 13
         set(gcbf,'userdata','ok');
      case 27
         set(gcbf,'userdata','cancel');
   end
end
