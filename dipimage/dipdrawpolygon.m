%DIPDRAWPOLYGON   Interactive polygon drawing tool
%   Allows the user to draw a polygon over an existing image display.
%
% SYNOPSIS:
%   v = dipdrawpolygon(figure_handle)
%
% PARAMETERS:
%   figure_handle: optional, defaults to GCF.
%
% RETURNS
%   v: vertices of polygon
%
%   To create the polygon, use the left mouse button to add vertices.
%   A double-click adds a last vertex and finishes the interaction.
%   'Enter' ends the tool without adding a vertex. To remove vertices,
%   use the 'Backspace' or 'Delete' keys, or the right mouse button.
%   'Esc' aborts the operation (returns an error message). Shift-click
%   will add a vertex constrained to a horizontal or vertical location
%   with respect to the previous vertex.
%
%   Pressing space will switch to the editing mode, where each vertex
%   is converted to a little circle and the user can drag these around
%   to change the polygon. Pressing space again will return the tool
%   to the vertex adding mode.
%
%   It is still possible to use all the menus in the victim figure
%   window, but you won't be able to access any of the tools (like
%   zooming and testing). The regular key-binding is also disabled.
%
%   Note: If you feel the need to interrupt this function with Ctrl-C,
%   you will need to refresh the display (by re-displaying the image
%   or changing the 'Actions' state).
%
%   See also DRAWPOLYGON, DIPSHOW, DIPGETCOORDS, DIPROI.

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

function coords = dipdrawpolygon(varargin)

if nargin == 1
   s = varargin{1};
   if ischar(s)
      switch s
      case 'dip_dp_MotionFcn'
         dip_dp_MotionFcn
         return
      case 'dip_dp_MovePoint_ButtonDown'
         dip_dp_MovePoint_ButtonDown
         return
      case 'dip_dp_MovePoint_ButtonMotion'
         dip_dp_MovePoint_ButtonMotion
         return
      case 'dip_dp_MovePoint_ButtonUp'
         dip_dp_MovePoint_ButtonUp
         return
      end
   elseif ishandle(s)
      fig = s;
   else
      error('Illegal input');
   end
else
   fig = get(0,'CurrentFigure');
   if isempty(fig)
      error('No figure window open to do operation on.')
   end
end

tag = get(fig,'Tag');
if ~strncmp(tag,'DIP_Image',9)
   error('DIPGETCOORDS only works on images displayed using DIPSHOW.')
end
ax = findobj(fig,'Type','axes');
if length(ax)~=1
   error('DIPGETCOORDS only works on images displayed using DIPSHOW.')
end

% Store old settings
au = get(ax,'Units');
wbdF = get(fig,'WindowButtonDownFcn');
wbuP = get(fig,'WindowButtonUpFcn');
wbmF = get(fig,'WindowButtonMotionFcn');
bdF = get(fig,'ButtonDownFcn');
kpF = get(fig,'KeyPressFcn');
pscd = get(fig,'PointerShapeCData');
pshs = get(fig,'PointerShapeHotSpot');
ptr = get(fig,'pointer');
nt = get(fig,'NumberTitle');

% Set new settings
figure(fig);
set(fig,'WindowButtonDownFcn','',...
        'WindowButtonUpFcn','',...
        'WindowButtonMotionFcn',@dip_dp_MotionFcn,...
        'ButtonDownFcn','',...
        'KeyPressFcn',@(~,~)set(gcbf,'WindowButtonDownFcn','Key!'),...
        'NumberTitle','off');
dipfig_setpointer(fig,'cross');
set(ax,'Units','pixels');

% Do your stuff
udata = get(fig,'UserData');
udata.ax = ax;
udata.mode = 0;
set(fig,'UserData',[]);
set(fig,'UserData',udata);
coords = [];
done = 0;
escape = 0;
while ~done
   if udata.mode==0
      set(fig,'WindowButtonDownFcn',@(~,~)set(gcbf,'WindowButtonDownFcn','Click!'));
   else
      set(fig,'WindowButtonDownFcn',@dip_dp_MovePoint_ButtonDown);
   end
   waitfor(fig,'WindowButtonDownFcn'); % The ButtonDown callback changes the callback.
                                       % This way, we also detect a change in state!
   if ~ishandle(fig)
      error('Aborted.')
   end
   switch get(fig,'WindowButtonDownFcn')
   case 'Click!'
      switch get(fig,'SelectionType')
         case 'normal'
            pt = dipfig_getcurpos(ax);
            coords = [coords;pt];
            if isfield(udata,'lineh')
               set(udata.lineh,'XData',[coords(:,1);pt(1)],'YData',[coords(:,2);pt(2)]);
            else
               if ~isnumeric(fig)
                  udata.lineh = line([coords(:,1);pt(1)],[coords(:,2);pt(2)],...
                                     'Color',[0,0,0.8]);
               else
                  udata.lineh = line([coords(:,1);pt(1)],[coords(:,2);pt(2)],...
                                     'EraseMode','xor','Color',[0,0,0.8]);
               end
               set(fig,'UserData',[]);
               set(fig,'UserData',udata);
            end
         case 'open' % second click of double-click: end
            done = 1;
         case 'extend' % shift-click: constrained
            pt = dipfig_getcurpos(ax);
            if ~isempty(coords)
               lstpt = coords(end,:);
               [~,I] = min(abs(lstpt-pt));
               pt(I) = lstpt(I);
            end
            coords = [coords;pt];
            if isfield(udata,'lineh')
               set(udata.lineh,'XData',[coords(:,1);pt(1)],'YData',[coords(:,2);pt(2)]);
            else
               if ~isnumeric(fig)
                  udata.lineh = line([coords(:,1);pt(1)],[coords(:,2);pt(2)],...
                                     'Color',[0,0,0.8]);
               else
                  udata.lineh = line([coords(:,1);pt(1)],[coords(:,2);pt(2)],...
                                     'EraseMode','xor','Color',[0,0,0.8]);
               end
               set(fig,'UserData',[]);
               set(fig,'UserData',udata);
            end
         case 'alt' % right-click: remove last point
            coords = coords(1:end-1,:);
            if isfield(udata,'lineh')
               if isempty(coords)
                  delete(udata.lineh);
                  udata = rmfield(udata,'lineh');
                  set(fig,'UserData',[]);
                  set(fig,'UserData',udata);
               else
                  pt = dipfig_getcurpos(ax);
                  set(udata.lineh,'XData',[coords(:,1);pt(1)],'YData',[coords(:,2);pt(2)]);
               end
            end
      end
   case 'Key!'
      ch = double(get(fig,'CurrentCharacter'));
      if ~isempty(ch)
         switch ch
            case 13 % Enter: end
               if udata.mode==0
                  done = 1;
               end
            case {127,8} % Delete/Backsp: remove a point
               if udata.mode==0
                  coords = coords(1:end-1,:);
                  if isfield(udata,'lineh')
                     if isempty(coords)
                        delete(udata.lineh);
                        udata = rmfield(udata,'lineh');
                        set(fig,'UserData',[]);
                        set(fig,'UserData',udata);
                     else
                        pt = dipfig_getcurpos(ax);
                        set(udata.lineh,'XData',[coords(:,1);pt(1)],'YData',[coords(:,2);pt(2)]);
                     end
                  end
               end
            case 32 % Space: change mode
               if isfield(udata,'lineh')
                  if udata.mode==0
                     % Change to moving control points
                     udata.mode = 1;
                     set(fig,'WindowButtonDownFcn','',...
                             'WindowButtonUpFcn','',...
                             'WindowButtonMotionFcn','');
                     delete(udata.lineh);
                     if ~isnumeric(fig)
                        udata.lineh = line(coords(:,1),coords(:,2),...
                                           'linestyle',':','marker','o',...
                                           'Color',[0,0,0.8]);
                     else
                        udata.lineh = line(coords(:,1),coords(:,2),...
                                           'linestyle',':','marker','o',...
                                           'EraseMode','xor','Color',[0,0,0.8]);
                     end
                     set(fig,'Name','');
                     dipfig_setpointer(fig,'hand_finger');
                     set(fig,'UserData',[]);
                     set(fig,'UserData',udata);
                  else
                     % Change to default mode
                     udata.mode = 0;
                     set(fig,'WindowButtonDownFcn','',...
                             'WindowButtonUpFcn','',...
                             'WindowButtonMotionFcn',@dip_dp_MotionFcn);
                     xd = get(udata.lineh,'XData');
                     yd = get(udata.lineh,'YData');
                     coords = [xd(:),yd(:)]; % get the new coords from the line
                     delete(udata.lineh);
                     pt = dipfig_getcurpos(ax);
                     if ~isnumeric(fig)
                        udata.lineh = line([coords(:,1);pt(1)],[coords(:,2);pt(2)],...
                                           'Color',[0,0,0.8]);
                     else
                        udata.lineh = line([coords(:,1);pt(1)],[coords(:,2);pt(2)],...
                                           'EraseMode','xor','Color',[0,0,0.8]);
                     end
                     if isfield(udata,'selectedptindex')
                        udata = rmfield(udata,'selectedptindex');
                     end
                     set(fig,'Name','');
                     dipfig_setpointer(fig,'cross');
                     set(fig,'UserData',[]);
                     set(fig,'UserData',udata);
                  end
               end
            case 27 % Escape: quit
               done = 1;
               escape = 1;
         end
      end
   otherwise
      % The user just changed the state. Store the new settings and revert to our own...
      wbdF = get(fig,'WindowButtonDownFcn');
      wbuP = get(fig,'WindowButtonUpFcn');
      wbmF = get(fig,'WindowButtonMotionFcn');
      bdF = get(fig,'ButtonDownFcn');
      pscd = get(fig,'PointerShapeCData');
      pshs = get(fig,'PointerShapeHotSpot');
      ptr = get(fig,'pointer');
      set(fig,'WindowButtonDownFcn','',...
              'WindowButtonUpFcn','',...
              'WindowButtonMotionFcn','',...
              'ButtonDownFcn','',...
              'KeyPressFcn',@(~,~)set(gcbf,'WindowButtonDownFcn','Key!'));
      if udata.mode==0
         set(fig,'WindowButtonMotionFcn',@dip_dp_MotionFcn);
         dipfig_setpointer(fig,'cross');
      else
         dipfig_setpointer(fig,'hand_finger');
      end
      set(ax,'Units','pixels');
   end
end

% Restore old settings
if isfield(udata,'lineh')
   delete(udata.lineh);
   udata = rmfield(udata,'lineh');
end
udata = rmfield(udata,{'mode','ax'});
set(ax,'Units',au);
set(fig,'WindowButtonDownFcn',wbdF,...
        'WindowButtonUpFcn',wbuP,...
        'WindowButtonMotionFcn',wbmF,...
        'ButtonDownFcn',bdF,...
        'KeyPressFcn',kpF,...
        'PointerShapeCData',pscd,...
        'PointerShapeHotSpot',pshs,...
        'pointer',ptr,...
        'NumberTitle',nt,...
        'UserData',[]);
set(fig,'UserData',udata);
dipfig_titlebar(fig,udata);
% Generate output image
if escape
   error('Aborted.')
end
  


%
% Callback funtion for mouse move
%
function dip_dp_MotionFcn(fig,~)
if strncmp(get(fig,'Tag'),'DIP_Image',9)
   udata = get(fig,'UserData');
   pt = dipfig_getcurpos(udata.ax);
   if isfield(udata,'lineh')
      xd = get(udata.lineh,'XData');
      yd = get(udata.lineh,'YData');
      xd(end) = pt(1);
      yd(end) = pt(2);
      set(udata.lineh,'XData',xd,'YData',yd);
   end
   set(fig,'Name',['(',num2str(pt(1)),',',num2str(pt(2)),')']);
end


%
% Callback functions for mode==1, where we allow the user to move points around
%
function dip_dp_MovePoint_ButtonDown(fig,~)
if strncmp(get(fig,'Tag'),'DIP_Image',9)
   udata = get(fig,'UserData');
   if isfield(udata,'lineh')
      pt = dipfig_getcurpos(udata.ax);
      xd = get(udata.lineh,'XData');
      yd = get(udata.lineh,'YData');
      [dist,ii] = min((xd-pt(1)).^2+(yd-pt(2)).^2);
      if dist<5
         set(fig,'WindowButtonUpFcn','dipdrawpolygon(''dip_dp_MovePoint_ButtonUp'')',...
                 'WindowButtonMotionFcn','dipdrawpolygon(''dip_dp_MovePoint_ButtonMotion'')');
         udata.selectedptindex = ii;
         set(fig,'Name',['(',num2str(xd(ii)),',',num2str(yd(ii)),')']);
         set(fig,'UserData',[]);
         set(fig,'UserData',udata);
      end
   end
end

function dip_dp_MovePoint_ButtonMotion(fig,~)
if strncmp(get(fig,'Tag'),'DIP_Image',9)
   udata = get(fig,'UserData');
   if isfield(udata,'lineh') && isfield(udata,'selectedptindex')
      pt = dipfig_getcurpos(udata.ax);
      xd = get(udata.lineh,'XData');
      yd = get(udata.lineh,'YData');
      ii = udata.selectedptindex;
      xd(ii) = pt(1);
      yd(ii) = pt(2);
      set(udata.lineh,'XData',xd,'YData',yd);
      set(fig,'Name',['(',num2str(pt(1)),',',num2str(pt(2)),')']);
   end
end

function dip_dp_MovePoint_ButtonUp(fig,~)
if strncmp(get(fig,'Tag'),'DIP_Image',9)
   udata = get(fig,'UserData');
   if isfield(udata,'selectedptindex')
      udata = rmfield(udata,'selectedptindex');
      set(fig,'UserData',[]);
      set(fig,'UserData',udata);
   end
   set(fig,'Name','');
   set(fig,'WindowButtonUpFcn','',...
           'WindowButtonMotionFcn','');
end
