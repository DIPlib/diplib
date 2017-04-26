%DIPSHOW   Shows a dip_image object as an image
%   DIPSHOW(B) displays the image in B. To which figure window and in what
%   mode depends on the current default settings, which can be specified
%   through DIPSETPREF and DIPFIG. DIPSHOW works for 1, 2, 3 and 4 dimensional
%   grey-value images, as well as 2D color images.
%
%   DIPSHOW(B,[MIN MAX]) displays the image in B as a grey-value image in
%   the range [MIN MAX]. MIN is shown black, MAX is shown white. For color
%   images, each channel is stretched in the same way.
%
%   DIPSHOW(B,RANGESTR), with RANGESTR a string, provides shortcuts for the
%   range selection. The following list shows the posibilities:
%      'unit'              [0 1]
%      'normal' or '8bit'  [0 255]
%      '12bit'             [0 4095]
%      '16bit'             [0 65535]
%      's8bit'             [-128 127]
%      's12bit'            [-2048 2047]
%      's16bit'            [-32768 32767]
%      'lin' or 'all'      [MIN(B) MAX(B)]
%      'percentile'        [PERCENTILE(B,5) PERCENTILE(B,95)]
%      'base'              [MIN([MIN(B),-MAX(B)]) MAX([-MIN(B),MAX(B)])]
%      'angle'             [-PI PI]
%      'orientation'       [-PI/2 PI/2]
%   These modes additionally set the colormap to 'grey', except for the
%   'angle' and 'orientation' modes, which set the colormap to 'periodic',
%   and the 'base' mode, which sets the colormap to 'zerobased' (see below).
%
%   DIPSHOW(B,'log') displays the image in B using logarithmic stretching.
%   The colormap is set to 'grey'.
%
%   DIPSHOW(...,COLMAP) set the colormap to a custom colormap COLMAP. See the
%   function COLORMAP for more information on colormaps. Additionally, these
%   strings choose one of the default color maps:
%      'grey'         grey-value colormap
%      'periodic'     periodic colormap for angle display
%      'saturation'   grey-value colormap with saturated pixels in red and
%                     underexposed pixels in blue.
%      'zerobased'    The 50% value is shown in grey, with lower values
%                     increasingly saturated and lighter blue, and higher
%                     values increasingly saturated and lighter yellow.
%   When combining a COLMAP and a RANGESTR parameter, the COLMAP overrules
%   the default color map belonging to RANGESTR.
%
%   DIPSHOW(B,'labels') displays the image in B as a labeled image. This
%   mode cannot be combined with a RANGESTR or a COLMAP parameter.
%
%   DIPSHOW(...,TSMODE) can be used to overrule the default 'TrueSize'
%   setting. If TSMODE is 'truesize', TRUESIZE will be called after
%   displaying the image. IF TSMODE is 'notruesize', TRUESIZE will not be
%   called.
%
%   DIPSHOW(H,B,...) displays the image in B to the figure window with
%   handle H, overruling any settings made for variable B. H must be a valid
%   figure handle, or an integer value that will be used to create a new
%   window. If H is 0, a new window will be created.
%
%   H = DIPSHOW(...) returns the handle to the figure window used.
%
%   For more info on the figure windows created, see the DIPimage User Guide.
%
%   See also DIPSETPREF, DIPFIG, DIPTRUESIZE, DIPMAPPING, DIPZOOM, DIPTEST,
%   DIPORIEN, DIPLINK, DIPSTEP.

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

% UNDOCUMENTED
%
%   DIPSHOW(...,'name',NAME) causes the figure window's title to be set
%   to NAME instead of INPUTNAME(IN). This is used by DIP_IMAGE/DISPLAY.
%
%   DIPSHOW(...,'position',POS) causes the figure's 'Position' property
%   to be set to POS. POS must be a 1x4 array with [X Y H W] or a 1x2 array
%   with [W H]. In the last case, the X and Y position is not changed.
%   This disables the truesize setting.
%
%   DIPSHOW(H,'updatelinked',[]) causes the figures liked with H to be
%   updated.
%
%   If IN is [], the image is made black, and the zoom factor is not shown.
%   This is used by DIPFIG, together with the 'position' option, to create
%   a figure window.
%
%      DIPSHOW(H,'ch_mappingmode',RANGE)
%      DIPSHOW(H,'ch_mappingmode',MAPPINGMODE)
%   RANGE or MAPPINGMODE are as defined above for regular DIPSHOW syntax.
%      DIPSHOW(H,'ch_colormap', COLORMAP)
%   COLORMAP is 'grey', 'periodic', 'saturation', 'labels' or a colormap.
%      DIPSHOW(H,'ch_globalstretch',BOOLEAN)
%   BOOLEAN is 'yes', 'no', 'on', 'off', 1, 0, true or false.
%      DIPSHOW(H,'ch_complexmapping',COMPLEXMAP)
%   COMPLEXMAP is one of: 'magnitude', 'abs', 'real', 'imag', 'phase'.
%      DIPSHOW(H,'ch_slicing',SLICING)
%   SLICING is one of: 'xy', 'xz', 'yz', 'xt', 'yt', 'zt'.
%      DIPSHOW(H,'ch_slicemode',SLICEMODE)
%   SLICEMODE is one of: 'slice', 'max', 'mean'.
%      DIPSHOW(H,'ch_slice',SLICE), DIPSHOW(H,'ch_time',SLICE)
%   SLICE is a slice number.
%   H is a handle to a figure window, and is optional. (although sometimes...)
%   These cause the image to be re-displayed using the requested mode, and
%   are used as callback for the 'Mappings' menu items. They can be joined
%   in a single command:
%      DIPSHOW('ch_mappingmode','lin','ch_complexmapping','abs').

% ORGANIZATION
%
% The figure window created contains:
% - A Tag, composed of the string 'DIP_Image' and one of each of these:
%       - '_1D', '_2D', '_3D' or '_4D'.
%       - '_Binary', '_Grey', '_Color'.
% - A group of Uimenu objects, defined in CREATE_MENUS.
% - An Axes object with:
%   - An Image object.
% - A UserData object with:
%   - For all images:
%      - udata.handle         -> dip_imagedisplay handle.
%      - udata.imsize         -> Size of the display in pixels ([x] for 1D images, [x,y] for others).
%      - udata.colmap         -> String representing the colormap: 'grey', 'periodic', 'saturation',
%                                'zerobased', 'labels', 'custom'.
%      - udata.colspace       -> Name of color space: '' (for grey), 'RGB', 'Lab', 'XYZ', etc.
%      - udata.zoom           -> Pixel size or [] if aspect ratio is not 1:1. If 0, don't show zoom!
%      - udata.state          -> Action state: 'none', 'dipstep', 'diptest', 'dipzoom', etc.
%      - udata.imname         -> Name to be displayed in title bar.
%   - For 1D images:
%      - udata.imagedata      -> input data, used to recompute the display. In case of 1D images,
%                                udata.handle is used only to manage the display settings.
%   - For 3/4D images:
%      - udata.linkdisplay    -> If linked, list of figure handles, else [].
%   - In function KeyPressFcn:
%      - udata.nextslice      -> String being typed by user containing a number.
%      - udata.lastkeypress   -> String containing last keypress
%   - If DIPSTEP has been enabled:
%      - udata.prevclick      -> Previous direction (1,0,-1). When double clicking, first
%                                a single click is detected, then the double click. However,
%                                we don't know with which button was double-clicked. Using
%                                this variable, the second click repeats the first one.
%   - If DIPORIEN has been enabled:
%      - udata.orientationim  -> dip_image object with orientation image.

% TODO: use image pixel size to determine aspect ratio, and use aspect ratio to scale axes.

% COMMON PHRASES
%
% Tests using the Tag property:
%  Is created by DIPSHOW: strncmp(tag,'DIP_Image',9)
%  Is grey:               length(tag)==17 && tag(14:17)=='Grey')
%  Is color:              length(tag)==18 && tag(14:18)=='Color')
%  Is binary:             length(tag)==19 && tag(14:19)=='Binary')
%  Dimensionality:        str2num(tag(11))                        ONLY if created by dipshow.
%  Is a 3D image:         strncmp(tag,'DIP_Image_3D',12)
%  etc.

function h = dipshow(varargin)

% ORGANIZATION
% These are the functions in this file, in the order in which they appear:
%
%   get_default_mode              Called to get the mode in which to display an image
%   parse_rangestr                Parse the input parameter RANGE
%   parse_colmapstr               Parse the input parameter COLORMAP
%   dipshow_1D                    Core for 1D images
%   display_data_1D               Send 1D image to display
%   stretchYaxis_1D               Adjust display mode for 1D image
%   mapcomplexdata                Map complex data to the real domain
%   dipshow_core                  Core for 2D and 3D images
%   change_slice                  Change the slice to be shown, does not update the display
%   update_display                Update the 2D display
%   update_linked                 Update the slice in linked displays
%   clearnreset                   Delete all figure children and reset properties to DIPSHOW defaults
%   create_menus                  Create menus for figure window (contains callback definitions)
%   find_action_state             Find the correct action state for the current image
%   change_action_state           Set the action state
%   set_mode_check                Set the checkmarks in the Mappings menu
%   set_global_check              Set the checkmark to the global stretch menu item
%   change_mapping                Set the mapping modes (does some user-input parsing too)
%   change_keyboard_state         Enable or disable the keyboard callback for a figure window
%   save_figure_window            Callback for the "File->Save..." menu item
%   dipzoomWindowButtonDownFcn    (callback, DIPZOOM)
%   dipzoomWindowButtonMotionFcn  (callback, DIPZOOM)
%   dipzoomWindowButtonUpFcn      (callback, DIPZOOM)
%   dipzoomUpdateDisplay          Update the title bar while dragging the zoom tool
%   dipzoomConstrain              Compute a constrained rectangle
%   dipzoomZoom                   Execute the actual zoom (for both ButtonUp and KeyPress Fcns)
%   dipstepWindowButtonDownFcn    (callback, DIPSTEP)
%   dipstepWindowButtonMotionFcn  (callback, DIPSTEP)
%   dipstepWindowButtonUpFcn      (callback, DIPSTEP)
%   KeyPressFcn                   (callback)
%   ResizeFcn                     (callback)
%   position_axes                 Puts the axes in the right location
%   default_figure_size           Callback for "Sizes->Default window size" menu item


% We need to handle some "callbacks"
if nargin == 2 && ischar(varargin{1}) && ischar(varargin{2}) && strcmp(varargin{1},'DIP_callback')
   switch varargin{2}
      case 'dipzoomWindowButtonDownFcn'
         h = @dipzoomWindowButtonDownFcn;
      case 'dipstepWindowButtonDownFcn'
         h = @dipstepWindowButtonDownFcn;
      case 'clear'
         close all
         imagedisplay('unlock')
         clear imagedisplay
      otherwise
         error('Unknown callback function.')
   end
   return
end

% Parse input
hasrange = 0;     % True if range has been specified on the command line.
hascolmap = 0;    % True if colmap has been specified on the command line.
truesz = [];      % Set to 1 or 0 by an input parameter.
dontshowzoom = 0; % Set to 1 in a special (undocumented) case.
fig = [];
position = [];
n = 1;
if nargin >= n
   if ischar(varargin{n})
      if strcmpi(varargin{n},'other')
         error('String ''other'' no allowed as window handle');
      end
      fig = dipfig('-get',varargin{n});
      n = n+1;
   elseif isnumeric(varargin{n}) && length(varargin{n})==1
      fig = double(varargin{n});
      if ~isfigh(fig) && fix(fig) ~= fig
         error('Argument must be a valid figure handle.')
      end
      n = n+1;
   elseif numel(varargin{n})==1 && ishandle(varargin{n})
      fig = varargin{n};
      if ~isfigh(fig)
         error('Argument must be a valid figure handle.')
      end
      n = n+1;
   end
end
if nargin >= n
   in = varargin{n};
   if ischar(in)
      if isempty(fig)
         fig = get(0,'CurrentFigure');
         if isempty(fig)
            error('No figure window open to do operation on.')
         end
         if ~isnumeric(fig)
            fig = fig.Number;
         end
      else
         if ~isfigh(fig)
            error('Argument must be a valid figure handle.')
         end
      end
      if strcmp(in,'updatelinked')
         udata = get(fig,'UserData');
         handle = udata.handle;
         coords = imagedisplay(handle,'coordinates');
         slicing = imagedisplay(handle,'slicing');
         udata.linkdisplay = update_linked(fig,udata.linkdisplay,coords,slicing,udata.zoom);
         set(fig,'UserData',udata)
      else
         change_mapping(fig,varargin{n:end});
      end
      return
   end
   if ~isnumeric(in) || ~isempty(in)
      if ~isa(in,'dip_image')
         in = dip_image(in);
      end
      in = squeeze(in);
      if isempty(in)
         error('DIPSHOW cannot handle empty images.')
      elseif ndims(in)>4
         error('DIPSHOW cannot handle images with more than 4 dimensions.')
      end
      % -- handle expectation to display 0D images (useful for color) BR on request of PV
      if ndims(in)==0
         in = reshape(in,[1,1]);
      end
   % else it's OK, create empty display later on.
   end
   imname = inputname(n);
   n = n+1;
else
   error('Input image required.')
end
while nargin >= n
   tmp = varargin{n};
   n = n+1;
   if ischar(tmp)
      switch lower(tmp)
         case 'name'
            if nargin < n, error('DIPSHOW requires more arguments.'); end
            imname = varargin{n};
            if ~ischar(imname), error('Name should be a string.'); end
            n = n+1;
         case 'truesize'
            if ~isempty(truesz), error('DIPSHOW has too many arguments.'); end
            truesz = 1;
         case 'notruesize'
            if ~isempty(truesz), error('DIPSHOW has too many arguments.'); end
            truesz = 0;
         case 'position'
            if nargin < n, error('DIPSHOW requires more arguments.'); end
            if ~isempty(position), error('DIPSHOW has too many arguments.'); end
            position = varargin{n};
            if ~isnumeric(position) || all(numel(position)~=[2,4])
               error('Position should be an array with 2 or 4 numbers.');
            end
            position = position(:)';
            n = n+1;
         case {'normal','unit','lin','all','percentile','angle','orientation','base','log','8bit','12bit','16bit','u8bit','u12bit','u16bit','s8bit','s12bit','s16bit'}
            if hasrange, error('Only one stretching mode allowed on the command line.'); end
            rangestr = tmp;
            hasrange = 1;
         case {'grey','gray','saturation','zerobased','periodic'}
            if hascolmap, error('Only one colormap allowed on the command line.'); end
            colmapstr = tmp;
            hascolmap = 1;
         case 'labels'
            if hasrange, error('Only one stretching mode allowed on the command line.'); end
            if hascolmap, error('Only one colormap allowed on the command line.'); end
            rangestr = tmp;
            hasrange = 1;
            colmapstr = tmp;
            hascolmap = 1;
         otherwise
            error(['Illegal option ''',tmp,''' to DIPSHOW used.'])
      end
   else
      if ~isnumeric(tmp), error('DIPSHOW found an unexpected input argument.'); end
      if isequal(size(tmp),[1,2]) || isempty(tmp)
         if hasrange, error('DIPSHOW has too many RANGE arguments.'); end
         rangestr = tmp;
         hasrange = 1;
      elseif size(tmp,2)==3
         if hascolmap, error('DIPSHOW has too many COLMAP arguments.'); end
         colmapstr = tmp;
         hascolmap = 1;
      else
         error('Illegal parameter in call to DIPSHOW.');
      end
   end
end
fixposition = 0;
if ~isempty(position)
   truesz = 0;
   dontshowzoom = 1;
   fixposition = 1;
else
   if isempty(truesz)
      truesz = dipgetpref('TrueSize');
   end
end

% Find figure window & bring to front
if isempty(fig)
   % Look for the figure handle to use based on name.
   fig = dipfig('-get',imname);
end
newfig = 0;
if (fig == 0)
   if dipgetpref('RespectVisibility')
      fig = figure;
   else
      fig = figure('Visible','off');
   end
   newfig = 1;
else
   try
      % 27-10-2006 MvG -- Trying to get rid of this bloody behaviour where
      % the window is brought to the front on every display. It seems to be
      % due to design *AND* matlab bugs. On wintendo it's even worse since
      % bringing a window to the front implies that it takes the focus!!!
      % Try running batch jobs with this going on...
      if ~ishandle(fig)
         fig = figure(fig);
         if ~dipgetpref('RespectVisibility')
            set(fig,'Visible','off');
         end
         newfig = 1;
      elseif dipgetpref('BringToFrontOnDisplay')
         if dipgetpref('RespectVisibility')
            vis = get(fig,'Visible');
            figure(fig);
            set(fig,'Visible',vis); % figure(fig) will make the figure visible.
         else
            figure(fig);
            set(fig,'Visible','off');
         end
      else
         set(0,'CurrentFigure',fig);
         % Don't turn off visibility because that might bring the window to the front.
      end
   catch
      error('Argument must be a valid figure handle.');
   end
end
if isempty(position) && newfig
   % Use default figure size
   position = [dipgetpref('DefaultFigureWidth'),dipgetpref('DefaultFigureHeight')];
end
if ~isempty(position)
   if length(position)==2
      pos = get(fig,'position');
      top = pos(2)+pos(4);
      position = [pos(1),top-position(2),position];
   end
   set(fig,'position',position);
end

% Select display 'mode'
[currange,mappingmode,colmap,colmapdata,complexmapping,slicing,state,keys] = get_default_mode(fig);
if hasrange
   [currange,mappingmode,newcolmap] = parse_rangestr(rangestr);
   if ischar(newcolmap) && ~strcmp(newcolmap,'custom') && ~strcmp(newcolmap,colmap)
      [colmap,colmapdata] = parse_colmapstr(newcolmap);
   end
end
if hascolmap
   [colmap,colmapdata] = parse_colmapstr(colmapstr);
end

if isnumeric(in) && isempty(in)
   % Maybe we don't want to display any image
   dipshow_core(fig,dip_image(0),imname,currange,mappingmode,colmap,colmapdata,complexmapping,slicing,state,keys,0,1);
else
   % Display images
   if ndims(in)==1
      dipshow_1D(fig,in,imname,currange,mappingmode,complexmapping,state,keys,truesz,dontshowzoom);
   else
      dipshow_core(fig,in,imname,currange,mappingmode,colmap,colmapdata,complexmapping,slicing,state,keys,truesz,dontshowzoom);
   end
end
% Solve bug with positioning windows close to the top screen border on newer MATLABs with Java enabled.
if fixposition && length(position)==4
   set(fig,'position',position);
end
%
if nargout > 0
   if isnumeric(fig) && useshg2
      % Return handle to figure window, not an integer as we've been using here.
      fig = findobj(0,'Number',fig);
   end
   h = fig;
end


%
% The default is the current values for the figure
%
function [currange,mappingmode,colmap,colmapdata,complexmapping,slicing,state,keys] = get_default_mode(fig)
% defaults
mappingmode = dipgetpref('DefaultMappingMode');
currange = [];
colmap = dipgetpref('DefaultColorMap');
colmapdata = [];
complexmapping = dipgetpref('DefaultComplexMapping');
slicing = parse_slicing(dipgetpref('DefaultSlicing'));
state = dipgetpref('DefaultActionState');
if dipgetpref('EnableKeyboard')
   keys = 'on';
else
   keys = 'off';
end
% change defaults according to current settings
if ishandle(fig)
   udata = get(fig,'UserData');
   if isfield(udata,'handle')
      handle = udata.handle;
      mappingmode = imagedisplay(handle,'mappingmode');
      if strcmp(mappingmode,'manual')
         currange = imagedisplay(handle,'range');
      end
      complexmapping = imagedisplay(handle,'complexmapping');
   end
   if isfield(udata,'state')
      state = udata.state;
   end
   if isfield(udata,'colmap')
      colmap = udata.colmap;
      switch colmap
         case 'custom'
            colmapdata = get(fig,'colormap');
         case 'periodic'
            colmapdata = nicelut;
         case 'saturation'
            colmapdata = saturation_colormap;
         case 'zerobased'
            colmapdata = zerobased_colormap;
         case 'labels'
            colmapdata = label_colormap;
         otherwise
            colmapdata = gray(256);
      end
   end
end


%
% Parse the input parameter RANGE
%
function [currange,mappingmode,colmap] = parse_rangestr(currange)
colmap = 'grey';
if ischar(currange)
   mappingmode = lower(currange);
   currange = [];
   switch mappingmode
      case {'base','based'}
         colmap = 'zerobased';
      case 'labels'
         mappingmode = 'normal';
         colmap = 'labels';
   end
else
   if isempty(currange)
      mappingmode = 'lin';
   elseif length(currange) ~= 2
      error('Illegal range argument.')
   else
      mappingmode = 'manual';
      if (currange(1) > currange(2))
         tmp = currange(1);
         currange(1) = currange(2);
         currange(2) = tmp;
      end
   end
end


%
% Parse the input parameter COLORMAP
%
function [colmap,colmapdata] = parse_colmapstr(colmap)
if ischar(colmap)
   colmap = lower(colmap);
   switch colmap
      case {'grey','gray'}
         colmap = 'grey';
         colmapdata = gray(256);
      case 'saturation'
         colmapdata = saturation_colormap;
      case 'zerobased'
         colmapdata = zerobased_colormap;
      case 'periodic'
         colmapdata = nicelut;
      case 'labels'
         colmapdata = label_colormap;
      otherwise
         error(['Illegal argument ''',colmap,'''.'])
   end
else
   if isempty(colmap)
      colmap = 'grey';
      colmapdata = gray(256);
   elseif size(colmap,2) ~= 3
      error('Illegal range argument.')
   else
      colmapdata = colmap;
      colmap = 'custom';
   end
end


%
% Parse the input parameter SLICING
%
function slicing = parse_slicing(str)
if ischar(str)
   if numel(str) ~= 2
      error('Illegal slicing argument.')
   end
   slicing = [get__slicing__dim(str(1)), get__slicing__dim(str(2))];
   if slicing(1)==slicing(2)
      error('Illegal slicing argument.')
   end
else
   slicing = str;
   if numel(slicing) ~= 2 || ~isnumeric(slicing)
      error('Illegal slicing argument.')
   end
end

function n = get__slicing__dim(c)
switch c
   case 'x'
      n = 1;
   case 'y'
      n = 2;
   case 'z'
      n = 3;
   otherwise % case 't'
      n = 4;
end


%
% DIPSHOW core for 1D images.
%
function dipshow_1D(fig,in,imname,currange,mappingmode,complexmapping,state,keys,truesz,dontshowzoom)
% Check image type
sz = imsize(in);
if length(sz)~=1, error('Unexplicable error: data is not 1D in dipshow_1D.'); end
iscol = iscolor(in);
if iscol
   isbin = 0;
else
   isbin = islogical(in);
end
iscomp = iscomplex(in);
% Set up figure window.
clearnreset(fig);
bgcol = get(fig,'Color');
axh = axes('Parent',fig,'Visible','on','XGrid','off','YGrid','off',...
   'XTick',[],'YTick',[],'PlotBoxAspectRatioMode','auto','Units','normalized',...
   'Position',[0 0 1 1],'Xcolor',bgcol,'Ycolor',bgcol);
create_menus(fig,1,iscomp,iscol,isbin);
change_keyboard_state(fig,keys);
% Create UserData stuff
udata.imagedata = in;
udata.imsize = imsize(in);
handle = imagedisplay(in);
udata.handle = handle;
imagedisplay(handle,'complexmapping',complexmapping)
if strcmp(mappingmode,'manual')
   imagedisplay(handle,'mappingmode',currange)
else
   imagedisplay(handle,'mappingmode',mappingmode)
end
udata.state = ''; % Make sure the state is updated later on.
udata.colspace = colorspace(in);
udata.colmap = '';
udata.linkdisplay = [];
% Display 1D image
udata = display_data_1D(axh,udata);
set(axh,'XLim',[0,sz]-0.5);
% Set figure properties
set_mode_check(fig,imagedisplay(handle,'mappingmode'),'',imagedisplay(handle,'complexmapping'),[]);
if dontshowzoom
   udata.zoom = 0;
else
   udata.zoom = [];
end
if ~isempty(imname)
   udata.imname = imname;
else
   udata.imname = '';
end
if iscol
   udata.imname = [udata.imname,' (',udata.colspace,')'];
end
tag = 'DIP_Image_1D';
if iscol
   tag = [tag,'_Color'];
elseif isbin
   tag = [tag,'_Binary'];
else
   tag = [tag,'_Grey'];
end
set(fig,'Tag',tag,'UserData',[]);   % Solve MATLAB bug!
set(fig,'UserData',udata);
if truesz
   diptruesize(fig,'initial');
else
   dipfig_titlebar(fig,udata);
end
newstate = find_action_state(state,1,iscomp,iscol);
if ~strcmp(state,newstate)
   % Current state is not compatible with display. Try default state:
   state = dipgetpref('DefaultActionState');
   newstate = find_action_state(state,1,iscomp,iscol);
end
change_action_state(fig,newstate);
% 27-10-2006 MvG -- the 'visible' 'off'>'on' cycle brings the window to
% the front! Worse: even if the figure *IS* on, set('visible','on')
% *SOMETIMES* still brings the window to the front!
% With RespectVisibility the windows will actually be turned off by
% dipshow unless BringToFrontOnDisplay is 'off' -- my bad, but does not
% affect the default settings.
if strcmp(get(fig,'Visible'),'off') && ~dipgetpref('RespectVisibility')
   set(fig,'Visible','on');
end


%
% Calculate the display data for a 1D image
%
function udata = display_data_1D(axh,udata)
cdata = mapcomplexdata(udata.imagedata,imagedisplay(udata.handle,'complexmapping'));
sz = imsize(cdata);
delete(get(axh,'Children'));
cdata = squeeze(double(cdata))';
xdata = (0:sz)-0.5;
xdata = reshape(repmat(xdata,2,1),[1,sz*2+2]);
xdata = xdata(2:end-1);
colors = get(0,'DefaultAxesColorOrder');
for ii=1:size(cdata,2)
   ydata = reshape(repmat(cdata(:,ii),2,1),[1,sz*2]);
   jj = mod(ii-1,size(colors,1))+1;
   line('xdata',xdata,'ydata',ydata,'color',colors(jj,:),'linestyle','-');
end
udata = stretchYaxis_1D(axh,udata);


%
% Set the Y-axis properties for 1D image
%
function udata = stretchYaxis_1D(axh,udata)
handle = udata.handle;
complexmapping = imagedisplay(handle,'complexmapping');
cdata = mapcomplexdata(udata.imagedata,complexmapping);
logmode = 0;
if islogical(cdata)
   currange = [-0.2,1.2];
else
   mappingmode = imagedisplay(handle,'mappingmode');
   switch mappingmode
      case 'log'
         % won't work if any pixel is non-finite, nor if there are non-positive values
         if any(~isfinite(cdata))
            canlog = 'non-finite values';
         elseif any(cdata<0)
            canlog = 'negative values';
         else
            canlog = '';
         end
         if ~isempty(canlog)
            warning(['Image contains ',canlog,'. Cannot perform log stretch.'])
            currange = [0,255];
            imagedisplay(handle,'range',currange);
         else
            logmode = 1;
            currange = getmaximumandminimum(cdata);
         end
      case 'based'
         currange = getmaximumandminimum(cdata);
         currange(2) = max(abs(currange));
         currange(1) = -currange(2);
      case 'percentile'
         currange = [percentile(cdata,5),percentile(cdata,95)];
      case 'lin'
         currange = getmaximumandminimum(cdata);
      otherwise
         currange = imagedisplay(handle,'range');
   end
   if currange(1) == currange(2)
      currange = currange + [-1,1];
   elseif currange(1) > currange(2)
      currange = currange([2,1]);
   end
end
set(axh,'Ylim',currange);
if logmode
   set(axh,'YScale','log');
else
   set(axh,'YScale','lin');
end


%
% Map complex data to the real domain
%
function cdata = mapcomplexdata(cdata,complexmapping)
if ~isreal(cdata)
   switch complexmapping
      case 'real'
         cdata = real(cdata);
      case 'imag'
         cdata = imag(cdata);
      case 'phase'
         cdata = phase(cdata);
      otherwise
         cdata = abs(cdata);
   end
end


%
% DIPSHOW core for 2D, 3D and 4D images
%
function dipshow_core(fig,in,imname,currange,mappingmode,colmap,colmapdata,complexmapping,slicing,state,keys,truesz,dontshowzoom)
% Check image type
iscol = iscolor(in);
if iscol
   isbin = 0;
else
   isbin = islogical(in);
end
iscomp = iscomplex(in);
sz = imsize(in);
nD = length(sz);
% Get some data out of the figure window before resetting
linkdisplay=[];
coords = [];
if nD>=3
   udata = get(fig,'UserData');
   globalstretch = dipgetpref('DefaultGlobalStretch');
   if isfield(udata,'handle')
      handle = udata.handle;
      newcoords = imagedisplay(handle,'coordinates');
      if length(newcoords) == nD
         slicing = imagedisplay(handle,'slicing');
         globalstretch = imagedisplay(handle,'globalstretch');
         coords = newcoords;
      end
   end
   if isfield(udata,'linkdisplay')
      linkdisplay = udata.linkdisplay;
   end
   udata = [];
elseif nD==0
   nD = 2; % Default figure: nice menus and stuff.
   in.expanddim(2);
end
% Set up figure window.
clearnreset(fig);
axh = axes('Parent',fig,'Visible','off','XGrid','off','YGrid','off','YDir','reverse',...
   'PlotBoxAspectRatioMode','auto','Units','normalized','Position',[0 0 1 1]);
imh = image('BusyAction','cancel','Parent',axh,'Interruptible','off','CDataMapping','direct','CData',[]);
create_menus(fig,nD,iscomp,iscol,isbin);
change_keyboard_state(fig,keys);
% Create imagedisplay object, set properties
handle = imagedisplay(in);
if nD>=3
   imagedisplay(handle,'globalstretch',globalstretch)
   imagedisplay(handle,'slicing',slicing);
   if ~isempty(coords)
      imagedisplay(handle,'coordinates',coords);
   end
end
imagedisplay(handle,'complexmapping',complexmapping)
if strcmp(mappingmode,'manual')
   imagedisplay(handle,'mappingmode',currange)
else
   imagedisplay(handle,'mappingmode',mappingmode)
end
   %if iscol
      %g = dipgetpref('Gamma');
   %else
      %g = dipgetpref('GammaGrey');
   %end
   % TODO: use `g` in `imagedisplay`.
% Create UserData stuff
udata.state = ''; % Make sure the state is updated later on.
udata.handle = handle;
udata.colmap = colmap;
udata.colspace = colorspace(in);
udata.linkdisplay = linkdisplay;
udata = update_display(udata,imh,handle);
if isbin
   colmapdata = [0,0,0;dipgetpref('BinaryDisplayColor')];
else
   if isempty(colmapdata)
      [~,colmapdata] = parse_colmapstr(udata.colmap);
   end
end
set(fig,'Colormap',colmapdata);
% Set figure properties
set_mode_check(fig,imagedisplay(handle,'mappingmode'),udata.colmap,imagedisplay(handle,'complexmapping'),imagedisplay(handle,'slicing'));
if nD>=3
   set_global_check(fig,imagedisplay(handle,'globalstretch'));
   if ~isempty(udata.linkdisplay)
      set(findobj(fig,'Tag','diplink'),'Checked','on');
   end
end
if dontshowzoom
   udata.zoom = 0;
else
   udata.zoom = [];
end
if ~isempty(imname)
   udata.imname = imname;
else
   udata.imname = '';
end
if iscol
   udata.imname = [udata.imname,' (',udata.colspace,')'];
end
tag = ['DIP_Image_',num2str(nD),'D'];
if iscol
   tag = [tag,'_Color'];
elseif isbin
   tag = [tag,'_Binary'];
else
   tag = [tag,'_Grey'];
end
set(fig,'Tag',tag,'UserData',[]);   % Solve MATLAB bug!
set(fig,'UserData',udata);
if truesz
   diptruesize(fig,'initial');
else
   dipfig_titlebar(fig,udata);
end
newstate = find_action_state(state,nD,iscomp,iscol);
if ~strcmp(state,newstate)
   % Current state is not compatible with display. Try default state:
   state = dipgetpref('DefaultActionState');
   newstate = find_action_state(state,nD,iscomp,iscol);
end
change_action_state(fig,newstate);
% 27-10-2006 MvG -- the 'visible' 'off'>'on' cycle brings the window to
% the front! Worse: even if the figure *IS* on, set('visible','on')
% *SOMETIMES* still brings the window to the front!
% With RespectVisibility the windows will actually be turned off by
% dipshow unless BringToFrontOnDisplay is 'off' -- my bad, but does not
% affect the default settings.
if strcmp(get(fig,'Visible'),'off') && ~dipgetpref('RespectVisibility')
   set(fig,'Visible','on');
end


%
% Changes the current slice (for 2/3/4D images), doesn't update the display
%
function change_slice(udata,newslice)
handle = udata.handle;
sz = imagedisplay(handle,'sizes');
nD = length(sz);
if nD>2
   coords = imagedisplay(handle,'coordinates');
   k = imagedisplay(handle,'orthogonal');
   if newslice(1)>=0
      coords(k(1)) = newslice(1);
   end
   if nD>3
      if newslice(2)>=0
         coords(k(2)) = newslice(2);
      end
   end
   imagedisplay(handle,'coordinates',coords)
end


%
% Updates the 2D display
%
function udata = update_display(udata,imh,handle)
if imagedisplay(handle,'dirty')
   change = imagedisplay(handle,'change');
   set(imh,'cdata',imagedisplay(handle));
   if change
      axh = get(imh,'Parent');
      sz = imagedisplay(handle,'sizes');
      slicing = imagedisplay(handle,'slicing');
      xsz = sz(slicing(1));
      ysz = sz(slicing(2));
      set(imh,'XData',[0,xsz-1],'YData',[0,ysz-1]);
      set(axh,'XLim',[0,xsz]-0.5,'Ylim',[0,ysz]-0.5);
      udata.imsize = [xsz,ysz];
   end
end


%
% Update slices in linked displays
%
function hlist = update_linked(fig,hlist,newcoords,newslicing,curzoom)
%input of newcoords and newslicing is only required for 3D, 4D images

keep = ones(size(hlist));
curax = findobj(fig,'Type','axes');
curxlim = get(curax,'xlim');
curylim = get(curax,'ylim');

tag = get(fig,'Tag');
for ii=1:length(hlist)
   if ~isfigh(hlist(ii)) || ~strncmp(get(hlist(ii),'Tag'),tag,12)
      disp(['Warning: the linked display ',num2str(hlist(ii)),' no longer matches this one.']);
      keep(ii) = 0;
   else
      udata = get(hlist(ii),'UserData');
      if ~isempty(newcoords) && isfield(udata,'handle')
         imh = findobj(hlist(ii),'Type','image');
         if length(imh)~=1, return, end
         handle = udata.handle;
         imagedisplay(handle,'coordinates',newcoords);
         imagedisplay(handle,'slicing',newslicing);
         udata = update_display(udata,imh,handle);
         % TODO: axes don't resize?
         set(hlist(ii),'UserData',[]);    % Solve MATLAB bug!
         set(hlist(ii),'UserData',udata);
      end
      % Change the axis and zoom also for 2D data
      ax = findobj(hlist(ii),'Type','axes');
      if udata.zoom ~= curzoom
         au = get(ax,'Units');
         set(ax,'Units','pixels');
         set(hlist(ii),'Units','pixels');
         winsize = get(hlist(ii),'Position');
         winsize = winsize(3:4);
         pt = [mean(get(ax,'XLim')),mean(get(ax,'YLim'))];
         dipzoomZoom(curzoom/udata.zoom,pt,ax,udata,winsize)
         udata.zoom = dipfig_isnormalaspect(ax);
         set(hlist(ii),'UserData',[]);    % Solve MATLAB bug!
         set(hlist(ii),'UserData',udata);
         dipfig_titlebar(hlist(ii),udata);
         set(ax,'Units',au);
      end
      set(ax,'xlim',curxlim);
      set(ax,'ylim',curylim);
      % TODO: Update the displays linked by this display!
   end
end
hlist = hlist(keep);
if isempty(hlist)
   diplink(fig,'off');
end


%
% Clear a figure window
%
function clearnreset(fig)
delete(findobj(allchild(fig),'flat','serializable','on'));
set(fig,...
   'BusyAction','queue',...
   'ButtonDownFcn','',...
   'CloseRequestFcn','closereq',...
   'Color',[0.8 0.8 0.8],...
   'CreateFcn','',...
   'DeleteFcn','',...
   'DoubleBuffer','on',...
   'HandleVisibility','on',...
   'HitTest','on',...
   'Interruptible','on',...
   'KeyPressFcn','',...
   'MenuBar','none',...
   'NextPlot','replace',...
   'NumberTitle','on',...
   'PaperPositionMode','auto',...
   'Pointer','arrow',...
   'RendererMode','auto',...
   'Resize','on',...
   'ResizeFcn',@ResizeFcn,...
   'UIContextMenu',[],...
   'Units','pixels',...
   'UserData',[],...
   'WindowButtonDownFcn','',...
   'WindowButtonMotionFcn','',...
   'WindowButtonUpFcn','',...
   'WindowStyle','normal');

%
% Add the menus to the figure window
%
function create_menus(fig,nD,iscomp,iscol,isbin)
% Delete all menus
h = findobj(fig,'Type','uimenu');
if ~isempty(h)
   delete(h);
end
% Create 'File' menu
h = uimenu(fig,'Label','File','Tag','file');
uimenu(h,'Label','Save display ...','Tag','save','Callback',@save_figure_window,...
       'Accelerator','s');
if ~isunix
   uimenu(h,'Label','Copy display','Tag','copy','Callback',@(~,~)print(gcbf,'-dbitmap'),'Accelerator','c');
end
uimenu(h,'Label','Clear','Tag','clear','Callback',@(~,~)dipclf(gcbf),'Separator','on','Accelerator','x');
uimenu(h,'Label','Close','Tag','close','Callback',@(~,~)close(gcbf),'Separator','on','Accelerator','w');
% Create 'Sizes' menu
h = uimenu(fig,'Label','Sizes','Tag','sizes');
uimenu(h,'Label','10%','Tag','10','Callback',@(~,~)diptruesize(gcbf,10));
uimenu(h,'Label','25%','Tag','25','Callback',@(~,~)diptruesize(gcbf,25));
uimenu(h,'Label','50%','Tag','50','Callback',@(~,~)diptruesize(gcbf,50),'Accelerator','5');
uimenu(h,'Label','100%','Tag','100','Callback',@(~,~)diptruesize(gcbf,100),'Accelerator','1');
uimenu(h,'Label','200%','Tag','200','Callback',@(~,~)diptruesize(gcbf,200),'Accelerator','2');
uimenu(h,'Label','400%','Tag','400','Callback',@(~,~)diptruesize(gcbf,400));
uimenu(h,'Label','1000%','Tag','1000','Callback',@(~,~)diptruesize(gcbf,1000));
uimenu(h,'Label','Stretch to fill','Tag','off','Callback',@(~,~)diptruesize(gcbf,'off'),'Accelerator','0');
uimenu(h,'Label','Default window size','Tag','defaultsize','Separator','on','Callback',...
       @(~,~)default_figure_size(gcbf));
% Create 'Mappings' menu
h = uimenu(fig,'Label','Mappings','Tag','mappings');
if isbin
   uimenu(h,'Label','Normal','Tag','normal','Callback',...
          @(~,~)dipshow(gcbf,'ch_mappingmode','normal'));
else
   uimenu(h,'Label','Unit [0,1]','Tag','unit','Callback',...
          @(~,~)dipshow(gcbf,'ch_mappingmode','unit'));
   uimenu(h,'Label','Normal [0,255]','Tag','normal','Callback',...
          @(~,~)dipshow(gcbf,'ch_mappingmode','normal'));
   uimenu(h,'Label','12-bit [0,4095]','Tag','12bit','Callback',...
          @(~,~)dipshow(gcbf,'ch_mappingmode','12bit'));
   uimenu(h,'Label','16-bit [0,65535]','Tag','16bit','Callback',...
          @(~,~)dipshow(gcbf,'ch_mappingmode','16bit'));
   uimenu(h,'Label','Linear stretch','Tag','lin','Callback',...
          @(~,~)dipshow(gcbf,'ch_mappingmode','lin'),'Accelerator','L');
   uimenu(h,'Label','Percentile stretch','Tag','percentile','Callback',...
          @(~,~)dipshow(gcbf,'ch_mappingmode','percentile'));
   uimenu(h,'Label','Log stretch','Tag','log','Callback',...
          @(~,~)dipshow(gcbf,'ch_mappingmode','log'));
   uimenu(h,'Label','Based at 0','Tag','base','Callback',...
          @(~,~)dipshow(gcbf,'ch_mappingmode','base'));
   uimenu(h,'Label','Angle','Tag','angle','Callback',...
          @(~,~)dipshow(gcbf,'ch_mappingmode','angle'));
   uimenu(h,'Label','Orientation','Tag','orientation','Callback',...
          @(~,~)dipshow(gcbf,'ch_mappingmode','orientation'));
   uimenu(h,'Label','Manual ...','Tag','manual','Callback',...
          @(~,~)dipmapping(gcbf,'manual'));
   if nD>=3
      uimenu(h,'Label','Global stretch','Tag','globalstretch','Callback',...
             @(~,~)dipshow(gcbf,'ch_globalstretch',true),'Separator','on');
   end
   if ~iscol && nD>1
      uimenu(h,'Label','Grey','Tag','grey','Callback',...
             @(~,~)dipshow(gcbf,'ch_colormap','grey'),'Separator','on');
      uimenu(h,'Label','Saturation','Tag','saturation','Callback',...
             @(~,~)dipshow(gcbf,'ch_colormap','saturation'));
      uimenu(h,'Label','Zero-based','Tag','zerobased','Callback',...
             @(~,~)dipshow(gcbf,'ch_colormap','zerobased'));
      uimenu(h,'Label','Periodic','Tag','periodic','Callback',...
             @(~,~)dipshow(gcbf,'ch_colormap','periodic'));
      uimenu(h,'Label','Labels','Tag','labels','Callback',...
             @(~,~)dipshow(gcbf,'ch_mappingmode','labels'));
      uimenu(h,'Label','Custom...','Tag','custom','Callback',...
             @(~,~)dipmapping(gcbf,'custom'));
   end
   if iscomp
      uimenu(h,'Label','Magnitude','Tag','abs','Callback',...
             @(~,~)dipshow(gcbf,'ch_complexmapping','abs'),'Separator','on');
      uimenu(h,'Label','Phase','Tag','phase','Callback',...
             @(~,~)dipshow(gcbf,'ch_complexmapping','phase'));
      uimenu(h,'Label','Real part','Tag','real','Callback',...
             @(~,~)dipshow(gcbf,'ch_complexmapping','real'));
      uimenu(h,'Label','Imaginary part','Tag','imag','Callback',...
             @(~,~)dipshow(gcbf,'ch_complexmapping','imag'));
   end
end
if nD>=3
   uimenu(h,'Label','X-Y slice','Tag','xy','Callback',...
          @(~,~)dipshow(gcbf,'ch_slicing','xy'),'Separator','on');
   uimenu(h,'Label','X-Z slice','Tag','xz','Callback',...
          @(~,~)dipshow(gcbf,'ch_slicing','xz'));
   uimenu(h,'Label','Y-Z slice','Tag','yz','Callback',...
          @(~,~)dipshow(gcbf,'ch_slicing','yz'));
end
if nD==4
   uimenu(h,'Label','X-T slice','Tag','xt','Callback',...
          @(~,~)dipshow(gcbf,'ch_slicing','xt'));
   uimenu(h,'Label','Y-T slice','Tag','yt','Callback',...
          @(~,~)dipshow(gcbf,'ch_slicing','yt'));
   uimenu(h,'Label','Z-T slice','Tag','zt','Callback',...
          @(~,~)dipshow(gcbf,'ch_slicing','zt'));
end
% TODO: add slice / max projection / mean projection as options for nD>=3

% Create 'Actions' menu
h = uimenu(fig,'Label','Actions','Tag','actions');
uimenu(h,'Label','None','Tag','mouse_none','Callback',...
       @(~,~)change_action_state(gcbf,'none'));
uimenu(h,'Label','Pixel testing','Tag','mouse_diptest','Callback',@(~,~)diptest(gcbf,'on'),...
       'Accelerator','i');
uimenu(h,'Label','Zoom','Tag','mouse_dipzoom','Callback',@(~,~)dipzoom(gcbf,'on'),...
       'Accelerator','z');
uimenu(h,'Label','Looking Glass','Tag','mouse_diplooking','Callback',@(~,~)diplooking(gcbf,'on'));
uimenu(h,'Label','Pan','Tag','mouse_dippan','Callback',@(~,~)dippan(gcbf,'on'),...
       'Accelerator','p');
if nD>=2
      uimenu(h,'Label','Link displays ...','Tag','diplink','Callback',@(~,~)diplink(gcbf,'on'),...
          'Separator','on');
   % too lazy to also include it for 1D images (BR)
end
if nD>=3
   uimenu(h,'Label','Step through slices','Tag','mouse_dipstep','Callback',@(~,~)dipstep(gcbf,'on'),...
          'Accelerator','o');
   uimenu(h,'Label','Animate','Tag','dipanimate','Callback',@(~,~)dipanimate(gcbf));
   if nD==3 && ~iscol
      uimenu(h,'Label','Isosurface plot ...','Tag','dipiso','Callback',@(~,~)dipisosurface(gcbf));
   end

end
if nD > 1 && exist('javachk','file') && isempty(javachk('jvm'))
   uimenu(h,'Label','View5d','Tag','viewer5d','Callback',@(~,~)view5d(gcbf));
end
uimenu(h,'Label','Enable keyboard','Tag','keyboard','Separator','on',...
         'Callback',@(~,~)change_keyboard_state(gcbf,'toggle'),'Accelerator','k');
% Work around a bug in MATLAB 7.1 (R14SP3) (solution # 1-1XPN82).
h = uimenu(fig);drawnow;delete(h);


%
% Find the correct action state for the current image
%
function state = find_action_state(state,nD,iscomplex,iscolor)
switch state
   case 'diptest'
   case 'diporien'
      if nD~=2 || iscolor, state = 'none'; end
   case 'dipzoom'
   case 'dippan'
   case 'dipstep'
      if nD<3
         state = 'none';
       end
   otherwise
      state = 'none';
end


%
% Set the figure to the correct state
%
function change_action_state(fig,state)
udata = get(fig,'UserData');
if ~strcmp(state,udata.state)
   switch state
      case 'dipstep'
         dipstep(fig,'on');
      case 'diptest'
         diptest(fig,'on');
      case 'diporien'
         diporien(fig,'on');
      case 'dipzoom'
         dipzoom(fig,'on');
      case 'dippan'
         dippan(fig,'on');
      otherwise
         dipfig_clear_state(fig,udata);
   end
end


%
% Set the checkmarks to the current modes
%
function set_mode_check(fig,mappingmode,colmap,complexmapping,slicing)
m = findobj(get(fig,'Children'),'Tag','mappings');
set(get(m,'Children'),'Checked','off');
if ~isempty(mappingmode)
   set(findobj(m,'Tag',mappingmode),'Checked','on');
end
if ~isempty(colmap)
   set(findobj(m,'Tag',colmap),'Checked','on');
end
if ~isempty(complexmapping)
   set(findobj(m,'Tag',complexmapping),'Checked','on');
end
if ~isempty(slicing)
   map = 'xyzt';
   set(findobj(m,'Tag',map(slicing)),'Checked','on');
end


%
% Set the checkmark to the global stretch menu item
% (always call right after set_mode_check, which resets this one...)
%
function set_global_check(fig,globalstretch)
m = findobj(get(fig,'Children'),'Tag','globalstretch');
if globalstretch
   set(m,'Checked','on','Callback',@(~,~)dipshow(gcbf,'ch_globalstretch',false));
else
   set(m,'Checked','off','Callback',@(~,~)dipshow(gcbf,'ch_globalstretch',true));
end


%
% Set the mapping modes (does some user-input parsing too)
%
function change_mapping(fig,varargin)
N = nargin-1;
ii = 1;
udata = get(fig,'UserData');
handle = udata.handle;
disp1D = false;
colmap = [];
while ii<=N
   item = varargin{ii};
   if ~ischar(item)
      error('Illegal argument')
   end
   ii = ii+1;
   if ii>N
      error('More arguments expected')
   end
   switch item
      case 'ch_mappingmode'
         currange = varargin{ii};
         if ~ischar(currange) && ~isnumeric(currange)
            error('Illegal argument for mappingmode')
         end
         [currange,mappingmode,colmap] = parse_rangestr(currange);
         if strcmp(colmap,udata.colmap)
            colmap = [];
         elseif strcmp(colmap,'grey') && ~strcmp(udata.colmap,'labels')
            colmap = [];
         end
         if ~isempty(colmap)
            [udata.colmap,colmap] = parse_colmapstr(colmap);
         end
         if strcmp(mappingmode,'manual')
            if ~isempty(currange)
               imagedisplay(handle,'mappingmode',currange);
            end
         else
            imagedisplay(handle,'mappingmode',mappingmode);
         end
      case 'ch_colormap'
         colmap = varargin{ii};
         if ~ischar(colmap) && ~isnumeric(colmap)
            error('Illegal argument for colormap')
         end
         [udata.colmap,colmap] = parse_colmapstr(colmap);
      case 'ch_complexmapping'
         imagedisplay(handle,'complexmapping',varargin{ii});
         disp1D = true;
      case 'ch_slicing'
         slicing = parse_slicing(varargin{ii});
         imagedisplay(handle,'slicing',slicing);
      case 'ch_globalstretch'
         imagedisplay(handle,'globalstretch',varargin{ii});
      case 'ch_slice'
         newslice = varargin{ii};
         if ~isnumeric(newslice) || length(newslice)>1
            error('Illegal argument for slice number selection')
         end
         change_slice(udata,[newslice,-1])
      case  'ch_time'
         newtime = varargin{ii};
         if ~isnumeric(newtime) || length(newtime)>1
            error('Illegal argument for time number selection')
         end
         change_slice(udata,[-1,newtime])
      otherwise
         error('Illegal argument to change mapping in dipshow.')
   end
   ii = ii+1;
end

change = false;
dolinked = false;
if isfield(udata,'imagedata') %1D display
   axh = findobj(fig,'Type','axes');
   if length(axh)~=1, return, end
   if disp1D
      % 1D data change
      udata = display_data_1D(axh,udata);
   else
      % 1D range change
      udata = stretchYaxis_1D(axh,udata);
   end
else %other dimensionality
   change = imagedisplay(handle,'change'); % update axes?
   dolinked = imagedisplay(handle,'dirty');  % update linked displays?
   imh = findobj(fig,'Type','image');
   if length(imh)~=1, return, end
   udata = update_display(udata,imh,handle);
   if ~isempty(colmap)
      set(fig,'Colormap',colmap);
   end
end
set_mode_check(fig,imagedisplay(handle,'mappingmode'),udata.colmap,imagedisplay(handle,'complexmapping'),imagedisplay(handle,'slicing'));
if imagedisplay(handle,'dimensionality')>=3
   set_global_check(fig,imagedisplay(handle,'globalstretch'))
end
set(fig,'UserData',[]);    % Solve MATLAB bug!
set(fig,'UserData',udata);
if change && ~isempty(udata.zoom)
   diptruesize(fig,udata.zoom*100);
else
   dipfig_titlebar(fig,udata);
end
if dolinked && ~isempty(udata.linkdisplay)
   coords = imagedisplay(handle,'coordinates');
   slicing = imagedisplay(handle,'slicing');
   newlinks = update_linked(fig,udata.linkdisplay,coords,slicing,udata.zoom);
   if ~isequal(newlinks,udata.linkdisplay)
      udata.linkdisplay = newlinks;
      set(fig,'UserData',[]);    % Solve MATLAB bug!
      set(fig,'UserData',udata);
   end
end


%
% Change the keyboard state. CMD is 'on', 'off' or 'toggle'
%
function change_keyboard_state(fig,cmd)
if strcmp(cmd,'toggle')
   if isempty(get(fig,'KeyPressFcn'))
      cmd = 'on';
   else
      cmd = 'off';
   end
end
if strcmp(cmd,'on')
   set(fig,'KeyPressFcn',@KeyPressFcn);
   set(findobj(fig,'tag','keyboard'),'Checked','on');
else
   set(fig,'KeyPressFcn','');
   set(findobj(fig,'tag','keyboard'),'Checked','off');
end


%
% Callback functions for File->Save... menu item
%
function save_figure_window(~,~)
fig = gcbf;
p = dipgetpref('CurrentImageSaveDir');
curp = cd;
if isempty(p) || strcmp(p,curp)
   % The default directory is the current one
   isset = false;
else
   % The default directory is not the current one
   isset = true;
   cd(p);
end
% Get a directory and file name from the user
[filename,p] = uiputfile({'*.png';'*.jpg';'*.tif';'*.eps'},'Save display as');
if isset
   cd(curp);
end
if ischar(filename)
   % Remove ending separator from directory name
   if length(p)>1 && p(end)==filesep
      p(end) = [];
   end
   if strcmp(p,curp)
      % Saving in current directory
      if isset
         % If there was a directory name saved, delete it (current directory is new default)
         dipsetpref('CurrentImageSaveDir','');
      end
   else
      % Save the selected directory as default
      dipsetpref('CurrentImageSaveDir',p);
   end
   % Figure out the format from the extension
   [~,~,ext] = fileparts(filename);
   switch lower(ext)
   case {'.tif','.tiff'}
      format = '-dtiff';
   case '.png'
      format = '-dpng';
   case {'.jpg','.jpeg'}
      format = '-djpeg80';
   case '.eps'
      format = '-deps2c';
   otherwise
      warning('Could not determine file format from extension. Saving as TIFF with wrong file extension!')
      format = '-dtiff';
   end
   % Do the saving
   print(fig,format,'-r0',fullfile(p,filename));
end



%
% Callback functions for DIPZOOM
%
function dipzoomWindowButtonDownFcn(fig,~)
if strncmp(get(fig,'Tag'),'DIP_Image',9)
   udata = get(fig,'UserData');
   ax = findobj(fig,'Type','axes');
   if length(ax)~=1
      return
   end
   udata.ax = ax;
   udata.oldAxesUnits = get(ax,'Units');
   udata.oldNumberTitle = get(fig,'NumberTitle');
   set(ax,'Units','pixels');
   set(fig,'Units','pixels');
   udata.figsz = get(fig,'position');
   udata.figsz = udata.figsz(3:4);
   udata.coords = dipfig_getcurpos(ax); % Always over image!
   if isfield(udata,'imagedata') % 1D
      ylim = get(ax,'YLim');
      pos = [udata.coords(1)-0.5,ylim(1)-1,1,diff(ylim)+3];
   else
      pos = [udata.coords-0.5,1,1];
   end
   if useshg2
      udata.recth = rectangle('Position',pos,'EdgeColor',[0,0,0.8]);
   else
      udata.recth = rectangle('Position',pos,'EraseMode','xor','EdgeColor',[0,0,0.8]);
   end
   set(fig,'WindowButtonMotionFcn',@dipzoomWindowButtonMotionFcn,...
           'WindowButtonUpFcn',@dipzoomWindowButtonUpFcn,...
           'NumberTitle','off',...
           'UserData',[]);   % Solve MATLAB bug!
   set(fig,'UserData',udata);
   dipzoomUpdateDisplay(fig,ax,udata);
end

function dipzoomWindowButtonMotionFcn(fig,~)
if strncmp(get(fig,'Tag'),'DIP_Image',9)
   udata = get(fig,'UserData');
   dipzoomUpdateDisplay(fig,udata.ax,udata);
end

function dipzoomWindowButtonUpFcn(fig,~)
if strncmp(get(fig,'Tag'),'DIP_Image',9)
   udata = get(fig,'UserData');
   delete(udata.recth);
   pt = dipfig_getcurpos(udata.ax);
   if isfield(udata,'imagedata')
      pt = pt(1);
      udata.coords = udata.coords(1);
   end
   if abs(pt-udata.coords) > 2
      % Dragged a rectangle
      %axpos = get(udata.ax,'Position');
      if isfield(udata,'imagedata')
         delta = abs(pt-udata.coords)+1;
         pt = min(pt,udata.coords);
         set(udata.ax,'XLim',pt+[0,delta]-0.5);
         position_axes(udata.ax,0,delta,udata.figsz);
      else
         [pt,delta] = dipzoomConstrain(pt,udata);
         pt = min(pt,udata.coords);
         pelsize = min(udata.figsz./delta);
         set(udata.ax,'XLim',pt(1)+[0,delta(1)]-0.5,'YLim',pt(2)+[0,delta(2)]-0.5);
         position_axes(udata.ax,[pelsize,pelsize],delta,udata.figsz);
      end
   else
      % Clicked
      switch get(fig,'SelectionType')
         case 'normal'
            % zoom in
            dipzoomZoom(2,pt,udata.ax,udata,udata.figsz);
         case 'alt'
            % zoom out
            dipzoomZoom(0.5,pt,udata.ax,udata,udata.figsz);
         case 'open'
            % set to 100%
            dipzoomZoom(0,pt,udata.ax,udata,udata.figsz);
            %if isfield(udata,'imagedata')
            %   set(udata.ax,'XLim',[0,udata.imsize]-0.5,...
            %       'Units','normalized','Position',[0,0,1,1]);
            %else
            %   set(udata.ax,'XLim',[0,udata.imsize(1)]-0.5,'YLim',[0,udata.imsize(2)]-0.5,...
            %       'Units','normalized','Position',[0,0,1,1]);
            %end
         otherwise
      end
   end
   % Clean up
   udata = rmfield(udata,{'recth','coords','figsz'});
   if ~isequal(udata.zoom,0)
      udata.zoom = dipfig_isnormalaspect(udata.ax);
   end
   set(udata.ax,'Units',udata.oldAxesUnits);
   set(fig,'WindowButtonMotionFcn','','WindowButtonUpFcn','',...
           'NumberTitle',udata.oldNumberTitle);
   udata = rmfield(udata,{'ax','oldAxesUnits','oldNumberTitle'});
   dipfig_titlebar(fig,udata);
   % Update linked displays
   if ~isempty(udata.linkdisplay)
      udata.linkdisplay = update_linked(fig,udata.linkdisplay,[],[],udata.zoom);
   end
   set(fig,'UserData',[]);    % Solve MATLAB bug!
   set(fig,'UserData',udata);
end

function dipzoomUpdateDisplay(fig,ax,udata)
pt = dipfig_getcurpos(ax);
if isfield(udata,'imagedata')
   delta = abs(pt(1)-udata.coords(1))+1;
   pos = get(udata.recth,'Position');
   pos(1) = min(pt(1),udata.coords(1))-0.5;
   pos(3) = delta;
   set(udata.recth,'Position',pos);
   set(fig,'Name',['(',num2str(pt(1)),') ',' size: ',num2str(delta(1))]);
else
   [pt,delta] = dipzoomConstrain(pt,udata);
   % Update display
   set(udata.recth,'Position',[min(pt,udata.coords)-0.5,delta]);
   set(fig,'Name',['(',num2str(pt(1)),',',num2str(pt(2)),') ',...
          ' size: ',num2str(delta(1)),'x',num2str(delta(2))]);
end

function [pt,delta] = dipzoomConstrain(pt,udata)
% Constrain proportions to display's: udata.coords is the fixed corner.
direction = sign(pt-udata.coords);
delta = abs(pt-udata.coords)+1;
delta = floor(max(delta./udata.figsz)*udata.figsz);
pt = udata.coords + (delta-1).*direction;
% Constrain size to not exceed image dimensions
pt = max(pt,0);
pt = min(pt,udata.imsize-1);
% Again constrain proportions, this time take smallest rectangle
direction = sign(pt-udata.coords);
delta = abs(pt-udata.coords)+1;
delta = ceil(min(delta./udata.figsz)*udata.figsz);
pt = udata.coords + (delta-1).*direction;

function dipzoomZoom(zoom,pt,ax,udata,winsize)
axpos = get(ax,'Position');
dispsize = winsize;
if isfield(udata,'imagedata')
   curxlim = get(ax,'XLim');
   pelsize = axpos(3)/diff(curxlim);
   winsize = winsize(1);
else
   axsize = axpos([3,4]);
   %axpos = axpos([1,2]);
   curxlim = get(ax,'XLim'); curxrange = diff(curxlim);
   curylim = get(ax,'YLim'); curyrange = diff(curylim);
   pelsize = [(axsize(1)/curxrange),(axsize(2)/curyrange)];
end
imsz = udata.imsize;
newpelsize = pelsize*zoom;
if zoom==0
   newpelsize(:) = 1;
end
sz = min(imsz,ceil(winsize./(newpelsize)));
sz = max(sz,1); % Minimum image size: 1 pixel.
pt = round(pt-sz/2);
pt = max(pt,0);
pt = min(pt,imsz-sz);
if length(imsz) == 1
   set(ax,'XLim',pt+[0,sz]-0.5);
else
   set(ax,'XLim',pt(1)+[0,sz(1)]-0.5,'YLim',pt(2)+[0,sz(2)]-0.5);
end
position_axes(ax,newpelsize,sz,dispsize);


%
% Callback function for DIPSTEP
%
function dipstepWindowButtonDownFcn(fig,~)
if strncmp(get(fig,'Tag'),'DIP_Image_3D',12) || strncmp(get(fig,'Tag'),'DIP_Image_4D',12)
   udata = get(fig,'UserData');
   ax = findobj(fig,'Type','axes');
   udata.ax = ax;
   udata.imh = findobj(fig,'Type','image');
   if length(ax)~=1 || length(udata.imh)~=1
      return
   end
   udata.oldAxesUnits = get(ax,'Units');
   set(ax,'Units','pixels');
   pt = get(0,'PointerLocation');
   udata.coords = [pt(1,1),-pt(1,2)];
   coords = imagedisplay(udata.handle,'coordinates');
   k = imagedisplay(udata.handle,'orthogonal');
   udata.startslice = coords(k);
   udata.moved = 0;
   set(fig,'WindowButtonMotionFcn',@dipstepWindowButtonMotionFcn,...
           'WindowButtonUpFcn',@dipstepWindowButtonUpFcn,...
           'UserData',[]);   % Solve MATLAB bug!
   set(fig,'UserData',udata);
end

function dipstepWindowButtonMotionFcn(fig,~)
if strncmp(get(fig,'Tag'),'DIP_Image_3D',12) || strncmp(get(fig,'Tag'),'DIP_Image_4D',12)
   udata = get(fig,'UserData');
   pt = get(0,'PointerLocation');
   pt = [pt(1,1),-pt(1,2)];
   delta = (pt-udata.coords)/3; % move one slice for each 3 pixel cursor movement
   [~,dir] = max(abs(delta));
   delta = round(delta(dir));
   udata.moved = 1;
   newslice = udata.startslice;
   if length(udata.startslice)==2
      switch get(fig,'SelectionType')
         case 'alt'
            newslice(1) = newslice(1)+delta;
         otherwise
            newslice(2) = newslice(2)+delta;
      end
   else
      newslice = newslice+delta;
   end
   change_slice(udata,newslice)
   udata = update_display(udata,udata.imh,udata.handle);
   dipfig_titlebar(fig,udata);
   set(fig,'UserData',[]);   % Solve MATLAB bug!
   set(fig,'UserData',udata);
end

function dipstepWindowButtonUpFcn(fig,~)
udata = get(fig,'UserData');
if strncmp(get(fig,'Tag'),'DIP_Image_3D',12) || strncmp(get(fig,'Tag'),'DIP_Image_4D',12)
   if ~udata.moved
      newslice = udata.startslice;
      switch get(fig,'SelectionType')
         case {'normal','extend'}
            newslice(1) = newslice(1)+1;
            udata.prevclick = 1;
         case 'alt'
            newslice(1) = newslice(1)-1;
            udata.prevclick = -1;
         case 'open' %double-click: repeat last click
            newslice(1) = newslice(1)+udata.prevclick;
         otherwise
            return
      end
      change_slice(udata,newslice)
      udata = update_display(udata,udata.imh,udata.handle);
      dipfig_titlebar(fig,udata);
   end
   if ~isempty(udata.linkdisplay)
      handle = udata.handle;
      coords = imagedisplay(handle,'coordinates');
      slicing = imagedisplay(handle,'slicing');
      udata.linkdisplay = update_linked(fig,udata.linkdisplay,coords,slicing,udata.zoom);
   end
   % Clean up
   set(udata.ax,'Units',udata.oldAxesUnits);
   set(fig,'WindowButtonMotionFcn','','WindowButtonUpFcn','');
   udata = rmfield(udata,{'ax','imh','oldAxesUnits','moved','coords','startslice'});
   set(fig,'UserData',[]);    % Solve MATLAB bug!
   set(fig,'UserData',udata);
end


%
% Callback function for keyboard event
%
function KeyPressFcn(fig,~)
if strncmp(get(fig,'Tag'),'DIP_Image',9)
   udata = get(fig,'UserData');
   nD = imagedisplay(udata.handle,'dimensionality');
   ch = get(fig,'CurrentCharacter');
   if ~isempty(ch)
      udata.lastkeypress=upper(ch);
      set(fig,'UserData',[]);
      set(fig,'UserData',udata);
      switch ch
         case {'0','1','2','3','4','5','6','7','8','9'}
            if nD>=3
               if isfield(udata,'nextslice')
                  udata.nextslice = [udata.nextslice,ch];
               else
                  udata.nextslice = ch;
               end
               set(fig,'UserData',[]);    % Solve MATLAB bug!
               set(fig,'UserData',udata);
            end
         case {char(13),';'} % Enter: go to selected slice
            if (nD>=3) && isfield(udata,'nextslice') && ~isempty(udata.nextslice)
               newslice = str2double(udata.nextslice);
               if ~isnan(newslice)
                  imh = findobj(fig,'Type','image');
                  if (nD>3) && ch==';'
                     change_slice(udata,[-1,newslice])
                  else
                     change_slice(udata,[newslice,-1])
                  end
                  udata = update_display(udata,imh,udata.handle);
                  dipfig_titlebar(fig,udata);
                  if ~isempty(udata.linkdisplay)
                     handle = udata.handle;
                     coords = imagedisplay(handle,'coordinates');
                     slicing = imagedisplay(handle,'slicing');
                     udata.linkdisplay = update_linked(fig,udata.linkdisplay,coords,slicing,udata.zoom);
                  end
               end
               udata.nextslice = '';
               set(fig,'UserData',[]);    % Solve MATLAB bug!
               set(fig,'UserData',udata);
            end
         case {'p','P','n','N'} % Previous/next slice
            if nD>=3
               handle = udata.handle;
               dim3 = imagedisplay(handle,'orthogonal');
               dim3 = dim3(1);
               newslice = imagedisplay(handle,'coordinates');
               newslice = newslice(dim3);
               if upper(ch)=='P'
                  newslice = newslice-1;
               else
                  newslice = newslice+1;
               end
               change_slice(udata,[newslice,-1])
               imh = findobj(fig,'Type','image');
               udata = update_display(udata,imh,udata.handle);
               dipfig_titlebar(fig,udata);
               if ~isempty(udata.linkdisplay)
                  coords = imagedisplay(handle,'coordinates');
                  slicing = imagedisplay(handle,'slicing');
                  udata.linkdisplay = update_linked(fig,udata.linkdisplay,coords,slicing,udata.zoom);
               end
               udata.nextslice = '';
               set(fig,'UserData',[]);    % Solve MATLAB bug!
               set(fig,'UserData',udata);
            end
         case {'f','b','F','B'}
            if nD>=4
               handle = udata.handle;
               dim4 = imagedisplay(handle,'orthogonal');
               dim4 = dim4(2);
               newslice = imagedisplay(handle,'coordinates');
               newslice = newslice(dim4);
               if upper(ch)=='P'
                  newslice = newslice-1;
               else
                  newslice = newslice+1;
               end
               change_slice(udata,[-1,newslice])
               imh = findobj(fig,'Type','image');
               udata = update_display(udata,imh,udata.handle);
               dipfig_titlebar(fig,udata);
               if ~isempty(udata.linkdisplay)
                  coords = imagedisplay(handle,'coordinates');
                  slicing = imagedisplay(handle,'slicing');
                  udata.linkdisplay = update_linked(fig,udata.linkdisplay,coords,slicing,udata.zoom);
               end
               udata.nextslice = '';
               set(fig,'UserData',[]);    % Solve MATLAB bug!
               set(fig,'UserData',udata);
            end
         case {'i','I','o','O'} % Zoom in/out
            ax = findobj(fig,'Type','axes');
            au = get(ax,'Units');
            set(ax,'Units','pixels');
            set(fig,'Units','pixels');
            winsize = get(fig,'Position');
            winsize = winsize(3:4);
            if length(udata.imsize)==1
               pt = mean(get(ax,'XLim'));
            else
               pt = [mean(get(ax,'XLim')),mean(get(ax,'YLim'))];
            end
            if upper(ch)=='I'
               dipzoomZoom(2,pt,ax,udata,winsize)
            else
               dipzoomZoom(0.5,pt,ax,udata,winsize)
            end
            if ~isequal(udata.zoom,0)
               udata.zoom = dipfig_isnormalaspect(ax);
            end
            set(ax,'Units',au);
            dipfig_titlebar(fig,udata);
            if ~isempty(udata.linkdisplay)
               udata.linkdisplay = update_linked(fig,udata.linkdisplay,[],[],udata.zoom);
            end
            set(fig,'UserData',[]);    % Solve MATLAB bug!
            set(fig,'UserData',udata);
         case {'a','A',char(28)} % Pan left
            ax = findobj(fig,'Type','axes');
            curxlim = get(ax,'Xlim');
            stepsize = ceil(diff(curxlim)/2);
            stepsize = min(stepsize,curxlim(1)+0.5);
            curxlim = curxlim-stepsize;
            set(ax,'Xlim',curxlim);
            if ~isempty(udata.linkdisplay)
               udata.linkdisplay = update_linked(fig,udata.linkdisplay,[],[],udata.zoom);
               set(fig,'UserData',[]);    % Solve MATLAB bug!
               set(fig,'UserData',udata);
            end
         case {'d','D',char(29)} % Pan right
            udata = get(fig,'UserData');
            ax = findobj(fig,'Type','axes');
            curxlim = get(ax,'Xlim');
            stepsize = ceil(diff(curxlim)/2);
            stepsize = min(stepsize,udata.imsize(1)-curxlim(2)-0.5);
            curxlim = curxlim+stepsize;
            set(ax,'Xlim',curxlim);
            if ~isempty(udata.linkdisplay)
               udata.linkdisplay = update_linked(fig,udata.linkdisplay,[],[],udata.zoom);
               set(fig,'UserData',[]);    % Solve MATLAB bug!
               set(fig,'UserData',udata);
            end
         case {'w','W',char(30)} % Pan up
            udata = get(fig,'UserData');
            if length(udata.imsize)>1
               ax = findobj(fig,'Type','axes');
               curylim = get(ax,'Ylim');
               stepsize = ceil(diff(curylim)/2);
               stepsize = min(stepsize,curylim(1)+0.5);
               curylim = curylim-stepsize;
               set(ax,'Ylim',curylim);
               if ~isempty(udata.linkdisplay)
                  udata.linkdisplay = update_linked(fig,udata.linkdisplay,[],[],udata.zoom);
                  set(fig,'UserData',[]);    % Solve MATLAB bug!
                  set(fig,'UserData',udata);
               end
            end
         case {'s','S',char(31)} % Pan down
            udata = get(fig,'UserData');
            if length(udata.imsize)>1
               ax = findobj(fig,'Type','axes');
               curylim = get(ax,'Ylim');
               stepsize = ceil(diff(curylim)/2);
               stepsize = min(stepsize,udata.imsize(2)-curylim(2)-0.5);
               curylim = curylim+stepsize;
               set(ax,'Ylim',curylim);
               if ~isempty(udata.linkdisplay)
                  udata.linkdisplay = update_linked(fig,udata.linkdisplay,[],[],udata.zoom);
                  set(fig,'UserData',[]);    % Solve MATLAB bug!
                  set(fig,'UserData',udata);
               end
            end
         case {char(27)} % Esc: disable this callback
            change_keyboard_state(fig,'off');
      end
   end
end


%
% Callback function for resizing windows
%
function ResizeFcn(fig,~)
udata = get(fig,'UserData');
% 27-10-2006 MvG -- ResizeFcn gets called while the "construction" of the
% display data is still ongoing. This was originally not the case, because
% the figure's visibility was 'off' during this construction. Unfortunately,
% the visibility off-on cycle implies that the window comes to front. With
% 'BringToFrontOnDisplay' off, we cannot turn off the window's visibility
% and therefore this callback gets called. We can simply test whether udata
% is empty or not to ignore the callback during the construction phase.
if isempty(udata)
   return;
end
if ~isequal(udata.zoom,0)
   ax = findobj(fig,'Type','axes');
   if length(ax)==1
      if ~isempty(udata.zoom)
         % there's an aspect ratio we want to keep
         zoom = udata.zoom;
         set(ax,'Units','pixels');
         set(fig,'Units','pixels');
         figsz = get(fig,'Position');
         figsz = figsz(3:4);
         if length(udata.imsize)==1
            axsz = floor(figsz(1)/zoom);
            axsz = min(axsz,udata.imsize);
            axleft = get(ax,'XLim');
            axleft = axleft(1);
            axleft = min(udata.imsize-axsz-0.5,axleft);
            set(ax,'XLim',[axleft,axleft+axsz]);
            position_axes(ax,zoom,axsz,figsz)
         else
            axsz = floor(figsz/zoom);
            axsz = min(axsz,udata.imsize(1:2));
            axleft = [get(ax,'XLim'),get(ax,'YLim')];
            axleft = axleft([1,3]);
            axleft = min(udata.imsize(1:2)-axsz-0.5,axleft);
            set(ax,'XLim',[axleft(1),axleft(1)+axsz(1)],'YLim',[axleft(2),axleft(2)+axsz(2)]);
            position_axes(ax,[zoom,zoom],axsz,figsz)
         end
         % Restore the units
         set(ax,'Units','normalized');
      end
   end
end


%
% Puts the axes in the right location
% zoom = factor
% axsz = number of pixels to show  --- axsz*zoom = size of axes in screen pixels
% figsz = size of figure window in screen pixels
%
function position_axes(ax,zoom,axsz,figsz)
if length(zoom)==1
   if zoom==0
      zoom = figsz(1)/axsz;
   end
   scrsz = axsz.*zoom;
   if scrsz<1
      zoom = zoom / scrsz;
   end
   ifigsz = floor(figsz(1)/zoom);
   leftgutter = floor((ifigsz-axsz)/2)*zoom;
   set(ax,'Position',[leftgutter+1,0,axsz.*zoom,figsz(2)]);
else
   if zoom(1)==0
      zoom(1) = figsz(1)/axsz(1);
   end
   if zoom(2)==0
      zoom(2) = figsz(2)/axsz(2);
   end
   scrsz = axsz.*zoom;
   if any(scrsz<1)
      zoom = zoom / min(scrsz);
   end
   ifigsz = floor(figsz./zoom);
   leftgutter = floor((ifigsz(1)-axsz(1))/2)*zoom(1);
   buttomgutter = figsz(2)-(axsz(2)+floor((ifigsz(2)-axsz(2))/2))*zoom(2);
   set(ax,'Position',[leftgutter+1,buttomgutter+1,axsz.*zoom]);
end

function default_figure_size(fig)
set(fig,'Units','pixels');
pos = get(fig,'position');
pos(2) = pos(2)+pos(4);
pos(3:4) = [dipgetpref('DefaultFigureWidth'),dipgetpref('DefaultFigureHeight')];
pos(2) = pos(2)-pos(4);
set(fig,'position',pos);
