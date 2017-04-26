%DIPPAN   Interactive panning of an image
%   DIPPAN ON turns on panning of the current image.
%   DIPPAN OFF turns it off.
%   DIPPAN by itself toggles the state.
%
%   When DIPPAN is turned on, it is possible to pan the image by clicking
%   and dragging with the mouse on the image. This is especially useful
%   when working with very large images (i.e. they don't fit on your screen)
%   or when zooming in on an image.
%
%   DIPPAN only works on figure windows created through DIPSHOW.
%
%   Also: DIPPAN(H,'ON'), DIPPAN(H,'OFF'), etc. to specify a window handle.
%
%   See also DIPSHOW, DIPZOOM, DIPSTEP.

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

function dippan(arg1,arg2)

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
      udata = get(fig,'UserData');
      switch (action)
      case 'toggle'
         if strcmp(udata.state,'dippan')
            dipfig_clear_state(fig,udata);
         else
            makeDIPpanObj(fig,udata);
         end
      case 'on'
         if ~strcmp(udata.state,'dippan')
            makeDIPpanObj(fig,udata);
         end
      case 'off'
         if strcmp(udata.state,'dippan')
            dipfig_clear_state(fig,udata);
         end
      end
   end
end


%
% Enable the dippan tool.
%
function makeDIPpanObj(fig,udata)
udata.state = 'dippan';
set(fig,'WindowButtonDownFcn',@dippanButtonDownFcn);
set(fig,'WindowButtonUpFcn','',...
        'WindowButtonMotionFcn','',...
        'ButtonDownFcn','',...
        'UserData',[]);   % Solve MATLAB bug!
set(fig,'UserData',udata);
dipfig_setpointer(fig,'hand_open');
dipfig_set_action_check(fig,udata.state);


%
% Callback function for mouse down in images
%
function dippanButtonDownFcn(fig,~)
if strncmp(get(fig,'Tag'),'DIP_Image',9)
   ax = findobj(fig,'Type','axes');
   if length(ax)==1
      dipfig_setpointer(fig,'hand_closed');
      udata = get(fig,'UserData');
      udata.ax = ax;
      udata.oldAxesUnits = get(udata.ax,'Units');
      set(udata.ax,'Units','pixels');
      udata.coords = dipfig_getcurpos(udata.ax,0);
      set(fig,'WindowButtonMotionFcn',@dippanMotionFcn,...
              'WindowButtonUpFcn',@dippanButtonUpFcn,...
              'UserData',[]);   % Solve MATLAB bug!
      set(fig,'UserData',udata);
   end
end


%
% Callback function for mouse move in images
%
function dippanMotionFcn(fig,~)
if strncmp(get(fig,'Tag'),'DIP_Image',9)
   udata = get(fig,'UserData');
   stepsize = udata.coords - dipfig_getcurpos(udata.ax,0);
   if length(udata.imsize) == 1
      stepsize(2) = 0;
   end
   curxlim = get(udata.ax,'Xlim');
   curylim = get(udata.ax,'Ylim');
   if stepsize(1) < 0
      stepsize(1) = max(stepsize(1),-0.5-curxlim(1));
   elseif stepsize(1) > 0
      stepsize(1) = min(stepsize(1),udata.imsize(1)-0.5-curxlim(2));
   end
   if length(udata.imsize) > 1
      if stepsize(2) < 0
         stepsize(2) = max(stepsize(2),-0.5-curylim(1));
      elseif stepsize(2) > 0
         stepsize(2) = min(stepsize(2),udata.imsize(2)-0.5-curylim(2));
      end
   end
   set(udata.ax,'Xlim',curxlim+stepsize(1),'Ylim',curylim+stepsize(2));
   % We're not updating udata.coords, since it shouldn't change at all!
   %udata.coords = dipfig_getcurpos(udata.ax,0);
   %set(fig,'UserData',[]);    % Solve MATLAB bug!
   %set(fig,'UserData',udata);
end

%
% Callback function for mouse up in images
%
function dippanButtonUpFcn(fig,~)
if strncmp(get(fig,'Tag'),'DIP_Image',9)
   udata = get(fig,'UserData');
   set(udata.ax,'Units',udata.oldAxesUnits);
   udata = rmfield(udata,{'ax','oldAxesUnits','coords'});
   set(fig,'WindowButtonMotionFcn','','WindowButtonUpFcn','');
   dipfig_setpointer(fig,'hand_open');
   set(fig,'UserData',[]);    % Solve MATLAB bug!
   set(fig,'UserData',udata);
   % Update linked displays
   if ~isempty(udata.linkdisplay)
      dipshow(fig,'updatelinked',[]);
   end
end
