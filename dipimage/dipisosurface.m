%DIPISOSURFACE   Plot isosurfaces of 3D grey value images
%
%   DIPISOSURFACE(IMAGE,THRESH) plots the THRESH isosurface of IMAGE
%   (i.e. the surface of IMAGE thresholded at THRESH). The initial
%   threshold is (max(IMAGE)+min(IMAGE))/2.
%
%   DIPISOSURFACE(H) plots an isosurface for the image in the display
%   with handle H.
%
%   Slow for large images, and uses lots of memory for the calculation.

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

% TODO: make the window resizable, and add a Resize callback to position the controls

function dipisosurface(in,value)

%Tread the callbacks first
if nargin == 1 && ischar(in)
   switch in
      case 'axis_change'
         axis_change
      case 'text_change'
         text_change
      case 'slider_change'
         slider_change
      case 'grid_change'
         grid_change
      case 'printi'
         printi
      otherwise
         error('Shouldn''t happen.');
   end
   return
end

%Check if input is a figure handle or an image
if isfigh(in)
   udata = get(in,'UserData');
   if strncmp(get(in,'Tag'),'DIP_Image_3D',12) && ~iscolor(udata.slices)
      in = udata.slices;
   else
      error('Cannot create isosurface for image.')
   end
else
   in = dip_image(in);
end

ma = max(in);
mi = min(in);
sz = size(in);
if nargin<2
   value = (ma+mi)/2;
end

plotF = figure('Resize','off'); %don't mess with the window
col = get(plotF,'Color');
ax = axes('parent',plotF);
Hslider = uicontrol('Parent',plotF,'Style','slider',...
   'Position',[10 90 20 256],'Min',mi,'Max',ma,'SliderStep',[.01 .25 ],...
   'Callback','dipisosurface slider_change','Value',value,...
   'Interruptible','off','BusyAction','cancel');
Htext = uicontrol('Parent',plotF,'Style','edit',...
   'Position',[10 350 60 20],'Callback','dipisosurface text_change',...
   'Interruptible','off','BusyAction','cancel',...
   'String',num2str(value),'BackgroundColor',[1,1,1]);
uicontrol('Parent',plotF,'Style','checkbox','String','Grid',...
   'Position',[10 40 70 25],'Callback','dipisosurface grid_change',...
   'BackgroundColor',col,'Interruptible','off','BusyAction','cancel');
uicontrol('Parent',plotF,'Style','checkbox', 'String', 'Axis Equal',...
   'Position',[10 10 70 25], 'Callback', 'dipisosurface axis_change',...
   'BackgroundColor',col,'Interruptible','off','BusyAction','cancel');
uicontrol('Parent',plotF,'Style','pushbutton', 'String', 'Delete Buttons',...
   'Position',[450 10 90 25], 'Callback', 'dipisosurface printi',...
   'BackgroundColor',col,'Interruptible','off','BusyAction','cancel');
      
Hcomp = uicontrol('Parent',plotF,'Style','text','Position',[150 390 100 15],...
   'String','Computing...','ForegroundColor',[1 0 0],...
   'BackgroundColor',col,'Interruptible','off','BusyAction','cancel');

udata = [];
udata.slices = in;
udata.sl = Hslider;
udata.te = Htext;
udata.co = Hcomp;
udata.max = ma;
udata.min = mi;
set(plotF,'UserData',udata);   %copy all stuff from fig to plotF

drawnow
patch(isosurface(0:sz(1)-1,0:sz(2)-1,0:sz(3)-1,double(in),value),'parent',ax,...
   'facelighting','phong','facecolor',[0,1,0],'edgecolor','none');
set(ax,'xlim',[0,sz(1)-1],'ylim',[0,sz(2)-1],'zlim',[0,sz(3)-1],...
   'projection','perspective','box','on',...
   'cameraposition',[-4.0657,-5.4502,4.8301].*sz); % view(3)
% xlabel('x'), ylabel('y'), zlabel('z')
delete(findobj(plotF,'type','light'));
light('parent',ax,'position',get(ax,'CameraPosition'));
light('parent',ax,'position',-get(ax,'CameraPosition')); % one light on each side of the surface
drawnow
set(udata.co,'Visible','off');
xlabel('x-axis')
ylabel('y-axis')
axis ij

function axis_change
[ob,plotF] = gcbo;
ax = findobj(plotF,'type','axes');
if length(ax) ~= 1, return, end
value = get(ob,'Value');
if value
   set(ax,'DataAspectRatio',[1,1,1]);
else
   udata = get(gcf,'UserData');
   sz = size(udata.slices);
   set(ax,'DataAspectRatioMode','auto');
end


function grid_change
[ob,plotF] = gcbo;
ax = findobj(plotF,'type','axes');
if length(ax) ~= 1, return, end
value = get(ob,'Value');
if value
   set(ax,'XGrid','on','YGrid','on','ZGrid','on');
else
   set(ax,'XGrid','off','YGrid','off','ZGrid','off');
end


function slider_change
[ob,plotF] = gcbo;
ax = findobj(plotF,'type','axes');
if length(ax) ~= 1, return, end
udata = get(plotF,'UserData');
value = get(ob,'Value');
set(udata.te,'String',num2str(value));
set(udata.co,'visible','on');
drawnow
delete(get(ax,'Children')); % deletes patch and light objects
patch(isosurface(double(udata.slices),value),'parent',ax,...
   'facelighting','phong','facecolor',[0,1,0],'edgecolor','none');
light('parent',ax,'position',get(ax,'CameraPosition'));
drawnow
set(udata.co,'visible','off');


function text_change
[ob,plotF] = gcbo;
udata = get(plotF,'UserData');
value = str2num(get(udata.te,'String'));
if value
   if value < udata.min
      value = udata.min;
   elseif value > udata.max
      value = udata.max;
   end
   ax = findobj(plotF,'type','axes');
   if length(ax) ~= 1, return, end
   set(udata.te,'String',num2str(value));
   set(udata.sl,'Value',value);
   set(udata.co,'visible','on');
   drawnow
   delete(get(ax,'Children')); % deletes patch and light objects
   patch(isosurface(double(udata.slices),value),'parent',ax,...
      'facelighting','phong','facecolor',[0,1,0],'edgecolor','none');
   light('parent',ax,'position',get(ax,'CameraPosition'));
   drawnow
   set(udata.co,'visible','off');
end

function printi
[ob,plotF] = gcbo;
h=findobj(plotF,'type','uicontrol');
for ii=1:length(h)
   delete(h(ii));
end
