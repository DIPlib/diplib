%DIPSTACKINSPECT   Interactive inspection of the third dimension
%   [out,p] = DIPSTACKINSPECT(H, CW, ROI) for a 3D figure window with handle H.
%
% Left-click in the image to see the intensity along the hidden dimension.
%            Possibly the sum intensity in a box if you set ROI
% Right-click to terminate.
%
% OUTPUT:
%   out: 1D image along the selected point
%   p:   Coordinate of the selected point
%
% PARAMETERS:
%   H:   figure handle
%   CW:  boolean, close window after terimnation?
%   ROI: boolean, do you want to select a Region?
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
%   See also DIPSHOW, DIPTEST, DIPCROP, DIPPROFILE, DIPGETCOORDS.

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

function [out,coords] = dipstackinspect(fig,clwin,yroi)

% TODO: test input parameters: isfigh(fig),clwin==bool,yroi==bool

tag = get(fig,'Tag');
if ~strncmp(tag,'DIP_Image_3D',12)
   error('DIPSTACKINSPECT only works on 3D images displayed using DIPSHOW.')
end
ax = findobj(fig,'Type','axes');
if length(ax)~=1
   error('DIPSTACKINSPECT only works on images displayed using DIPSHOW.')
end

if yroi
    [out,roi] = dipcrop(fig);
    droi = abs([roi(2,1) roi(2,2)]);
    fprintf('Selected box size %d %d\n',droi+1);
    hrect = rectangle('Position', [roi(1,:) roi(2,:)]);
    set(hrect,'EdgeColor',[0.8 0.8 0]);
else
    droi= [0 0];
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
   while 1
      set(fig,'WindowButtonDownFcn','set(gcbf,''WindowButtonDownFcn'',''Click!'')');
      waitfor(fig,'WindowButtonDownFcn'); % The ButtonDown callback changes the callback.
               % This way, we also detect a change in state!
      if ~ishandle(fig)
         error('You closed the window! That wasn''t the deal!')
         done = 1;
         break;
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
         switch get(fig,'SelectionType');
         case 'normal' % left click
            pt = dipfig_getcurpos(ax);
         case 'alt'    % right click
            done = 1;
         end
         break;
      end
   end
   if yroi;delete(hrect);end
   if done
      break
   end
   udata = get(fig,'UserData');
   switch udata.slicing
      case 'yz'
         pt = [udata.curslice,pt];
         r2 = [pt(2) pt(2)+droi(1)];
         r3 = [pt(3) pt(3)+droi(2)];
         [r2 r3] = clip_rect(r2, r3, udata.imsize);
         imline = squeeze(double(sum(udata.slices(:, r2(1):r2(2), r3(1):r3(2) ), [], [2:3])));
         ttl = [':,' num2str(r2(1)) ':' num2str(r2(2)) ',' num2str(r3(1)) ':' num2str(r3(2))];
        if yroi
            hrect = rectangle('Position', [pt(2) pt(3) droi]);
        end
       case 'xz'
         pt = [pt(1),udata.curslice,pt(2)];
         r1 = [pt(1) pt(1)+droi(1)];
         r3 = [pt(2) pt(2)+droi(2)];
         [r1,r3] = clip_rect(r1, r3, udata.imsize);
         imline = squeeze(double(udata.slices(r1(1):r1(2) , :,  r3(1):r3(2))));
         ttl = [num2str(r1(1)) ':' num2str(r1(2)) ',:,' num2str(r3(1)) ':' num2str(r3(2))];
        if yroi
            hrect = rectangle('Position', [pt(1) pt(2) droi]);
        end
       otherwise % case 'xy'
         pt = [pt,udata.curslice];
         r1 = [pt(1) pt(1)+droi(1)];
         r2 = [pt(2) pt(2)+droi(2)];
         [r1,r2] = clip_rect(r1, r2, udata.imsize);
         imline = squeeze(double(sum(udata.slices(r1(1):r1(2), r2(1):r2(2), : ), [], [1:2]) ));
         ttl = [num2str(r1(1)) ':' num2str(r1(2)) ',' num2str(r2(1)) ':' num2str(r2(2)) ',:'];
         if yroi
             hrect = rectangle('Position', [pt(1) pt(2) droi]);
         end
   end
   if yroi; set(hrect,'EdgeColor',[0.8 0.8 0]); end
   if ~ishandle(h)
       h = figure;
       h = axes('parent',h);
   end
   plot(h,[0:length(imline)-1],imline);
   xlabel(h,'Stack index');
   if any(droi>1)
      ylabel(h,'Sum intensity');
      title(h,['Sum intensity over (',ttl,')']);
   else
      ylabel(h,'Intensity');
      title(h,['Intensity at (',ttl,')']);
   end
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

if clwin
   close(get(h,'parent'));
end

% Generate output
if isempty(imline)
   error('No point selected');
else
   if nargout~=0
      out = dip_image(imline);
      if any(droi>1)
          coords = [pt droi];
      else
          coords = pt;
      end
   end
end


function [r2,r3] = clip_rect(r2, r3, imsize);
r2(r2<0) = 0;
r3(r3<0) = 0;
if r2(2)>imsize(1)-1
    r2(2) = imsize(1)-1;
end
if r3(2)>imsize(2)-1
    r3(2) = imsize(2)-1;
end
