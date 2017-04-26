%DIPTRUESIZE   Sets the size of a display to its natural size
%   DIPTRUESIZE(H,PERC) sets the size of the image in the figure
%   window with handle H to a percentage of the original. PERC
%   can be both larger and smaller than 100.
%
%   DIPTRUESIZE(H,'MAX') chooses the percentage to maximize the
%   window size.
%
%   DIPTRUESIZE(H,'OFF') causes the axes of the image in H to fill
%   the figure window. The aspect ratio is lost, but the figure
%   window is not moved nor resized.
%
%   DIPTRUESIZE(H,'TIGHT') sets the aspect ratio of the image to 1:1,
%   adjusting the window to wrap it tightly, so that there is no
%   margin. This is useful when copying or saving an image through
%   the File menu of its display window.
%   DIPTRUESIZE(H,'TIGHT',PERC) additionally sets a zoom percentage.
%
%   The H can be left out of all syntaxes, in which case the
%   current figure window will be addressed. PERC cannot be left
%   out.
%
%   See also DIPSHOW, DIPZOOM.

%   Undocumented:
%   DIPTRUESIZE(H,'INITIAL') sets the percentage to 100% unless the
%   image is larger than the screen, in which case it uses 'MAX'.

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

function diptruesize(varargin)

% Parse input
state = 1;
tight = 0;
initial = 0;
ii = 1;
if nargin > 1 && ~ischar(varargin{1})
   try
      fig = getfigh(varargin{1});
   catch
      error('Argument must be a valid figure handle.')
   end
   ii = 2;
else
   fig = get(0,'CurrentFigure');
   if isempty(fig)
      error('No figure window open to do operation on.')
   end
end
if nargin < ii
   zoom = 1;
else
   if ischar(varargin{ii})
      if strcmpi(varargin{ii},'off')
         state = 0;
      elseif strcmpi(varargin{ii},'tight')
         tight = 1;
         if nargin>ii
            ii = ii+1;
            if isnumeric(varargin{ii}) && numel(varargin{ii})==1
               zoom = varargin{ii}/100;
            else
               error('Percentage expected.')
            end
         else
            zoom = 1;
         end
      elseif strcmpi(varargin{ii},'max')
         zoom = 0;
      elseif strcmpi(varargin{ii},'initial')
         initial = 1;
      else
         error('Illegal mode string.')
      end
   elseif isnumeric(varargin{ii}) && numel(varargin{ii})==1
      zoom = varargin{ii}/100;
   else
      error('Percentage expected.')
   end
end
ii = ii+1;
if nargin>=ii
   error('Too many arguments in call to DIPTRUESIZE.')
end

% Get handles
if ~strncmp(get(fig,'Tag'),'DIP_Image',9)
   return
end
udata = get(fig,'UserData');

% Get border and screen size
if state
   set(fig,'Units','pixels');
   rootUnits = get(0,'Units');
   set(0,'Units','pixels');
   scrsz = get(0,'ScreenSize');
   border = dipfig_getbordersize(fig);
   scrsz = scrsz(3:4)-[border(1)+border(3),border(2)+border(4)];
   set(0,'Units',rootUnits);
end

if length(udata.imsize)==1

   %%% 1D truesize %%%

   ax = findobj(fig,'Type','axes');
   if length(ax) ~= 1
      return
   end

   % Reset image view
   set(ax,'XLim',[0,udata.imsize]-0.5);

   if state

      % Calculate figure size.
      if initial
         if udata.imsize > scrsz(1)
            zoom = 0;
         else
            zoom = 1;
         end
      end
      if zoom==0
         imsize = scrsz(1);
         zoom = imsize/udata.imsize;
      else
         imsize = min(scrsz(1),round(udata.imsize*zoom));
      end
      if tight
         newimsz = imsize;
      else
         newimsz = max(imsize,dipgetpref('DefaultFigureWidth'));
      end

      udata.zoom = zoom;
      set(fig,'UserData',[]);    % Solve MATLAB bug!
      set(fig,'UserData',udata);

      % Set the position of the figure & axes
      set(fig,'Units','pixels');
      figPos = get(fig,'Position');
      if figPos(3) == newimsz
         % We're not resizing; call the callback manually.
         f = get(fig,'ResizeFcn');
         f(fig);
      else
         % We're resizing; the callback will be called automatically.
         figPos(3) = newimsz;
         set(fig,'Position',figPos);
      end

   else

      % Turn TRUESIZE off. The image will cover the whole figure window
      set(ax,'Units','normalized','Position',[0,0,1,1]);
      udata.zoom = [];
      set(fig,'UserData',[]);    % Solve MATLAB bug!
      set(fig,'UserData',udata);

   end

else

   %%% 2D/3D truesize %%%

   img = findobj(fig,'Type','image');
   if length(img) ~= 1
      return
   end
   ax = get(img,'Parent');

   % Reset image view
   set(ax,'XLim',[0,udata.imsize(1)]-0.5,'Ylim',[0,udata.imsize(2)]-0.5);

   if state

      % Calculate figure and axes sizes.
      if initial
         if any(udata.imsize(1:2) > scrsz)
            zoom = 0;
         else
            zoom = 1;
         end
      end
      if zoom==0
         zoom = min(scrsz./udata.imsize(1:2));
      end
      imsize = round(udata.imsize(1:2)*zoom);
      if any(imsize>scrsz)
         imsize = min(scrsz,imsize);
      end
      if tight
         newimsz = imsize;
      else
         newimsz(1) = max(imsize(1),dipgetpref('DefaultFigureWidth'));
         newimsz(2) = max(imsize(2),dipgetpref('DefaultFigureHeight'));
      end

      udata.zoom = zoom;
      set(fig,'UserData',[]);    % Solve MATLAB bug!
      set(fig,'UserData',udata);

      % Set the position of the figure & axes
      set(fig,'Units','pixels');
      figPos = get(fig,'Position');
      if figPos(3:4) == newimsz
         % We're not resizing; call the callback manually.
         f = get(fig,'ResizeFcn');
         f(fig);
      else
         % We're resizing; the callback will be called automatically.
         figPos(2) = max(border(2),figPos(2)+figPos(4)-newimsz(2));
         figPos(3:4) = newimsz;
         if figPos(1)+figPos(3) > scrsz(1)+border(1)
            figPos(1) = max(border(1),scrsz(1)+border(1)-figPos(3));
         end
         set(fig,'Position',figPos);
      end

      % Update linked displays
      if ~isempty(udata.linkdisplay)
         dipshow(fig,'updatelinked',[]);
      end

   else

      % Turn TRUESIZE off. The image will cover the whole figure window
      set(ax,'Units','normalized','Position',[0,0,1,1]);
      udata.zoom = [];
      set(fig,'UserData',[]);    % Solve MATLAB bug!
      set(fig,'UserData',udata);

   end

end

dipfig_titlebar(fig,udata);
