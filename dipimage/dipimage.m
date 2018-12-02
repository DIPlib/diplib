%DIPIMAGE   Starts the interactive user interface DIPimage
%   DIPIMAGE, on its own, starts DIPimage, an graphical user interface
%   that gives easy access to the basic functionality of the DIPimage
%   toolbox.

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

function dipimage(arg)

figureWindowTag = 'DIPimage_Main_Window';

currsetting = get(0,'ShowHiddenHandles');
set(0,'ShowHiddenHandles','on');
difig = findobj('Tag',figureWindowTag);  % the DIPIMAGE figure handle
set(0,'ShowHiddenHandles',currsetting);

if nargin==1
   if isequal(arg,'clear')
      % Clear everything DIPimage-related from memory, useful when developing
      delete(difig)
      close all
      dipfig -unlink
      imagedisplay unlock
      dipsetpref -unload
      numberofthreads unlock
      clear functions
      return
   else
      error('Unknown argument to DIPIMAGE')
   end
end

% Do we already exist?
if isempty(difig)
   do_start;
else
   % We exist: just raise the window.
   figure(difig)
   return
end

%
% Initialize the DIPimage window
%
function do_start
% Create a new DIPimage window
fig = figure( ...
   'BusyAction','cancel', ...
   'Color',get(0,'defaultuicontrolbackgroundcolor'), ...
   'CloseRequestFcn',@(~,~)delete(gcbf), ...
   'CreateFcn','', ...
   'DeleteFcn','',...
   'HandleVisibility','off', ...
   'IntegerHandle','off', ...
   'Interruptible','off',...
   'MenuBar','none', ...
   'Name','DIPimage', ...
   'NumberTitle','off',...
   'SizeChangedFcn',@format_dialog, ...
   'Tag',figureWindowTag, ...
   'Units','pixels', ...
   'Visible','off');
setappdata(fig,'isrecording',false);
setappdata(fig,'command','');
%jFrame = get(fig,'JavaFrame');
%bgMode = jFrame.UICONTROLBACKGROUND_COMPATIBLE;
%jFrame.setUIControlBackgroundCompatibilityMode(bgMode);
% Create the menu system
try
   create_menus(fig);
catch ME
   disp(ME.message);
   delete(fig);
   error('Couldn''t generate menu system.')
end
% Position the window
currsetting = get(0,'Units');
set(0,'Units','pixels');
ss = get(0,'ScreenSize');
set(0,'Units',currsetting);
height = 200;
width = floor(ss(3)/2);
left = 10;                % guesstimate - will correct for later
bottom = ss(4)-height-60; % guesstimate - will correct for later
set(fig,'Position',[left,bottom,width,height]);
% Put some data in the window to initialise it
do_about(fig,'DIPimage')
% Now correct window position to make it fit snuggly agains the top left corner of the screen
border = dipfig_getbordersize(fig);
fp = get(fig,'Position');
fp(1) = border(1);
fp(2) = ss(4)-fp(4)-border(2)-border(4);
set(fig,'Position',fp);
set(fig,'Visible','on');
% Do user-customized initialization
%#function dipinit
if exist('dipinit','file')
   evalin('base','dipinit');
end
end

%
% Start or stop recording the macro
%
function do_macro(cbo,~)
fig = ancestor(cbo,'figure');
if getappdata(fig,'isrecording')
   % stop recording the macro
   setappdata(fig,'isrecording',false);
   setappdata(fig,'macroname','');
   set(cbo,'Label','Record macro')
else
   % start recording the macro
   [fname,pname] = uiputfile('macro.m','Record macro as');
   if isequal(fname,0) || isequal(pname,0)
      % user pressed cancel
      return;
   end
   macroname = fullfile(pname,fname);
   try
      f = fopen(macroname,'at');
      fprintf(f,'%% --- Macro recording started %s ---\n',datestr(now));
      fclose(f);
      edit(macroname);
   catch
      errordialog('Error writing to selected file.')
      return
   end
   set(cbo,'Label','Stop recording macro')
   setappdata(fig,'macroname',macroname);
   setappdata(fig,'isrecording',true);
end
end

%
% Handle the RELOAD MENUS command
%
function do_reload(cbo,~)
% Create the menu system
fig = ancestor(cbo,'figure');
try
   create_menus(fig);
catch ME
   disp(ME.message);
   delete(fig);
   error('Couldn''t generate menu system.')
end
end

%
% Handle the ABOUT command
%
function do_about(cbo,component)
if strcmp(component,'DIPimage')
   info = toolboxinformation;
   strings = {info.name,info.version,info.copyright{:},info.URL};
else
   info = libraryinformation;
   if strncmp(info.type,'Debug',5)
      type = 'Debug';
   else
      type = 'Release';
   end
   if ~isempty(strfind(info.type,'OpenMP'))
      type = [type ', with OpenMP'];
   end
   copyright = textscan(info.copyright,'%s','delimiter',char(10));
   strings = {[info.name ', ' info.description],...
              ['Version ' info.version ' (' info.date ', ' type ')'],...
              copyright{1}{:}, info.URL};
end
fig = ancestor(cbo,'figure');
create_dialog(fig,'about',['About ',component],strings)
end

%
function info = toolboxinformation
info.name = 'DIPimage Toolbox for Quantitative Image Analysis';
info.version = 'Version unknown';
info.copyright = {'(c)2016-, Cris Luengo and contributors','(c)1999-2014, Delft University of Technology'};
info.URL = 'http://www.diplib.org';
p = fileparts(mfilename('fullpath'));
f = fopen(fullfile(p,'Contents.m'),'r');
if f>0
   % This file starts with a header that we simply copy over into lines of the info box.
   ln = fgetl(f);
   info.name = ln(3:end);
   ln = fgetl(f);
   info.version = ln(3:end);
   ln = fgetl(f);
   info.copyright{1} = ln(3:end);
   ln = fgetl(f);
   info.copyright{2} = ln(3:end);
   fclose(f);
end
end

%
% Handle the HELP button and commands
%
function do_help(h,~)
fig = ancestor(h,'figure');
helpwin(getappdata(fig,'command'))
end

%
% Open a web site (handles the USER MANUAL command and more)
%
function do_website(url)
try
   web(url,'-browser');
catch
   try
      web(url);
   catch
      error(['Could not open the online help file. You can find it here:',10,url]);
   end
end
end

%
% Handle the menu commands
%
function do_menucommand(cbo,~,command)
fig = ancestor(cbo,'figure');
% Find command parameters
functionlist = getappdata(fig,'functionlist');
if functionlist.isKey(command)
   d = functionlist(command);
else
   errordialog('Command not in function list')
   return
end
if isempty(d) || ~isstruct(d)
   errordialog('No ParamList structure')
else
   msg = testdefs(d);
   if ~isempty(msg)
      errordialog({'ParamList structure incorrect:',msg})
      return
   end
   create_dialog(fig,'command',command,d);
end
end

%
% Tests the DEFS structure for correctness. Returns a string in MSG
% if there is an error. MSG is empty if its OK. The returned DEFS has
% some elements changed to avoid ambiguities (easier further parsing).
%
function msg = testdefs(defs)
msg = [];
if isfield(defs,'inparams')
   params = defs.inparams;
   if ~isfield(params,'description') || ~isfield(params,'type') || ...
      ~isfield(params,'constraint') || ~isfield(params,'default')
      msg = 'structure elements missing in INPARAMS';
      return
   end
   N = length(params);
   for ii=1:N
      if ~ischar(params(ii).description) || ~ischar(params(ii).type)
         msg = ['DESCRIPTION and TYPE must be strings (input parameter #',num2str(ii),')'];
         return
      end
      msg = paramtype('definition_test',params(ii),params);
      if ~isempty(msg)
         msg = [msg,' (input parameter #',num2str(ii),')'];
         return;
      end
   end
end
if isfield(defs,'outparams')
   params = defs.outparams;
   if ~iscellstr(params)
      msg = 'OUTPARAMS must be a cell array of strings';
      return
   end
end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                              Creating the menus                           %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function create_menus(fig)

% Start by removing all the already defined menus
delete(findobj(fig,'Tag','mainmenu'));

% read in the default menus and command ordering, and the exclude list
try
   clear dipmenus
   [menulist,functionlist] = dipmenus();
catch ME
   disp([13,13,'Warning: Call to DIPMENUS failed:',13,13,ME.message,13,13])
   menulist = {'File I/O',{'reload','exit'}}; % Bare minimum?
   functionlist = [];
end
setappdata(fig,'functionlist',functionlist);

% iterate through the MENULIST structure to get all the menus made
menulist{1,3} = [];
for ii=1:size(menulist,1)
   menulist{ii,3} = uimenu(fig,'Label',menulist{ii,1},'Tag','mainmenu');
end
% iterate through the MENULIST structure to get all the items made
for ii=1:size(menulist,1)
   h = menulist{ii,3};
   menuitems = menulist{ii,2};
   separator = 'off';
   for jj=1:length(menuitems)
      mn = menuitems{jj};
      if mn(1) == '#'
         % we put the indicated menu as a submenu here
         mn = mn(2:end);
         I = find(strcmp(mn,menulist(:,1)),1,'first');
         if ~isempty(I)
            set(menulist{I,3},'parent',h,'Separator',separator);
            separator = 'off';
         end
      else
         switch mn
            case '-'
               % add separator above next menu item
               separator = 'on';
            case 'macro'
               uimenu(h,'Label','Record macro','Callback',@do_macro,'Separator',separator);
               separator = 'off';
            case 'reload'
               uimenu(h,'Label','Reload menus','Callback',@do_reload,'Separator',separator);
               separator = 'off';
            case 'exit'
               uimenu(h,'Label','Exit','Callback',@(~,~)delete(fig),'Separator',separator);
               separator = 'off';
            otherwise
               if functionlist.isKey(mn)
                  item = functionlist(mn);
                  if isfield(item,'display') && ~isempty(item.display)
                     uimenu(h,'Label',[item.display,' (',mn,')'],'Callback',...
                           {@do_menucommand,mn},'Separator',separator);
                  else
                     uimenu(h,'Label',mn,'Callback',...
                           {@do_menucommand,mn},'Separator',separator);
                  end
                  separator = 'off';
               end
         end
      end
   end
end
% add the help menu at the end
h = uimenu(fig,'Label','Help','Separator','on','Tag','mainmenu');
uimenu(h,'Label','DIPimage User Manual','Callback',@(~,~)do_website(dipgetpref('UserManualLocation')));
uimenu(h,'Label','DIPimage Reference','Callback',@(~,~)helpwin('DIPimage'),'tag','reference');
uimenu(h,'Label','About DIPimage','Callback',@(cbo,~)do_about(cbo,'DIPimage'),'Separator','on');
uimenu(h,'Label','About DIPlib','Callback',@(cbo,~)do_about(cbo,'DIPlib'),'Separator','on');
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                           Creating the dialog box                         %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function create_dialog(fig,type,command,defs)
% Remove all controls
delete(findobj(fig,'Type','uicontrol'));
delete(findobj(fig,'Type','uicontextmenu'));
setappdata(fig,'command','');
% Avoid the dialog being executed when manipulating a pull-down menu.
set(fig,'CurrentCharacter',' ');
% Get sizes and positions
sizes.leftmargin = 20;          % Space between left edge and controls
sizes.internalmargin = 10;      % Horizontal space between controls
sizes.rightmargin = 20;         % Space between right edge and controls
sizes.topmargin = 10;           % Space between top edge and controls
sizes.bottommargin = 10;        % Space between bottom edge and controls
sizes.framemargin = 10;         % Space between frame and controls
sizes.vspacing = 6;             % Space between rows
sizes.vslack = 4;               % Extra space allotted to controls (around text)
sizes.hslack = 10;              % Extra space allotted to controls (around text)
h = uicontrol(fig,'Style','text','String','Browse...','Visible','off');
extent = get(h,'Extent'); % Size for browse button
delete(h);
sizes.mincntrlwidth = 4*(extent(3)+sizes.hslack)+sizes.internalmargin;
sizes.textheight = extent(4);
sizes.controlheight = sizes.textheight + sizes.vslack;
sizes.lineheight = sizes.controlheight + sizes.vspacing;
% There are two different types of dialogs
switch lower(type)
   case 'about'
      % This is the about dialog box
      nnames = length(defs);
      sizes.labelwidth = max(get_maximum_width(fig,defs,0),get_maximum_width(fig,defs{1},1));
      sizes.totalwidth = sizes.labelwidth;
      pos = set_dialog_size(fig,sizes,nnames+1);
      sizes.leftmargin = floor((pos(3)-sizes.labelwidth)/2);
      add_control(fig,pos,sizes,0,'title',defs{1})
      for ii=2:nnames
         add_control(fig,pos,sizes,ii-1,'text',defs{ii})
      end
      command = type;
   case 'command'
      % This is the command execution dialog box
      if isfield(defs,'display')
         title = [defs.display,' (',command,')'];
      else
         title = command;
      end
      if isfield(defs,'inparams')
         nin = length(defs.inparams);
         names = {defs.inparams.description};
      else
         nin = 0;
         names = {};
      end
      if isfield(defs,'outparams')
         nout = length(defs.outparams);
         names = [names,defs.outparams];
      else
         nout = 0;
      end
      nnames = nin+nout;
      sizes.labelwidth = get_maximum_width(fig,names,0);
      % We want the controls to be at least sizes.mincntrlwidth.
      sizes.totalwidth = sizes.labelwidth+sizes.mincntrlwidth+sizes.internalmargin;
      sizes.totalwidth = max(get_maximum_width(fig,title,1),sizes.totalwidth);
      pos = set_dialog_size(fig,sizes,nnames+3+(nin*nout>0));
      if pos(3) > (sizes.totalwidth+sizes.leftmargin+sizes.rightmargin)
         % Adjust sizes to center stuff in dialog.
         sizes.leftmargin = floor((pos(3)-sizes.totalwidth)/2);
         sizes.rightmargin = pos(3)-sizes.totalwidth-sizes.leftmargin;
      end
      add_frames(fig,pos,sizes,nin,nout)
      add_control(fig,pos,sizes,0,'title',title)
      for ii=1:nin
         add_control(fig,pos,sizes,ii,'input',defs.inparams(ii))
      end
      for ii=1:nout
         data = struct('description',defs.outparams{ii},'default','');
         if ii==1
            data.default = 'ans';
         end
         add_control(fig,pos,sizes,nin+(nin>0)+ii,'output',data)
      end
      add_buttons(fig,pos,sizes,nnames+2+(nin*nout>0))
   otherwise
      error('Internal error: unknown dialog mode.')
end
% Make all controls visible
set(findobj(fig,'Type','uicontrol'),'Visible','on');
setappdata(fig,'command',command);
setappdata(fig,'defs',defs);
setappdata(fig,'sizes',sizes);
end

%
% A subroutine to reposition the dialog box controls
%    Callback to SizeChangedFcn
%
function format_dialog(fig,~)
% Get sizes and positions
command = getappdata(fig,'command');
if ~isempty(command)
   defs = getappdata(fig,'defs');
   sizes = getappdata(fig,'sizes');
   sizes.leftmargin = 20;
   sizes.rightmargin = 20;    % these values need resetting.
   if strcmp(command,'about')
      % It is an about dialog box.
      nnames = length(defs);
      pos = set_dialog_size(fig,sizes,nnames+1);
      sizes.leftmargin = floor((pos(3)-sizes.labelwidth)/2);
      for ii=0:nnames-1
         h = findobj(fig,'Type','uicontrol','Tag',['label',num2str(ii)]);
         position_text_control(h,pos,sizes,ii)
      end
   else
      % It is a command dialog box.
      if isfield(defs,'inparams')
         nin = length(defs.inparams);
      else
         nin = 0;
      end
      if isfield(defs,'outparams')
         nout = length(defs.outparams);
      else
         nout = 0;
      end
      nnames = nin+nout;
      pos = set_dialog_size(fig,sizes,nnames+3+(nin*nout>0));
      if pos(3) > (sizes.totalwidth+sizes.leftmargin+sizes.rightmargin)
         % Adjust sizes to center stuff in dialog.
         sizes.leftmargin = floor((pos(3)-sizes.totalwidth)/2);
         sizes.rightmargin = pos(3)-sizes.totalwidth-sizes.leftmargin;
      end
      h1 = findobj(fig,'Type','uicontrol','Tag','inframe');
      h2 = findobj(fig,'Type','uicontrol','Tag','outframe');
      position_frames(h1,h2,pos,sizes,nin,nout)
      h = findobj(fig,'Type','uicontrol','Tag','label0');
      position_text_control(h,pos,sizes,0)
      for ii=1:nin
         h = findobj(fig,'Type','uicontrol','Tag',['label',num2str(ii)]);
         position_label(h,pos,sizes,ii)
         eh = findobj(fig,'Type','uicontrol','Tag',['control',num2str(ii)]);
         bh = findobj(fig,'Type','uicontrol','Tag',['control',num2str(ii),'bis']);
         if isempty(bh)
            position_control(eh,pos,sizes,ii)
         else
            position_browse_control(eh,bh,pos,sizes,ii)
         end
         h = get(eh,'UserData');
         if isfield(h,'contextmenu')
            h = h.contextmenu;
            p = get(eh,'Position');
            set(h,'Position',p(1:2));
         end
      end
      for ii=1:nout
         h = findobj(fig,'Type','uicontrol','Tag',['label',num2str(nin+(nin>0)+ii)]);
         position_label(h,pos,sizes,nin+(nin>0)+ii)
         h = findobj(fig,'Type','uicontrol','Tag',['control',num2str(nin+(nin>0)+ii)]);
         position_control(h,pos,sizes,nin+(nin>0)+ii)
      end
      he = findobj(fig,'Type','uicontrol','Tag','execute');
      hh = findobj(fig,'Type','uicontrol','Tag','help');
      position_buttons(he,hh,pos,sizes,nnames+2+(nin*nout>0));
   end
% else DON'T try to do anything if there is no data in dialog (error condition?)
end
end

%
% A subroutine to add the two frames to the input dialog.
%
function add_frames(fig,pos,sizes,nin,nout)
% fig = figure handle
% pos = figure position
% sizes = sizes of margins and controls
% nin,nout = number of lines in the two boxes.
if nin > 0
   h1 = uicontrol(fig,'Style','frame','Visible','off','Tag','inframe');
else
   h1 = [];
end
if nout > 0
   h2 = uicontrol(fig,'Style','frame','Visible','off','Tag','outframe');
else
   h2 = [];
end
position_frames(h1,h2,pos,sizes,nin,nout)
end

%
% A subroutine to add a control to the dialog box.
%
function add_control(fig,pos,sizes,index,style,data)
% fig = figure handle
% pos = figure position
% sizes = sizes of margins and controls
% index = item number in the dialog box (1 is topmost control, 0 is topmost label)
% style = {'title','text','input','output'}
% data = string for title, text or outparam, param struct for input
switch style
   case 'title' % Create a simple title control (same as text, but uses bold font.)
      labh = uicontrol(fig,...
                       'Style','text',...
                       'String',data,...
                       'Visible','off',...
                       'Tag',['label',num2str(index)],...
                       'HorizontalAlignment','center',...
                       'FontWeight','bold');
      position_text_control(labh,pos,sizes,index)
   case 'text' % Create a simple text control
      labh = uicontrol(fig,...
                       'Style','text',...
                       'String',data,...
                       'Visible','off',...
                       'Tag',['label',num2str(index)],...
                       'HorizontalAlignment','left');
      position_text_control(labh,pos,sizes,index)
      if strncmp(data,'http://',7)
         set(labh,'Enable','Inactive',...
                  'ButtonDownFcn',@(~,~)do_website(data),...
                  'ForegroundColor',[0,0,0.8]);
      end
   case 'output' % Create a variable name entering control for output variables
      labh = uicontrol(fig,...
                       'Style','text',...
                       'String',data.description,...
                       'Visible','off',...
                       'Tag',['label',num2str(index)],...
                       'HorizontalAlignment','right');
      position_label(labh,pos,sizes,index)
      h = uicontrol(fig,...
                    'Style','edit',...
                    'String',data.default,...
                    'Visible','off',...
                    'Tag',['control',num2str(index)],...
                    'HorizontalAlignment','left',...
                    'BackgroundColor',[1,1,1]);
      position_control(h,pos,sizes,index)
      set(h,'Callback',@enter_callback)
   case 'input' % Create a control for input variables through PARAMTYPE
      labh = uicontrol(fig,...
                       'Style','text',...
                       'String',data.description,...
                       'Visible','off',...
                       'Tag',['label',num2str(index)],...
                       'HorizontalAlignment','right');
      position_label(labh,pos,sizes,index)
      h = paramtype('control_create',data,fig);
      eh = h(1);
      set(eh,'Callback',@enter_callback)
      set(eh,'Tag',['control',num2str(index)]);
      if length(h)==1
         position_control(eh,pos,sizes,index)
      else
         bh = h(2);
         set(bh,'Tag',['control',num2str(index),'bis']);
         position_browse_control(eh,bh,pos,sizes,index)
      end
      h = get(eh,'UserData');
      if isfield(h,'contextmenu')
         h = h.contextmenu;
         p = get(eh,'Position');
         set(h,'Position',p(1:2));
      end
end
end

function enter_callback(cbo,~)
fig = ancestor(cbo,'figure');
if double(get(fig,'CurrentCharacter'))==13
   do_execute(fig)
end
end

%
% A subroutine to add a button to the dialog box.
%
function add_buttons(fig,pos,sizes,index)
% fig = figure handle
% pos = figure position
% sizes = sizes of margins and controls
% index = item number in the dialog box (1 is topmost control)
he = uicontrol(fig,...
               'Style','pushbutton',...
               'String','Execute',...
               'Visible','off',...
               'Tag','execute',...
               'HorizontalAlignment','center',...
               'BusyAction','cancel',...
               'Interruptible','off',...
               'Callback',@do_execute);
hh = uicontrol(fig,...
               'Style','pushbutton',...
               'String','Help',...
               'Visible','off',...
               'Tag','help',...
               'HorizontalAlignment','center',...
               'BusyAction','cancel',...
               'Interruptible','off',...
               'Callback',@do_help);
position_buttons(he,hh,pos,sizes,index);
end

%
% A subroutine to set the height of the dialog box
%
function pos = set_dialog_size(fig,sizes,nlines)
pos = get(fig,'Position');
newheight = sizes.lineheight*nlines+sizes.bottommargin+sizes.topmargin;
pos(2) = pos(2)+pos(4)-newheight;
pos(4) = newheight;
pos(3) = max(pos(3),sizes.totalwidth+sizes.leftmargin+sizes.rightmargin);
set(fig,'Position',pos);
end

%
% A subroutine to measure the width of the strings in NAMES
%
function width = get_maximum_width(fig,names,usebold)
h = uicontrol(fig,'Style','text','String',names,'Visible','off');
if usebold
   set(h,'FontWeight','bold');
end
width = get(h,'Extent');
delete(h);
width = width(3);
end

%
% A bunch of subroutines to set the position of the different types of controls
%
function position_label(h,pos,sizes,index)
if ~isempty(h)
   bottom = pos(4)-sizes.topmargin-sizes.lineheight*(index+1)+floor(sizes.vslack/2);
   set(h,'Position',[sizes.leftmargin,bottom,sizes.labelwidth,sizes.textheight]);
end
end

function position_text_control(h,pos,sizes,index)
if ~isempty(h)
   bottom = pos(4)-sizes.topmargin-sizes.lineheight*(index+1)+floor(sizes.vslack/2);
   width = get(h,'Extent');
   set(h,'Position',[sizes.leftmargin,bottom,width(3),sizes.textheight]);
end
end

function position_control(h,pos,sizes,index)
if ~isempty(h)
   bottom = pos(4)-sizes.topmargin-sizes.lineheight*(index+1);
   width = pos(3)-sizes.leftmargin-sizes.labelwidth-sizes.internalmargin-sizes.rightmargin;
   left = sizes.leftmargin+sizes.labelwidth+sizes.internalmargin;
   set(h,'Position',[left,bottom,width,sizes.controlheight]);
end
end

function position_browse_control(eh,bh,pos,sizes,index)
if ~isempty(eh) && ~isempty(bh)
   bottom = pos(4)-sizes.topmargin-sizes.lineheight*(index+1);
   width = pos(3)-sizes.leftmargin-sizes.labelwidth-sizes.internalmargin-sizes.rightmargin;
   left = sizes.leftmargin+sizes.labelwidth+sizes.internalmargin;
   w = get(bh,'Extent');
   w = w(3)+sizes.hslack;
   l = pos(3)-sizes.rightmargin-w;
   set(bh,'Position',[l,bottom,w,sizes.controlheight]);
   set(eh,'Position',[left,bottom,width-w-sizes.internalmargin,sizes.controlheight]);
end
end

function position_buttons(he,hh,pos,sizes,index)
left = pos(3)-sizes.rightmargin;
bottom = pos(4)-sizes.topmargin-sizes.lineheight*(index+1);
if ~isempty(he)
   width = get(he,'Extent');
   width = width(3)+sizes.hslack;
   left = left-width;
   set(he,'Position',[left,bottom,width,sizes.controlheight]);
   left = left-sizes.internalmargin;
end
if ~isempty(hh)
   width = get(hh,'Extent');
   width = width(3)+sizes.hslack;
   left = left-width;
   set(hh,'Position',[left,bottom,width,sizes.controlheight]);
end
end

function position_frames(h1,h2,pos,sizes,nin,nout)
left = sizes.leftmargin-sizes.framemargin;
width = sizes.totalwidth+2*sizes.framemargin;
offset = floor(sizes.controlheight/2)+sizes.vspacing;
if ~isempty(h1)
   height = (nin+1)*sizes.lineheight+1;
   bottom = pos(4)-sizes.topmargin-sizes.lineheight*(nin+1)-offset;
   set(h1,'Position',[left,bottom,width,height]);
end
if ~isempty(h2)
   height = (nout+1)*sizes.lineheight+1;
   bottom = pos(4)-sizes.topmargin-sizes.lineheight*(nout+1+nin+(nin>0))-offset;
   set(h2,'Position',[left,bottom,width,height]);
end
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                           Executing the commands                          %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


function do_execute(cbo,~)
fig = ancestor(cbo,'figure');
command = getappdata(fig,'command');
if isempty(command)
   return;
end
% Avoid the dialog being executed when manipulating a pull-down menu.
set(fig,'CurrentCharacter',' ');
defs = getappdata(fig,'defs');
if isfield(defs,'inparams')
   nin = length(defs.inparams);
else
   nin = 0;
end
if isfield(defs,'outparams')
   nout = length(defs.outparams);
else
   nout = 0;
end
if isfield(defs,'output_select')
   selectout = defs.output_select;
else
   selectout = 0;
end
nnames = nin+nout;
instrs = cell(1,nin);
inparams = cell(1,nin);
outparams = cell(1,nout);
dispout = ones(1,nout);
% Get & check all input parameters
for ii=1:nin
   control = findobj(fig,'Type','uicontrol','Tag',['control',num2str(ii)]);
   try
      [inparams{ii},instrs{ii}] = paramtype('control_value',defs.inparams(ii),control);
   catch ME
      errordialog({['Error evaluating parameter #',num2str(ii),':'],ME.message})
      return
   end
end
% Get all output parameters
if selectout
   selected = zeros(1,nout);
   for ii=1:nout
      control = findobj(fig,'Type','uicontrol','Tag',...
                ['control',num2str(nin+(nin>0)+ii)]);
      outparams{ii} = get(control,'String');
      if ~isempty(outparams{ii})
         if ~is_valid_varname(outparams{ii})
            errordialog(['Illegal name for output variable #',num2str(ii)])
            return
         else
            selected(ii) = 1;
         end
      end
   end
   if sum(selected)<1
      errordialog('Need at least one output variable.')
      return
   end
   names = defs.outparams;
   nin = nin+1;
   selected = find(selected);
   inparams{nin} = names(selected);
   instrs{nin} = cell2str(inparams{nin});
   outparams = outparams(selected);
   dispout = dispout(selected);
   nout = length(outparams);
else
   emptyname = 0;
   for ii=1:nout
      control = findobj(fig,'Type','uicontrol','Tag',...
                ['control',num2str(nin+(nin>0)+ii)]);
      outparams{ii} = get(control,'String');
      if isempty(outparams{ii})
         if ~emptyname
            emptyname = ii;
            if ii==1
               errordialog('First output variable needs a name.')
               return
            end
         end
      elseif ~is_valid_varname(outparams{ii})
         errordialog(['Illegal name for output variable #',num2str(ii)])
         return
      elseif emptyname
         errordialog('Cannot assign value into variable without assigning earlier values.')
         return
      end
   end
   if emptyname>0
      nout = emptyname-1;
   end
end
% Print to command line
if dipgetpref('PutInCommandWindow') || getappdata(fig,'isrecording')
   if nout > 1
      str = ['[',outparams{1}];
      for ii=2:nout
         str = [str,',',outparams{ii}];
      end
      str = [str,'] = ',command];
   elseif nout == 1 && ~strcmp(outparams{1},'ans')
      str = [outparams{1},' = ',command];
   else
      str = command;
   end
   if nin > 1
      str = [str,'(',instrs{1}];
      for ii=2:nin
         str = [str,',',instrs{ii}];
      end
      str = [str,')'];
   elseif nin == 1
      str = [str,'(',instrs{1},')'];
   end
   if dipgetpref('PutInCommandWindow')
      disp(str);
   end
   if getappdata(fig,'isrecording')
      try
         f = fopen(getappdata(fig,'macroname'),'at');
         fprintf(f,'%s\n',str);
         fclose(f);
      catch
         errordialog('Error writing to macro file.')
         return
      end
   end
end
% Evaluate command
try
   if nout > 0
      outvalues = cell(1,nout);
      [outvalues{:}] = feval(command,inparams{:});
   else
      feval(command,inparams{:});
   end
catch ME
   errordialog({'Error executing the command:',ME.message})
   return
end
% Assign results in base workspace
for ii=1:nout
   try
      assignin('base',outparams{ii},outvalues{ii});
      if dispout(ii)
         evalin('base',outparams{ii});
      end
   catch ME
      errordialog({['Error putting output argument # ',num2str(ii),':'],ME.message})
   end
end
end

end
