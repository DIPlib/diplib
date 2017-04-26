%DIPZOOM   Interactive image zooming
%   DIPZOOM ON turns on interactive zooming of the current image.
%   DIPZOOM OFF turns it off.
%   DIPZOOM by itself toggles the state.
%
%   When DIPZOOM is turned on, clicking on the image with the left mouse
%   button zooms in, and with the right mouse button zooms out. A double-
%   click will set the zoom factor to 100%. You can also drag a rectangle
%   around the region of interest, which will be zoomed such that it fits
%   the figure window. DIPZOOM never changes the size of the window itself.
%
%   DIPZOOM only works on figure windows created through DIPSHOW.
%
%   Also: DIPZOOM(H,'ON'), DIPZOOM(H,'OFF'), etc. to specify a window handle.
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

function dipzoom(arg1,arg2)

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
      if strcmp(udata.state,'dipzoom')
         dipfig_clear_state(fig,udata);
      else
         makeDIPzoomObj(fig,udata);
      end
   case 'on'
      if ~strcmp(udata.state,'dipzoom')
         makeDIPzoomObj(fig,udata);
      end
   case 'off'
      if strcmp(udata.state,'dipzoom')
         dipfig_clear_state(fig,udata);
      end
   end
end


%
% Enable to dipzoom tool
%
function makeDIPzoomObj(fig,udata)
udata.state = 'dipzoom';
set(fig,'WindowButtonDownFcn',dipshow('DIP_callback','dipzoomWindowButtonDownFcn'),...
        'WindowButtonUpFcn','',...
        'WindowButtonMotionFcn','',...
        'ButtonDownFcn','',...
        'UserData',[]);   % Solve MATLAB bug!
set(fig,'UserData',udata);
dipfig_setpointer(fig,'loupe');
dipfig_set_action_check(fig,udata.state);
