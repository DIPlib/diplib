%DIPSTEP   Stepping through slices of a 3D image
%   DIPSTEP ON turns on stepping through slices of the current 3D image.
%   DIPSTEP OFF turns it off.
%   DIPSTEP by itself toggles the state.
%
%   When DIPSTEP is turned on, clicking on the image with the left mouse
%   button shows the next slice of the 3D image, and with the right one
%   the previous slice.
%
%   Dragging the mouse over the image works like dragging a scrollbar:
%   drag down or to the right to go to higher slice numbers, drag up or to
%   the left to go to lower slice numbers. The linked displays will only
%   be updated after you release the mouse button. For 4D images, drag with
%   the right mouse button to move through the 4th dimension.
%
%   DIPSTEP only works on figure windows created through DIPSHOW with 3D
%   or 4D image data.
%
%   Also: DIPSTEP(H,'ON'), DIPSTEP(H,'OFF'), etc. to specify a window handle.
%
%   See also DIPSHOW, DIPTEST, DIPORIEN, DIPZOOM, DIPLINK.

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

function dipstep(arg1,arg2)

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
if strncmp(get(fig,'Tag'),'DIP_Image_3D',12) || strncmp(get(fig,'Tag'),'DIP_Image_4D',12)
   udata = get(fig,'UserData');
   switch (action)
   case 'toggle'
      if strcmp(udata.state,'dipstep')
         dipfig_clear_state(fig,udata);
      else
         makeDIPstepObj(fig,udata);
      end
   case 'on'
      if ~strcmp(udata.state,'dipstep')
         makeDIPstepObj(fig,udata);
      end
   case 'off'
      if strcmp(udata.state,'dipstep')
         dipfig_clear_state(fig,udata);
      end
   end
end


%
% Enable the stepping through the 3D images
%
function makeDIPstepObj(fig,udata)
udata.state = 'dipstep';
set(fig,'pointer','arrow',...
        'WindowButtonDownFcn',dipshow('DIP_callback','dipstepWindowButtonDownFcn'),...
        'WindowButtonUpFcn','',...
        'WindowButtonMotionFcn','',...
        'ButtonDownFcn','',...
        'UserData',[]);              % Solve MATLAB bug!
set(fig,'UserData',udata);
dipfig_set_action_check(fig,udata.state);
