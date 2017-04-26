%DIPLOOKING   Interactive looking glas over the image
%   DIPLOOKING ON turns on interactive looking glass of the current image.
%   DIPLOOKING OFF turns it off.
%   DIPLOOKING by itself toggles the state.
%
%   When DIPLOOKING is turned on, clicking on the image with the mouse
%   button zooms in on a region under the cursor, 3x magnification.
%
%   DIPLOOKING only works on figure windows created through DIPSHOW.
%
%   Also: DIPLOOKING(H,'ON'), DIPLOOKING(H,'OFF'), etc. to specify a window handle.
%
%   See also DIPSHOW, DIPTEST, DIPORIEN, DIPTRUESIZE, DIPSTEP, DIPLINK.

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

function diplooking(arg1,arg2)

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
if strncmp(get(fig,'Tag'),'DIP_Image',9)
   udata = get(fig,'UserData');
   switch (action)
   case 'toggle'
      if strcmp(udata.state,'diplooking')
         dipfig_clear_state(fig,udata);
      else
         makeDIPlookingObj(fig,udata);
      end
   case 'on'
      if ~strcmp(udata.state,'diplooking')
         makeDIPlookingObj(fig,udata);
      end
   case 'off'
      if strcmp(udata.state,'diplooking')
         dipfig_clear_state(fig,udata);
      end
   end
end


%
% Enable the diplooking glass tool
%
function makeDIPlookingObj(fig,udata)
% constants
width = 30;     % size of area in looking glass. repeated in diplookingButtonDownFcn() and diplookingUpdateMiniWindow()!
%
udata.state = 'diplooking';
set(fig,'WindowButtonDownFcn',@diplookingButtonDownFcn,...
        'WindowButtonUpFcn','',...
        'WindowButtonMotionFcn','',...
        'ButtonDownFcn','',...
        'UserData',[]);   % Solve MATLAB bug!
set(fig,'UserData',udata);
dipfig_setpointer(fig,'loupe');
dipfig_set_action_check(fig,udata.state);
%may or may not help to increase drawing speed
set(fig,'BackingStore','on');
set(fig,'DoubleBuffer','on');


%
% Callback function for mouse down in images
%
function diplookingButtonDownFcn(fig,~)
% constants
width = 30;     % size of area in looking glass. repeated in makeDIPlookingObj() and diplookingUpdateMiniWindow()!
%
if strncmp(get(fig,'Tag'),'DIP_Image',9)
   img = get(fig,'CurrentObject');
   if strcmp(get(img,'Type'),'image')
      udata = get(fig,'UserData');
      udata.ax = get(img,'Parent');
      udata.oldAxesUnits = get(udata.ax,'Units');
      set(udata.ax,'Units','pixels');
      udata.lookaxes = axes('Parent',fig,'Visible','off','XGrid','off','YGrid','off',...
         'YDir','reverse','PlotBoxAspectRatioMode','auto','Units','pixels',...
         'xlim',[1,width],'ylim',[1,width]);
      image('BusyAction','cancel','Parent',udata.lookaxes,'Interruptible','off',...
         'CDataMapping','direct');
      zo = udata.zoom;
      axpos = get(udata.ax,'position');
      curxlim = get(udata.ax,'XLim');
      curylim = get(udata.ax,'YLim');
      if isempty(zo) || zo==0
         udata.lookaspectratio = [axpos(3)/diff(curxlim),axpos(4)/diff(curylim)];
      else
         udata.lookaspectratio = [zo,zo];
      end
      udata.lookoffset = axpos(1:2);
      udata.lookbottom = [curxlim(1),curylim(2)];
      oldFigUnits = get(fig,'Units');
      set(fig,'Units','pixels');
      figpos = get(fig,'position');
      set(fig,'Units',oldFigUnits);
      udata.lookfigsize = figpos(3:4);
      udata = diplookingUpdateMiniWindow(fig,udata);
      set(fig,'WindowButtonDownFcn','',...
              'WindowButtonUpFcn',@diplookingButtonUpFcn,...
              'WindowButtonMotionFcn',@diplookingButtonMotionFcn,...
              'UserData',[]);   % Solve MATLAB bug!
      set(fig,'UserData',udata);
   end
end

function diplookingButtonUpFcn(fig,~)
if strncmp(get(fig,'Tag'),'DIP_Image',9)
   udata = get(fig,'UserData');
   h = udata.lookaxes;
   if ~isempty(h)
      delete(h);
   end
   set(udata.ax,'Units',udata.oldAxesUnits);
   udata = rmfield(udata,{'lookaxes','lookaspectratio','lookoffset','lookbottom',...
                          'lookfigsize','oldAxesUnits'});
   set(fig,'WindowButtonDownFcn','diplooking(''down'')',...
           'WindowButtonUpFcn','',...
           'WindowButtonMotionFcn','',...
           'UserData',[]);   % Solve MATLAB bug!
   set(fig,'UserData',udata);
end

function diplookingButtonMotionFcn(fig,~)
if strncmp(get(fig,'Tag'),'DIP_Image',9)
   udata = get(fig,'UserData');
   udata = diplookingUpdateMiniWindow(fig,udata);
   set(fig,'UserData',[]);      % Solve MATLAB bug!
   set(fig,'UserData',udata);
end

function udata = diplookingUpdateMiniWindow(fig,udata)
% constants
width = 30;     % size of area in looking glass. repeated in makeDIPlookingObj() and diplookingButtonDownFcn()!
zoomfactor = 3; % zoom factor of looking glass.
%
aspect = udata.lookaspectratio;
pos_co = dipfig_getcurpos(udata.ax);
sz = udata.imsize(1:2);
w_width = width*zoomfactor;  % windows size pixel
co = [pos_co(1)-udata.lookbottom(1),udata.lookbottom(2)-pos_co(2)] - w_width/2;
co = udata.lookoffset + (co.*aspect);
co = max(co,1);
co = min(co,udata.lookfigsize-w_width*aspect);
co(2) = co(2) + 1;
   %%% I've got no idea where this +1 comes from!!!
   %%% Is it necessary in other MATLABs as well, or is it a MATLAB7 problem I'm correcting for here?
mi = max(pos_co-width/2,0);
mi = min(mi,sz-width);
if any(mi<0)
   warning('Image slice too small to use the looking glass.');
   return;
end
cdata = get(findobj(udata.ax,'type','image'),'CData');
if ndims(cdata)==3
   cdata = cdata(mi(2)+(1:width),mi(1)+(1:width),:);%color images
else
   cdata = cdata(mi(2)+(1:width),mi(1)+(1:width));
end      
set(findobj(udata.lookaxes,'type','image'),'cdata',cdata);
set(udata.lookaxes,'Position',[co,w_width*aspect]);
