%DIPTEST   Interactive pixel testing
%   DIPTEST ON turns on interactive testing of the current image.
%   DIPTEST OFF turns it off.
%   DIPTEST by itself toggles the state.
%
%   When DIPTEST is turned on, the current coordinates and pixel value are
%   displayed while a mouse button is depressed. You can move the mouse
%   while holding down the mouse button to find some coordinates. The right
%   mouse button can be used to measure lengths.
%
%   DIPTEST only works on figure windows created through DIPSHOW.
%
%   Also: DIPTEST(H,'ON'), DIPTEST(H,'OFF'), etc. to specify a window handle.
%
%   See also DIPSHOW, DIPORIEN, DIPZOOM, DIPSTEP, DIPLINK.

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

function diptest(arg1,arg2)

if nargin == 0
   fig = get(0,'CurrentFigure');
   if isempty(fig)
      error('No figure window open to do operation on.')
   end
   action = 'toggle';
elseif nargin == 1
   if ischar(arg1)
      fig = get(0,'CurrentFigure');
      if isempty(fig)
         error('No figure window open to do operation on.')
      end
      action = lower(arg1);
   else
      try
         fig = getfigh(arg1);
      catch
         error('Argument must be a valid figure handle.')
      end
      action = 'toggle';
   end
else
   try
      fig = getfigh(arg1);
   catch
      error('Argument must be a valid figure handle.')
   end
   action = lower(arg2);
end
if (length(fig) == 1) && ishandle(fig)
   tag = get(fig,'Tag');
   if strncmp(tag,'DIP_Image',9)
      OneD = tag(11)=='1';
      udata = get(fig,'UserData');
      switch (action)
      case 'toggle'
         if strcmp(udata.state,'diptest')
            dipfig_clear_state(fig,udata);
         else
            makeDIPtestObj(fig,udata,OneD);
         end
      case 'on'
         if ~strcmp(udata.state,'diptest')
            makeDIPtestObj(fig,udata,OneD);
         end
      case 'off'
         if strcmp(udata.state,'diptest')
            dipfig_clear_state(fig,udata);
         end
      end
   end
end


%
% Enable the diptest tool.
%
function makeDIPtestObj(fig,udata,OneD)
udata.state = 'diptest';
if OneD
   set(fig,'WindowButtonDownFcn',@diptestButtonDown1Fcn);
else
   set(fig,'WindowButtonDownFcn',@diptestButtonDownFcn);
end
set(fig,'WindowButtonUpFcn','',...
        'WindowButtonMotionFcn','',...
        'ButtonDownFcn','',...
        'UserData',[]);   % Solve MATLAB bug!
set(fig,'UserData',udata);
dipfig_setpointer(fig,'cross');
dipfig_set_action_check(fig,udata.state);


%
% Callback function for mouse down in images
%
function diptestButtonDownFcn(fig,~)
if strncmp(get(fig,'Tag'),'DIP_Image',9)
   img = get(fig,'CurrentObject');
   if strcmp(get(img,'Type'),'image')
      udata = get(fig,'UserData');
      udata.ax = get(img,'Parent');
      udata.oldAxesUnits = get(udata.ax,'Units');
      udata.oldNumberTitle = get(fig,'NumberTitle');
      set(udata.ax,'Units','pixels');
      if strcmp(get(fig,'SelectionType'),'alt')
         % Right mouse button
         udata.coords = dipfig_getcurpos(udata.ax); % Always over image!
         if useshg2
            udata.lineh = line([udata.coords(1),udata.coords(1)],...
                [udata.coords(2),udata.coords(2)],'Color',[0,0,0.8]);
         else
            udata.lineh = line([udata.coords(1),udata.coords(1)],...
                [udata.coords(2),udata.coords(2)],'EraseMode','xor','Color',[0,0,0.8]);
         end
         set(fig,'WindowButtonMotionFcn',@diptestMotionRFcn,...
                 'WindowButtonUpFcn',@diptestButtonUpFcn,...
                 'NumberTitle','off','UserData',[]);   % Solve MATLAB bug!
         set(fig,'UserData',udata);
         updateDisplayR(fig,udata.ax,udata);
      else
         % Left mouse button
         set(fig,'WindowButtonMotionFcn',@diptestMotionFcn,...
                 'WindowButtonUpFcn',@diptestButtonUpFcn,...
                 'NumberTitle','off','UserData',[]);   % Solve MATLAB bug!
         set(fig,'UserData',udata);
         updateDisplay(fig,udata.ax,udata);
      end
   end
end


%
% Callback function for mouse move in images
%
function diptestMotionFcn(fig,~)
if strncmp(get(fig,'Tag'),'DIP_Image',9)
   if strcmp(get(get(fig,'CurrentObject'),'Type'),'image')
      udata = get(fig,'UserData');
      updateDisplay(fig,udata.ax,udata);
   end
end

function diptestMotionRFcn(fig,~)
if strncmp(get(fig,'Tag'),'DIP_Image',9)
   if strcmp(get(get(fig,'CurrentObject'),'Type'),'image')
      udata = get(fig,'UserData');
      updateDisplayR(fig,udata.ax,udata);
   end
end


%
% Callback function for mouse up in images
%
function diptestButtonUpFcn(fig,~)
if strncmp(get(fig,'Tag'),'DIP_Image',9)
   udata = get(fig,'UserData');
   %if isfield(udata,'ax')
      set(udata.ax,'Units',udata.oldAxesUnits);
      set(fig,'WindowButtonMotionFcn','','WindowButtonUpFcn','',...
              'NumberTitle',udata.oldNumberTitle);
      udata = rmfield(udata,{'ax','oldAxesUnits','oldNumberTitle'});
   %end
   if isfield(udata,'lineh')
      delete(udata.lineh);
      udata = rmfield(udata,{'coords','lineh'});
   end
   dipfig_titlebar(fig,udata);
   set(fig,'UserData',[]);    % Solve MATLAB bug!
   set(fig,'UserData',udata);
end


%
% Retrieve coordinates and image value, and update figure title.
%
function updateDisplay(fig,ax,udata)
pt = dipfig_getcurpos(ax);
handle = udata.handle;
coords = imagedisplay(handle,'coordinates');
slicing = imagedisplay(handle,'slicing');
coords(slicing(1)) = pt(1);
coords(slicing(2)) = pt(2);
coords = mat2str(coords);
value = imagedisplay(handle,pt);
str = [coords,' : ',value];
set(fig,'Name',str);
% TODO: what if empty display?
% TODO: what with dipgetpref('ComplexMappingDisplay')?

function updateDisplayR(fig,ax,udata) %right click for distance measurements in plane
pt = dipfig_getcurpos(ax);
set(udata.lineh,'XData',[udata.coords(1),pt(1)],'YData',[udata.coords(2),pt(2)]);
delta = pt - udata.coords;
coords = mat2str(delta);
len = sqrt(sum(delta.^2));
handle = udata.handle;
value = imagedisplay(handle,pt);
str = [coords,' ',num2str(len),' : ',value];
set(fig,'Name',str);


%
% Callback function for mouse down in 1D images
%
function diptestButtonDown1Fcn(fig,~)
if strncmp(get(fig,'Tag'),'DIP_Image_1D',12)
   ax = findobj(fig,'Type','axes');
   if length(ax)==1
      udata = get(fig,'UserData');
      if isfield(udata,'lineh'), return, end % avoid clicking with the other button
      udata.ax = ax;
      udata.oldAxesUnits = get(ax,'Units');
      udata.oldNumberTitle = get(fig,'NumberTitle');
      set(ax,'Units','pixels');
      if strcmp(get(fig,'SelectionType'),'alt')
         % Right mouse button
         udata.coords = dipfig_getcurpos(ax); % Always over image!
         if useshg2
            udata.Zlineh = line([udata.coords(1),udata.coords(1)],get(ax,'YLim'),...
                                'Color',[0.7,0,0]);
            udata.lineh = line([udata.coords(1),udata.coords(1)],get(ax,'YLim'),...
                               'Color',[0,0,0.8]);
         else
            udata.Zlineh = line([udata.coords(1),udata.coords(1)],get(ax,'YLim'),...
                                'EraseMode','xor','Color',[0.7,0,0]);
            udata.lineh = line([udata.coords(1),udata.coords(1)],get(ax,'YLim'),...
                               'EraseMode','xor','Color',[0,0,0.8]);
         end
         set(fig,'WindowButtonMotionFcn',@diptestMotion1Fcn,...
                 'WindowButtonUpFcn',@diptestButtonUp1Fcn,...
                 'NumberTitle','off','UserData',[]);   % Solve MATLAB bug!
         set(fig,'UserData',udata);
         updateDisplay1(fig,ax,udata);
      else
         % Left mouse button
         if useshg2
            udata.lineh = line([0,0],get(ax,'YLim'),'Color',[0,0,0.8]);
         else
            udata.lineh = line([0,0],get(ax,'YLim'),'EraseMode','xor','Color',[0,0,0.8]);
         end
         set(fig,'WindowButtonMotionFcn',@diptestMotion1Fcn,...
                 'WindowButtonUpFcn',@diptestButtonUp1Fcn,...
                 'NumberTitle','off','UserData',[]);   % Solve MATLAB bug!
         set(fig,'UserData',udata);
         updateDisplay1(fig,ax,udata);
      end
   end
end


%
% Callback functions for mouse move in 1D images
%
function diptestMotion1Fcn(fig,~)
if strncmp(get(fig,'Tag'),'DIP_Image_1D',12)
   udata = get(fig,'UserData');
   updateDisplay1(fig,udata.ax,udata);
end


%
% Callback function for mouse up in 1D images
%
function diptestButtonUp1Fcn(fig,~)
if strncmp(get(fig,'Tag'),'DIP_Image_1D',12)
   udata = get(fig,'UserData');
   set(udata.ax,'Units',udata.oldAxesUnits);
   set(fig,'WindowButtonMotionFcn','','WindowButtonUpFcn','',...
           'NumberTitle',udata.oldNumberTitle);
   delete(udata.lineh);
   udata = rmfield(udata,{'ax','oldAxesUnits','oldNumberTitle','lineh'});
   if isfield(udata,'Zlineh')
      delete(udata.Zlineh);
      udata = rmfield(udata,{'coords','Zlineh'});
   end
   dipfig_titlebar(fig,udata);
   set(fig,'UserData',[]);    % Solve MATLAB bug!
   set(fig,'UserData',udata);
end


%
% Retrieve 1D coordinate and image value, and update figure title.
%
function updateDisplay1(fig,ax,udata)
pt = dipfig_getcurpos(ax);
set(udata.lineh,'XData',[pt(1),pt(1)]);
str = ['(',num2str(pt(1)),')'];
if isfield(udata,'Zlineh')
   delta = pt(1) - udata.coords(1);
   len = abs(delta);
   str = [str,' ',num2str(len)];
end
str = [str,' : '];
if ~isempty(udata.colspace)
   str = [str,udata.colspace,' = ['];
   data = double(udata.colordata(pt(1)));
   for ii=1:length(data)
      str = [str,formatvalue(data(ii)),','];
   end
   str(end) = ']';
else
   str = [str,formatvalue(double(udata.imagedata(pt(1))))];
end
set(fig,'Name',str);
