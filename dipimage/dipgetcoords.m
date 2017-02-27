%DIPGETCOORDS   Interactive coordinate extraction
%   V = DIPGETCOORDS(N) returns coordinates of N pixels, to be
%   selected interactively with the mouse, in the current image.
%   N default to 1.
%
%   Right clicking in the image returns [-1 -1] as coordinates at any time.
%
%   V = DIPGETCOORDS(H,N) returns coordinates selected from
%   figure window with handle H. N cannot be ommited.
%
%   It is still possible to use all the menus in the victim figure
%   window, but you won't be able to access any of the tools (like
%   zooming and testing). You can, however, step through a 3D/4D
%   image by using the keyboard.
%
%   Note: If you feel the need to interrupt this function with Ctrl-C,
%   you will need to refresh the display (by re-displaying the image
%   or changing the 'Actions' state).
%
%   See also DIPSHOW, DIPTEST, DIPCROP, DIPPROFILE.

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

function coords = dipgetcoords(arg1,arg2)

% Parse input
if nargin == 0
   N = 1;
   fig = get(0,'CurrentFigure');
   if isempty(fig)
      error('No figure window open to do operation on.')
   end
elseif nargin == 1
   N = arg1;
   fig = get(0,'CurrentFigure');
   if isempty(fig)
      error('No figure window open to do operation on.')
   end
else % nargin == 2
   try
      fig = getfigh(arg1);
   catch
      error('Argument must be a valid figure handle.')
   end
   N = arg2;
end
if prod(size(N))~=1 || ~isnumeric(N) || fix(N)~=N || N<1
   error('N must be a positive integer.');
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
coords = [];
done =0;
ii=0;
while ~done
%for ii=1:N
   pt = [];
   while isempty(pt)
      set(fig,'WindowButtonDownFcn','set(gcbf,''WindowButtonDownFcn'',''Click!'')');
      waitfor(fig,'WindowButtonDownFcn'); % The ButtonDown callback changes the callback.
                                          % This way, we also detect a change in state!
      if ~ishandle(fig)
         error('You closed the window! That wasn''t the deal!')
      end
      udata = get(fig,'UserData');
      if length(udata.imsize)==3
         switch udata.slicing
            case 'yz'
               slicepos = 1;
            case 'xz'
               slicepos = 2;
            otherwise % case 'xy'
               slicepos = 3;
         end
      elseif length(udata.imsize)==4
         switch udata.slicing
            case 'yz'
               slicepos = 11;
            case 'xz'
               slicepos = 12;
            case 'xt'
               slicepos = 13;
            case 'yt'
               slicepos = 14;
            case 'zt'
               slicepos = 15;
            otherwise % case 'xy'
               slicepos = 16;
         end
      elseif length(udata.imsize)==1
         slicepos = -1; % meaning the image is 1D -> only keep first coordinate
      else
         slicepos = 0; % meaning no slice info must be added
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
      else
           switch get(fig,'SelectionType');
               case 'normal' %left click
                    pt = dipfig_getcurpos(ax);
               case 'alt' %right
                    pt= repmat(-1,1,length(udata.imsize));
                    done =1;
                    slicepos = 0;
           end
       end
   end
   switch slicepos
      case -1
         pt = pt(1);
      case 0
      case 1
         udata = get(fig,'UserData');
         pt = [udata.curslice,pt];
      case 2
         udata = get(fig,'UserData');
         pt = [pt(1),udata.curslice,pt(2)];
      case 3
         udata = get(fig,'UserData');
         pt = [pt,udata.curslice];
      case 11 %yz
         udata = get(fig,'UserData');
         pt = [udata.curslice,pt, udata.curtime];
      case 12 %xz
         udata = get(fig,'UserData');
         pt = [pt(1),udata.curslice,pt(2), udata.curtime];
      case 13 %xt
         udata = get(fig,'UserData');
         pt = [pt(1),udata.curslice,udata.curtime,pt(2)];
      case 14 %yt
         udata = get(fig,'UserData');
         pt = [udata.curslice,pt(1),udata.curtime,pt(2)];
      case 15 %zt
         udata = get(fig,'UserData');
         pt = [udata.curslice,udata.curtime,pt];   
      case 16 %xy
         udata = get(fig,'UserData');
         pt = [pt,udata.curslice, udata.curtime];
   end
   coords = [coords;pt];
   ii = ii +1;
   if ii == N
       done =1;
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
