%DIPPROFILE   Interactive extraction of 1D function from image
%   B = DIPPROFILE(H) returns a 1D image extracted from the image in
%   the figure window with handle H, which defaults to the current
%   figure. The user is allowed to define a line over the image
%   composed of multiple straight segments. The image is interpolated
%   along this line to obtain the 1D image (using cubic interpolation).
%
%   [B,X] = DIPPROFILE(H) also returns the coordinates of the samples
%   in X. X is a N-by-2 array, where N is the size of B.
%
%   B = DIPPROFILE(H,N) terminates automatically after N clicks.
%
%   DIPPROFILE is only available for 2D figure windows.
%
%   To create the line, use the left mouse button to add points.
%   A double-click adds a last point. 'Enter' terminates the line without
%   adding a point. To remove points, use the 'Backspace' or 'Delete'
%   keys, or the right mouse button. 'Esc' aborts the operation.
%   Shift-click will add a point constrained to a horizontal or vertical
%   location with respect to the previous vertex.
%
%   Note that you need to select at least two points. If you don't, an
%   error will be generated.
%
%   It is still possible to use all the menus in the victim figure
%   window, but you won't be able to access any of the tools (like
%   zooming and testing). The regular key-binding is also disabled.
%
%   Note: If you feel the need to interrupt this function with Ctrl-C,
%   you will need to refresh the display (by re-displaying the image
%   or changing the 'Actions' state).
%
%   See also DIPSHOW, DIPGETCOORDS, DIPCROP, DIPROI.

% (c)1999-2014, Delft University of Technology
% (c)2017, Cris Luengo

function [output,coords] = dipprofile(arg1,N)

% Parse input
vertices = 0;
if nargin == 0
   fig = [];
else
   if ischar(arg1)
      if strcmp(arg1,'motion')
         dipprofMotionFcn;
         return
      else
         fig = arg1;
      end
   else
      fig = arg1;
   end
end
if isempty(fig)
   fig = get(0,'CurrentFigure');
   if isempty(fig)
      error('No figure window open to do operation on.')
   end
else
   try
      fig = getfigh(fig);
   catch
      error('Argument must be a valid figure handle.')
   end
end

tag = get(fig,'Tag');
if ~strncmp(tag,'DIP_Image_2D',12)
   error('DIPPROFILE only works on 2D images displayed using DIPSHOW.')
end
ax = findobj(fig,'Type','axes');
if length(ax)~=1
   error('DIPPROFILE only works on 2D images displayed using DIPSHOW.')
end
if nargin <2
   N=0;
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
        'WindowButtonMotionFcn','dipprofile(''motion'')',...
        'ButtonDownFcn','',...
        'KeyPressFcn','set(gcbf,''WindowButtonDownFcn'',''Key!'')',...
        'NumberTitle','off');
dipfig_setpointer(fig,'cross');
set(ax,'Units','pixels');

% Do your stuff
udata = get(fig,'UserData');
udata.ax = ax;
set(fig,'UserData',[]);
set(fig,'UserData',udata);
coords = [];
done = 0;
escape = 0;
while ~done
   set(fig,'WindowButtonDownFcn','set(gcbf,''WindowButtonDownFcn'',''Click!'')');
   waitfor(fig,'WindowButtonDownFcn'); % The ButtonDown callback changes the callback.
                                       % This way, we also detect a change in state!
   if ~ishandle(fig)
      error('You closed the window! That wasn''t the deal!')
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
               if useshg2
                  udata.lineh = line([coords(:,1);pt(1)],[coords(:,2);pt(2)],...
                                     'Color',[0,0,0.8]);
               else
                  udata.lineh = line([coords(:,1);pt(1)],[coords(:,2);pt(2)],...
                                     'EraseMode','xor','Color',[0,0,0.8]);
               end
               set(fig,'UserData',[]);
               set(fig,'UserData',udata);
            end
            if size(coords,1)==N
               done=1;
            end
         case 'open' % second click of double-click: end
            done = 1;
         case 'extend' % shift-click: constrained
            pt = dipfig_getcurpos(ax);
            if ~isempty(coords)
               lstpt = coords(end,:);
               [v,I] = min(abs(lstpt-pt));
               pt(I) = lstpt(I);
            end
            coords = [coords;pt];
            if isfield(udata,'lineh')
               set(udata.lineh,'XData',[coords(:,1);pt(1)],'YData',[coords(:,2);pt(2)]);
            else
               if useshg2
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
               done = 1;
            case {127,8} % Delete/Backsp: remove a point
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
              'WindowButtonMotionFcn','dipprofile(''motion'')',...
              'ButtonDownFcn','',...
              'KeyPressFcn','set(gcbf,''WindowButtonDownFcn'',''Key!'')');
      dipfig_setpointer(fig,'cross');
      set(ax,'Units','pixels');
   end
end

% Restore old settings
if isfield(udata,'lineh')
   delete(udata.lineh);
   udata = rmfield(udata,'lineh');
end
udata = rmfield(udata,'ax');
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
points = coords;
N = size(points,1);
if N<2
   error('You need to select at least two points.')
end
coords = [];
for ii=1:(N-1)
   len = norm(points(ii+1,:)-points(ii,:));
   k = floor(len);
   tmp = repmat(points(ii,:),[k,1])+(0:k-1)'*((points(ii+1,:)-points(ii,:))/len);
   coords = [coords;tmp];
end

if ~isfield(udata,'channels')
   output = dip_image(get_subpixel(udata.imagedata,coords,'cubic'));
else
   output = newimar(udata.channels);
   tmp = udata.imagedata;
   for ii=1:udata.channels
      output{ii} = dip_image(get_subpixel(squeeze(tmp(:,:,ii-1)),coords,'cubic'));
   end
   output = colorspace(output,udata.colspace);
end

%
% Callback funtion for mouse move
%
function dipprofMotionFcn
fig = gcbf;
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
