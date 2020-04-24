%DIPSTACKINSPECT   Interactive inspection of the third dimension
%   [OUT,P] = DIPSTACKINSPECT(H), for a 3D figure window with
%   handle H, enables a tool to display the intensity profile along
%   the hidden dimension.
%
%   Left-click in the image shows the intensity along the hidden
%   dimension. Right-click to terminate.
%
%   OUT is a 1D image with the values along the hidden dimension
%   for the selected point. P contains the 2D coordinates of the
%   selected point.
%
%   DIPSTACKINSPECT(H,TRUE) closes the window after right-click.
%
%   DIPSTACKINSPECT(H,...,TRUE) allows to select a box size. The
%   profile shown is averaged over a box of the selected size
%   around the selected point.
%
%   It is still possible to use all the menus in the victim figure
%   window, but you won't be able to access any of the tools (like
%   zooming and testing). You can, however, step through a 3D
%   image by using the keyboard.
%
%   Note: If you feel the need to interrupt this function with Ctrl-C,
%   you will need to refresh the display (by re-displaying the image
%   or changing the 'Actions' state).
%
%   See also DIPSHOW, DIPCROP, DIPPROFILE, DIPGETCOORDS.

% (c)2018, Cris Luengo.
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

function [out,coords] = dipstackinspect(fig,closeWin,useRoi)

% Parse input
if nargin == 0
   fig = get(0,'CurrentFigure');
   if isempty(fig)
      error('No figure window open to do operation on')
   end
else
   try
      fig = getfigh(fig);
   catch
      error('Argument must be a valid figure handle')
   end
end

tag = get(fig,'Tag');
if ~strncmp(tag,'DIP_Image_3D',12)
   error('DIPSTACKINSPECT only works on 3D images displayed using DIPSHOW.')
end
ax = findobj(fig,'Type','axes');
if numel(ax)~=1
   error('DIPSTACKINSPECT only works on images displayed using DIPSHOW.')
end

if nargin<3
   useRoi = false;
end
if nargin<2
   closeWin = false;
end

udata = get(fig,'UserData');
img = imagedisplay(udata.handle,'input');

if useRoi
   [~,roi] = dipcrop(fig);
   roiSz = roi(2,:);
   fprintf('Selected box size %d %d\n',roiSz+1);
   hrect = rectangle('Position',[roi(1,:),roi(2,:)]);
   set(hrect,'EdgeColor',[0.8,0.8,0]);
   xroi = roi(1,1);
   xroi = (0:xroi)-floor(roiSz(1)/2);
   yroi = roi(1,2);
   yroi = (0:yroi)-floor(roiSz(2)/2);
else
   xroi = 0;
   yroi = 0;
end

% Store old settings
au = get(ax,'Units');
wbdF = get(fig,'WindowButtonDownFcn');
wbuP = get(fig,'WindowButtonUpFcn');
wbmF = get(fig,'WindowButtonMotionFcn');
bdF = get(fig,'ButtonDownFcn');
pscd = get(fig,'PointerShapeCData');
pshs = get(fig,'PointerShapeHotSpot');
ptr = get(fig,'pointer');

% Set new settings
figure(fig);
set(fig,'WindowButtonDownFcn','',...
        'WindowButtonUpFcn','',...
        'WindowButtonMotionFcn','',...
        'ButtonDownFcn','');
dipfig_setpointer(fig,'cross');
set(ax,'Units','pixels');

% Do your stuff
h = figure;
h = axes('parent',h);
pt = [];
imline = [];
done = 0;
while ~done
   while true
      set(fig,'WindowButtonDownFcn',@(fig,~)set(fig,'WindowButtonDownFcn','Click!'));
      waitfor(fig,'WindowButtonDownFcn'); % The ButtonDown callback changes the callback.
               % This way, we also detect a change in state!
      if ~ishandle(fig)
         error('You closed the window! That wasn''t the deal!')
      end

      if ~strcmp(get(fig,'WindowButtonDownFcn'),'Click!')
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
                 'ButtonDownFcn','');
         dipfig_setpointer(fig,'cross');
         set(ax,'Units','pixels');
         continue;
      else
         set(fig,'WindowButtonDownFcn','');
         switch get(fig,'SelectionType')
         case 'normal' % left click
            pt = dipfig_getcurpos(ax);
         case 'alt'    % right click
            done = 1;
         end
         break;
      end
   end
   if done
      break
   end


   slicing = imagedisplay(udata.handle,'slicing');
   coord = imagedisplay(udata.handle,'coordinates');
   coord(slicing) = pt;
   if useRoi
      set(hrect,'Position',[pt(1)+xroi(1),pt(2)+yroi(1),roiSz]);
   end
   indx = repmat({':'},3,1);
   indx{slicing(1)} = num2str(coord(slicing(1)));
   indx{slicing(2)} = num2str(coord(slicing(2)));
   ttl = sprintf('%s,%s,%s',indx{:});
   indx{slicing(1)} = clip_rect(coord(slicing(1))+xroi,imsize(img,slicing(1)));
   indx{slicing(2)} = clip_rect(coord(slicing(2))+yroi,imsize(img,slicing(2)));
   imline = squeeze(double(mean(img(indx{:}),[],slicing)));
   if ~ishandle(h)
	   h = figure;
	   h = axes('parent',h);
   end
   plot(h,0:numel(imline)-1,imline);
   xlabel(h,'Stack index');
   if useRoi
      ylabel(h,'Mean intensity');
      title(h,['Mean intensity around [',ttl,']']);
   else
      ylabel(h,'Intensity');
      title(h,['Intensity at [',ttl,']']);
   end
end

if useRoi
   delete(hrect)
end

% Restore old settings
set(ax,'Units',au);
set(fig,'WindowButtonDownFcn',wbdF,...
        'WindowButtonUpFcn',wbuP,...
        'WindowButtonMotionFcn',wbmF,...
        'ButtonDownFcn',bdF,...
        'PointerShapeCData',pscd,...
        'PointerShapeHotSpot',pshs,...
        'pointer',ptr);

if closeWin
   close(get(h,'parent'));
end

% Generate output
if isempty(imline)
   error('No point selected');
else
   if nargout~=0
      out = dip_image(imline);
      if useRoi
          coords = [pt,roiSz];
      else
          coords = pt;
      end
   end
end

function r = clip_rect(r,sz)
r = max(r,0);
r = min(r,sz-1);
