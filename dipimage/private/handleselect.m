%HANDELSELECT   Handle selection dialog box.
%   HANDLE = HANDLESELECT(PROMPTSTRING,CURRFIG,SELECTION,TAG)
%   creates a modal dialog box which allows you to select one or more
%   image display handles from a list.
%
%   HANDLE is the selected display handles, and will be '' if the user
%   pressed "Cancel", or [] if the user pressed "none". So the test
%   ISCHAR(HANDLE) determines if the user pressed cancel.
%
%   PROMPTSTRING is a string which appears as the text above the
%   list box.
%
%   CURRFIG is a figure handle to the current figure handle. It won't be
%   in the list.
%
%   SELECTION is the list of handles that should initially be selected.
%   Can be [], in which case the topmost handle one will be selected.
%
%   TAG is a string or a cell array with strings passed on to HANDLELIST,
%   to define which figure windows to display.
%
%   Double-clicking on an item has the same effect as clicking the
%   OK button.

% (c)2017, Cris Luengo.
% (c)1999-2014, Delft University of Technology.
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

function handle = handleselect(promptstring,currfig,selection,tag)

if nargin < 4
   tag = '';
if nargin < 3
   selection = [];
if nargin < 2
   error('Invalid input.');
end,end,end
listbox = 1;

% Get list of defined handles
[figh,liststring] = handlelist(tag);
if ~isempty(figh)
   %remove current figure from list to show
   I = find(figh==currfig);
   if ~isempty(I)
      figh(I) = [];
      liststring(I) = [];
   end
end
if isempty(figh)
   listbox = 0;
else
   value = [];
   for ii=1:length(selection)
      I = find(figh==selection(ii));
      if ~isempty(I)
         value = [value,I];
      end
   end
   if isempty(value)
      value = 1;
   end
end

errormsg = '(no relevant figure windows found)';

% Create dialog box
fig = figure('name','DIPimage dialog', ...
             'units','pixels', ...
             'resize','off', ...
             'numbertitle','off', ...
             'KeyPressFcn',@keypress_callback,...
             'createfcn','', ...
             'closerequestfcn',@(cbo,~)set(cbo,'userdata','cancel'), ...
             'color',get(0,'defaultuicontrolbackgroundcolor'), ...
             'windowstyle','modal', ...
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

% Calculate sizes of uicontrol elements
h = uicontrol(fig,'Style','text','String','Select All','Visible','off');
extent = get(h,'Extent');
controlwidth = extent(3)+hslack;
textheight = extent(4);
controlheight = textheight+vslack;
set(h,'String',promptstring);
extent = get(h,'Extent');
listwidth = max(extent(3),3*controlwidth+2*internalmargin);
if listbox
   % Calculate size for list box
   maxlistheight = 100;
   set(h,'String',liststring);
   extent = get(h,'Extent');
   listwidth = max(listwidth,extent(3))+hslack;
   listheight = min(extent(4)+2*vslack,maxlistheight);
else
   % Calculate size for message
   set(h,'String',errormsg);
   extent = get(h,'Extent');
   listwidth = max(listwidth,extent(3));
   listheight = extent(4); % same as textheight
end
listwidth = max(2*controlwidth+internalmargin,listwidth);
listwidth = max(mindlgsize(1)-leftmargin-rightmargin,listwidth);
dlgheight = topmargin+textheight+2*vspacing+controlheight+bottommargin;
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
uicontrol(fig,'style','pushbutton','string','None',...
          'position',[fp(3)-rightmargin-3*controlwidth-2*internalmargin,...
                      bottommargin,controlwidth,controlheight],...
          'callback',@(~,~)set(gcbf,'userdata','none'));
cancelbutton = uicontrol(fig,'style','pushbutton','string','Cancel',...
          'position',[fp(3)-rightmargin-2*controlwidth-internalmargin,...
                      bottommargin,controlwidth,controlheight],...
          'callback',@(~,~)set(gcbf,'userdata','cancel'));
okbutton = uicontrol(fig,'style','pushbutton','string','OK','Enable','off',...
          'position',[fp(3)-rightmargin-controlwidth,bottommargin,...
                      controlwidth,controlheight],...
          'callback',@(~,~)set(gcbf,'userdata','ok'));
if listbox
   listbox = uicontrol(fig,'style','listbox','string',liststring,...
             'backgroundcolor','w','max',2,'value',value,...
             'position',[leftmargin,...
                         fp(4)-topmargin-textheight-vspacing-listheight,...
                         listwidth,listheight],...
             'callback',{@listbox_callback,fig});
   set(okbutton,'Enable','on');
   initialfocus = listbox;
else
   uicontrol(fig,'style','text','string',errormsg,...
             'horizontalalignment','center','units','pixels',...
             'position',[leftmargin,...
                         fp(4)-topmargin-textheight-vspacing-textheight-...
                         (listheight-textheight)/2,listwidth,textheight]);
	initialfocus = cancelbutton;
end

% RUN!
set(fig,'visible','on');
drawnow;
uicontrol(initialfocus);
waitfor(fig,'userdata')
switch get(fig,'userdata')
   case 'ok'
      handle = figh(get(listbox,'value'));
   case 'none'
      handle = [];
   otherwise % 'cancel'
      handle = '';
end
delete(fig)

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

function listbox_callback(~,~,fig)
if(strcmp(get(fig,'SelectionType'),'open'))
   set(fig,'userdata','ok')
end
