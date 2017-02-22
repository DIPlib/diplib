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

% (c)1999-2014, Delft University of Technology
% (c)2017, Cris Luengo

%Undocumented:
%   DIPSHOW(...,'name',NAME) causes the figure window's title to be set
%   to NAME instead of INPUTNAME(IN). This is used by DIP_IMAGE/DISPLAY.
%
%   DIPSHOW(...,'position',POS) causes the figure's 'Position' property
%   to be set to POS. POS must be a 1x4 array with [X Y H W] or a 1x2 array
%   with [W H]. In the last case, the X and Y position is not changed.
%   This disables the truesize setting.
%
%   If IN is [], the image is made black, and the zoom factor is not shown.
%   This is used by DIPFIG, together with the 'position' option, to create
%   a figure window.
%
%   DIPSHOW(H,'ch_mappingmode',RANGE), DIPSHOW(H,'ch_mappingmode',MAPPINGMODE),
%   DIPSHOW(H,'ch_colormap', COLORMAP), DIPSHOW(H,'ch_globalstretch',BOOLEAN),
%   DIPSHOW(H,'ch_complexmapping',COMPLEXMAP), DIPSHOW(H,'ch_slicing',SLICING),
%   DIPSHOW(H,'ch_slice',SLICE), DIPSHOW(H,'ch_time',SLICE).
%   RANGE or MAPPINGMODE are as defined above for regular DIPSHOW syntax.
%   COLORMAP is 'grey', 'periodic', 'saturation', 'labels' or a colormap.
%   BOOLEAN is 'yes', 'no', 'on', 'off', 1 or 0.
%   COMPLEXMAP is one of: 'abs', 'real', 'imag', 'phase'.
%   SLICING is one of: 'xy', 'xz', 'yz', 'xt', 'yt', 'zt'.
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
%       - '_1D', '_2D' or '_3D'.
%       - '_Binary', '_Grey', '_Color'.
% - A group of Uimenu objects, defined in CREATE_MENUS.
% - An Axes object with:
%   - An Image object.
% - A UserData object with:
%   - For all images:
%      - udata.mappingmode    -> String representing mapping mode: 'normal', 'lin', 'log', etc.
%                                Equal to '' means that udata.currange is to be used as-is.
%      - udata.currange       -> 2 values with the current display range.
%      - udata.complexmapping -> String representing complex to real mapping: 'abs', 'real',
%                                'imag', 'phase'.
%      - udata.computed       -> The values computed for the different mapping modes, if they
%                                have been computed (it might not exist!). It is a structure
%                                with a field for each of the mappingmode strings. Only the
%                                elements that have been computed exist.
%      - udata.computed_XXX   -> For complex images, these contain a copy of udata.computed
%                                computed for each of the complexmapping strings (XXX). Only
%                                those that have been used exists.
%      - udata.slicing        -> String representing 3D to 2D mapping: '' (for 1D/2D images),
%                                'xy', 'xz', 'yz', 'xt', 'yt', 'zt'.
%      - udata.colmap         -> String representing the colormap: 'grey', 'periodic', 'saturation',
%                                'zerobased', 'labels', 'custom'.
%      - udata.imagedata      -> Original dip_image object (or slice out of the 3D volume),
%                                converted to RGB colorspace in case of color image.
%                                A color image has the color along the 3rd. dimension.
%      - udata.imsize         -> image size: [x,y,z] for 3D, [x,y] for 2D, [x] for 1D.
%                                The x and y components are the slice dimensions, thus this
%                                array is re-arranged when changing the udata.slicing.
%      - udata.colspace       -> Name of color space: '' (for grey), 'RGB', 'Lab', 'XYZ', etc.
%      - udata.zoom           -> Pixel size or [] if aspect ratio is not 1:1. If 0, don't show zoom!
%      - udata.state          -> Action state: 'none', 'dipstep', 'diptest', 'dipzoom', etc.
%      - udata.imname         -> Name to be displayed in title bar.
%   - For 3D images:
%      - udata.maxslice       -> Highest slice available (udata.imsize(3)-1).
%      - udata.curslice       -> Slice currently displayed.
%      - udata.slices         -> Originial dip_image object, in case of color image it's
%                                in the original color space.
%      - udata.linkdisplay    -> If linked, list of figure handles, else [].
%      - udata.globalstretch  -> 0 or 1, indicating if stretching is computed on one slice or
%                                the whole thing.
%      - udata.globalcomputed -> Copy of udata.computed computed on global stuff. Or, with complex
%                                images, copies of udata.computed_XXX.
%   - For 4D images the same stuff as in 3D, but also:
%      - udata.maxtime        -> Highest slice available (udata.imsize(4)-1).
%      - udata.curtime        -> Slice currently displayed.
%   - For color images
%      - udata.colordata      -> dip_image object in original colorspace. In case of
%                                3D image, it is a slice out of udata.slices.
%      - udata.channels       -> Number of channels (== length(udata.colordata)).
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

% COMMON PHRASES
%
% Tests using the UserData property:
%  Dimensionality :       length(udata.imsize)
%  Is color:              ~isempty(udata.colspace)
%  Is binary:             islogical(udata.imagedata)
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
%   dipshow_core                  Core for 2D and 3D images
%   update_slice                  Update the current slice for a 3D image display
%   update_linked                 Update the slice in linked displays
%   display_data                  Send 2D data to display (2D image or slice out of 3D image)
%   set_udata_computed_lin        Set the udata.computed.lin field.
%   set_udata_computed_percentile Set the udata.computed.percentile field.
%   mapcomplexdata                Map complex data to the real domain
%   clearnreset                   Delete all figure children and reset properties to DIPSHOW defaults
%   create_menus                  Create menus for figure window (contains callback definitions)
%   find_action_state             Find the correct action state for the current image
%   change_action_state           Set the action state
%   set_mode_check                Set the checkmarks in the Mappings menu
%   set_global_check              Set the checkmark to the global stretch menu item
%   change_mapping                Set the mapping modes (does some user-input parsing too)
%   change_keyboard_state         Enable or disable the keyboard callback for a figure window
%   save_figure_window            Callback for the File->Save... menu item
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


% We need to handle some callbacks
if nargin == 2
   if ischar(varargin{1}) && ischar(varargin{2}) && strcmp(varargin{1},'DIP_callback')
      switch varargin{2}
         case 'menu_actions_none_cb'
            change_action_state(gcbf,'none');
         case 'menu_actions_keyboard_cb'
            change_keyboard_state(gcbf,'toggle');
         case 'menu_file_save_cb'
            save_figure_window(gcbf);
         case 'dipzoomWindowButtonDownFcn'
            dipzoomWindowButtonDownFcn(gcbf);
         case 'dipzoomWindowButtonMotionFcn'
            dipzoomWindowButtonMotionFcn(gcbf);
         case 'dipzoomWindowButtonUpFcn'
            dipzoomWindowButtonUpFcn(gcbf);
         case 'dipstepWindowButtonDownFcn'
            dipstepWindowButtonDownFcn(gcbf);
         case 'dipstepWindowButtonMotionFcn'
            dipstepWindowButtonMotionFcn(gcbf);
         case 'dipstepWindowButtonUpFcn'
            dipstepWindowButtonUpFcn(gcbf);
         case 'KeyPressFcn'
            KeyPressFcn(gcbf);
         case 'ResizeFcn'
            ResizeFcn(gcbf);
         otherwise
            error('Unknown callback function.')
      end
      return
   end
elseif nargin == 3
   if ischar(varargin{1}) && ischar(varargin{2}) && strcmp(varargin{1},'DIP_callback')
      switch varargin{2}
         case 'ResizeFcn'
            ResizeFcn(varargin{3});
         otherwise
            error('Unknown callback function.')
      end
      return
   end
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
   elseif ishandle(varargin{n}) && numel(varargin{n})==1
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
      try
         change_mapping(fig,varargin{n:end});
      catch
         error(firsterr);
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
   try
      [currange,mappingmode,newcolmap] = parse_rangestr(rangestr);
      if ischar(newcolmap) && ~strcmp(newcolmap,'custom') && ~strcmp(newcolmap,colmap)
         [colmap,colmapdata] = parse_colmapstr(newcolmap);
      end
   catch
      error(firsterr)
   end
end
if hascolmap
   try
      [colmap,colmapdata] = parse_colmapstr(colmapstr);
   catch
      error(firsterr)
   end
end

if isnumeric(in) && isempty(in)
   % Maybe we don't want to display any image
   dipshow_core(fig,dip_image(0),imname,currange,mappingmode,colmap,colmapdata,complexmapping,slicing,state,keys,0,1);
else
   % Check input image
   if ~isscalar(in) && ~iscolor(in)
      error('Cannot show tensor image.')
   end
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
slicing = dipgetpref('DefaultSlicing');
state = dipgetpref('DefaultActionState');
if dipgetpref('EnableKeyboard')
   keys = 'on';
else
   keys = 'off';
end
% change defaults according to current settings
if ishandle(fig)
   udata = get(fig,'UserData');
   if isfield(udata,'mappingmode')
      mappingmode = udata.mappingmode;
   end
   if isempty(mappingmode) && isfield(udata,'currange')
      currange = udata.currange;
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
   if isfield(udata,'complexmapping')
      complexmapping = udata.complexmapping;
   end
   if isfield(udata,'slicing')
      slicing = udata.slicing;
   end
   if isfield(udata,'state')
      state = udata.state;
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
      case 'all'
         mappingmode = 'lin';
      case {'8bit','u8bit'}
         mappingmode = 'normal';
      case 'u12bit'
         mappingmode = '12bit';
      case 'u16bit'
         mappingmode = '16bit';
      case {'normal','unit','lin','12bit','16bit','s8bit','s12bit','s16bit'}
      case 'log'
      case 'percentile'
      case 'base'
         colmap = 'zerobased';
      case 'angle'
         colmap = 'periodic';
      case 'orientation'
         colmap = 'periodic';
      case 'labels'
         mappingmode = 'normal';
         colmap = 'labels';
      otherwise
         error(['Illegal argument ''',mappingmode,'''.'])
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
      if currange(1) == 0
         if currange(2) == 255
            currange = [];
            mappingmode = 'normal';
         elseif currange(2) == 4095
            currange = [];
            mappingmode = '12bit';
         elseif currange(2) == 65535
            currange = [];
            mappingmode = '16bit';
         end
      elseif currange(1) == -128 && currange(2) == 127
            currange = [];
            mappingmode = 's8bit';
      elseif currange(1) == -2048 && currange(2) == 2047
            currange = [];
            mappingmode = 's12bit';
      elseif currange(1) == -32768 && currange(2) == 32767
            currange = [];
            mappingmode = 's16bit';
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
udata.state = ''; % Make sure the state is updated later on.
udata.currange = currange;
udata.mappingmode = mappingmode;
udata.complexmapping = complexmapping;
udata.colspace = colorspace(in);
udata.imsize = sz;
if iscol
   udata.channels = length(in);
   udata.colordata = in;
   udata.imagedata = cat(3,in);
else
   udata.imagedata = in;
end
udata.slicing = '';
udata.colmap = '';
udata.linkdisplay = [];
% Display 1D image
udata = display_data_1D(axh,udata);
set(axh,'XLim',[0,sz]-0.5);
% Set figure properties
set_mode_check(fig,udata.mappingmode,'',udata.complexmapping,udata.slicing);
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
tag = ['DIP_Image_1D'];
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
cdata = mapcomplexdata(udata.imagedata,udata.complexmapping);
delete(get(axh,'Children'));
cdata = double(cdata);
xdata = [0:udata.imsize]-0.5;
xdata = reshape(repmat(xdata,2,1),[1,udata.imsize*2+2]);
xdata = xdata(2:end-1);
if ~isempty(udata.colspace);
   colors = [1,0,0;0,1,0;0,0,1;0,0,0];
   for ii=1:udata.channels
      ydata = reshape(repmat(cdata(1,:,ii),2,1),[1,udata.imsize*2]);
      line('xdata',xdata,'ydata',ydata,'color',colors(min(ii,4),:),'linestyle','-');
   end
else
   ydata = reshape(repmat(cdata,2,1),[1,udata.imsize*2]);
   line('xdata',xdata,'ydata',ydata,'color',[0,0,0],'linestyle','-');
end
udata = stretchYaxis_1D(axh,udata);


%
% Set the Y-axis properties for 1D image
%
function udata = stretchYaxis_1D(axh,udata)
cdata = mapcomplexdata(udata.imagedata,udata.complexmapping);
logmode = 0;
if islogical(cdata)
   mappingmode = 'normal';
   currange = [-0.2,1.2];
else
   mappingmode = udata.mappingmode;
   switch mappingmode
      case 'angle'
         currange = [-pi,pi];
      case 'orientation'
         currange = [-pi,pi]/2;
      case 'log'
         % won't work if any pixel is non-finite, nor if there are non-positive values
         if ~isfield(udata,'computed') || ~isfield(udata.computed,'canlog')
            if any(~isfinite(cdata))
               udata.computed.canlog = 'non-finite values';
            elseif any(cdata<0)
               udata.computed.canlog = 'negative values';
            else
               udata.computed.canlog = '';
            end
         end
         if ~isempty(udata.computed.canlog)
            warning(['Image contains ',udata.computed.canlog,'. Cannot perform log stretch.'])
            mappingmode = 'normal';
            currange = [0,255];
         else
            logmode = 1;
            udata = set_udata_computed_lin(udata,cdata);
            currange = udata.computed.lin;
         end
      case 'base'
         if ~isfield(udata,'computed') || ~isfield(udata.computed,'base')
            udata = set_udata_computed_lin(udata,cdata);
            currange = udata.computed.lin;
            currange(2) = max(abs(currange));
            currange(1) = -currange(2);
            udata.computed.base = currange;
         else
            currange = udata.computed.base;
         end
      case 'percentile'
         udata = set_udata_computed_percentile(udata,cdata);
         currange = udata.computed.percentile;
      case 'lin'
         udata = set_udata_computed_lin(udata,cdata);
         currange = udata.computed.lin;
      case 'unit'
         currange = [0,1];
      case 'normal'
         currange = [0,255];
      case '12bit'
         currange = [0,4095];
      case '16bit'
         currange = [0,65535];
      case 's8bit'
         currange = [-128,127];
      case 's12bit'
         currange = [-2048,2047];
      case 's16bit'
         currange = [-32768,32767];
      case {'','manual'}
         % use existing currange
         currange = udata.currange;
         if length(currange)~=2
            currange = [0,255];
            mappingmode = 'normal';
         end
      otherwise
         currange = [0,255];
         mappingmode = 'normal';
   end
   if currange(1) == currange(2)
      currange = currange + [-1,1];
   elseif currange(1) > currange(2)
      currange = currange([2,1]);
   end
end
udata.mappingmode = mappingmode;
udata.currange = currange;
set(axh,'Ylim',currange);
if logmode
   set(axh,'YScale','log');
else
   set(axh,'YScale','lin');
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
if nD>=3
   udata = get(fig,'UserData');
   newslice = 0;
   newtime = 0;
   globalstretch = dipgetpref('DefaultGlobalStretch');
   if isfield(udata,'curslice')
      newslice = udata.curslice;
   end
   if isfield(udata,'linkdisplay')
      linkdisplay = udata.linkdisplay;
   end
   if isfield(udata,'globalstretch') % We don't want global stretching on color images yet.
      globalstretch = udata.globalstretch;
   end
   udata = [];
elseif nD==0
   sz = [1,1];
   nD = 2; % Default figure: nice menus and stuff.
end
% Set up figure window.
clearnreset(fig);
axh = axes('Parent',fig,'Visible','off','XGrid','off','YGrid','off','YDir','reverse',...
   'PlotBoxAspectRatioMode','auto','Units','normalized','Position',[0 0 1 1]);
imh = image('BusyAction','cancel','Parent',axh,'Interruptible','off','CDataMapping','direct','CData',[]);
create_menus(fig,nD,iscomp,iscol,isbin);
change_keyboard_state(fig,keys);
% Create UserData stuff
udata.state = ''; % Make sure the state is updated later on.
udata.currange = currange;
udata.mappingmode = mappingmode;
udata.colmap = colmap;
udata.complexmapping = complexmapping;
udata.colspace = colorspace(in);
udata.linkdisplay = linkdisplay;
if nD==3
   if isempty(udata.colspace) || strcmp(udata.colspace,'RGB')
      udata.globalstretch = globalstretch;
   else
      if globalstretch
         warning('Cannot perform global stretching on non-RGB color images.')
      end
      udata.globalstretch = 0;
   end
   switch slicing
      case 'yz'
         udata.maxslice = sz(1)-1;
         udata.imsize = sz([2,3,1]);
      case 'xz'
         udata.maxslice = sz(2)-1;
         udata.imsize = sz([1,3,2]);
      otherwise % case 'xy', ''
         slicing = 'xy';
         udata.maxslice = sz(3)-1;
         udata.imsize = sz([1,2,3]);
   end
   udata.slices = in;
   if iscol
      udata.channels = length(in);
   end
   udata.curslice = -1;  % else UPDATE_SLICE might not update
   udata.slicing = slicing;

   % Show 3D image slice
   udata = update_slice(fig,imh,udata,newslice);
elseif nD==4
   if isempty(udata.colspace) || strcmp(udata.colspace,'RGB')
      udata.globalstretch = globalstretch;
   else
      if globalstretch
         warning('Cannot perform global stretching on non-RGB color images.')
      end
      udata.globalstretch = 0;
   end
   switch slicing
      case 'yz'
         udata.maxslice = sz(1)-1;%step n/p
         udata.maxtime  = sz(4)-1;%step b/f
         udata.imsize = sz([2,3,1,4]);
      case 'xz'
         udata.maxslice = sz(2)-1;
         udata.maxtime  = sz(4)-1;
         udata.imsize = sz([1,3,2,4]);
      case 'xt'
         udata.maxslice = sz(2)-1;
         udata.maxtime  = sz(3)-1;
         udata.imsize = sz([1,4,3,2]);
      case 'yt'
         udata.maxslice = sz(1)-1;
         udata.maxtime  = sz(3)-1;
         udata.imsize = sz([2,4,1,3]);
      case 'zt'
         udata.maxslice = sz(1)-1;
         udata.maxtime  = sz(2)-1;
         udata.imsize = sz([3,4,1,2]);
      otherwise % case 'xy', ''
         slicing = 'xy';
         udata.maxslice = sz(3)-1;
         udata.imsize = sz([1,2,3,4]);
         udata.maxtime = sz(4)-1;
   end
   udata.slices = in;
   if iscol
      udata.channels = length(in);
   end
   udata.curslice = -1;  % else UPDATE_SLICE might not update
   udata.curtime = -1;
   udata.slicing = slicing;
   % Show 4D image slice
   udata = update_slice(fig,imh,udata,[newslice,newtime]);
else
   udata.imsize = sz;
   if iscol
      udata.channels = length(in);
      udata.colordata = in;
      udata.imagedata = cat(3,colorspace(in,'RGB'));
   else
      udata.imagedata = in;
   end
   udata.slicing = '';
   % Show 2D image
   udata = display_data(fig,imh,udata);
end
if isbin
   colmapdata = [0,0,0;dipgetpref('BinaryDisplayColor')];
else
   if isempty(colmapdata)
      [colmap,colmapdata] = parse_colmapstr(udata.colmap);
   end
end
set(fig,'Colormap',colmapdata);
% Set axes and image properties
set(imh,'XData',[0,udata.imsize(1)-1],'YData',[0,udata.imsize(2)-1]);
set(axh,'XLim',[0,udata.imsize(1)]-0.5,'Ylim',[0,udata.imsize(2)]-0.5);
% Set figure properties
set_mode_check(fig,udata.mappingmode,udata.colmap,udata.complexmapping,udata.slicing);
if nD>=3
   set_global_check(fig,udata.globalstretch);
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
% Show a new slice of a 3D/4D image.
%
function udata = update_slice(fig,imh,udata,new_slice)

newslice = new_slice(1); %--BR----changed lastp parameter in function to [newslice newtime]---
if newslice > udata.maxslice
   newslice = udata.maxslice;
elseif newslice < 0
   newslice = 0;
end
is4D = length(udata.imsize)==4;
if ~is4D
   if newslice == udata.curslice
      return
   end
   switch udata.slicing
      case 'yz'
         udata.imagedata = squeeze(udata.slices(newslice,:,:));
      case 'xz'
         udata.imagedata = squeeze(udata.slices(:,newslice,:));
      otherwise % case 'xy'
         udata.imagedata = squeeze(udata.slices(:,:,newslice));
   end
else
   newtime = new_slice(2);
   if newtime > udata.maxtime
      newtime = udata.maxtime;
   elseif newtime < 0
      newtime = 0;
   end
   if (newslice == udata.curslice) && (newtime == udata.curtime)
      return
   end
   switch udata.slicing
      case 'yz'
         udata.imagedata = squeeze(udata.slices(newslice,:,:,newtime));
      case 'xz'
         udata.imagedata = squeeze(udata.slices(:,newslice,:,newtime));
      case 'xt'
         udata.imagedata = squeeze(udata.slices(:,newslice,newtime,:));
      case 'yt'
         udata.imagedata = squeeze(udata.slices(newslice,:,newtime,:));
      case 'zt'
         udata.imagedata = squeeze(udata.slices(newslice,newtime,:,:));
      otherwise % case 'xy'
         udata.imagedata = squeeze(udata.slices(:,:,newslice,newtime));
   end
end
if iscolor(udata.imagedata)
   udata.colordata = udata.imagedata;
   udata.imagedata = cat(3,colorspace(udata.colordata,'RGB'));
end
if ~udata.globalstretch && isfield(udata,'computed')
   udata = rmfield(udata,'computed');
end
udata.curslice = newslice;
if is4D
   udata.curtime = newtime;
end
udata = display_data(fig,imh,udata);


%
% Update slices in linked displays
%
function hlist = update_linked(fig,hlist,newslice,newslicing,curzoom)
%input of newslice,newslicing is only required for 3D, 4D images

keep = ones(size(hlist));
curax = findobj(fig,'Type','axes');
curxlim = get(curax,'xlim');
curylim = get(curax,'ylim');

for ii=1:length(hlist)
   if ~isfigh(hlist(ii)) || (~strncmp(get(hlist(ii),'Tag'),'DIP_Image_2D',12) ...
      && ~strncmp(get(hlist(ii),'Tag'),'DIP_Image_3D',12) && ~strncmp(get(hlist(ii),'Tag'),'DIP_Image_4D',12))
      disp(['Warning: the linked display ',num2str(hlist(ii)),' is no longer 2/3/4D.']);
      keep(ii) = 0;
   else
      udata = get(hlist(ii),'UserData');
      if ~isempty(newslice)
         if length(udata.imsize)==3 %3D display
            if ~strcmp(udata.slicing,newslicing)
               change_mapping(hlist(ii),'ch_slice',newslice,'ch_slicing',newslicing);
            elseif udata.curslice ~= newslice
               change_mapping(hlist(ii),'ch_slice',newslice);
            end
         elseif length(udata.imsize)==4 %4D display
            if ~strcmp(udata.slicing,newslicing)
               change_mapping(hlist(ii),'ch_slice',newslice(1),'ch_time',newslice(2),'ch_slicing',newslicing);
            elseif udata.curslice ~= newslice(1) || udata.curtime ~=newslice(2)
               change_mapping(hlist(ii),'ch_slice',newslice(1),'ch_time',newslice(2));
            end
         end
      end
      %change the axis and zoom also for 2D data
      ax = findobj(hlist(ii),'Type','axes');
      if udata.zoom ~= curzoom
         ax = findobj(hlist(ii),'Type','axes');
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
      end
      ax = findobj(hlist(ii),'Type','axes');
      set(ax,'xlim',curxlim);
      set(ax,'ylim',curylim);
   end
end
hlist = hlist(find(keep));
if isempty(hlist)
   diplink(fig,'off');
end


%
% Calculate the display data for a 2D image / slice
%
function udata = display_data(fig,imh,udata)
cdata = udata.imagedata;
iscol = ~isempty(udata.colspace);
if ~iscol && islogical(cdata)
   cdata = uint8(cdata);
   mappingmode = 'normal';
   currange = [0,1];
else
   params.mode = 'lin';
   params.complex = udata.complexmapping;
   params.projection = 'slice';
   mappingmode = udata.mappingmode;
   switch mappingmode
      case 'angle'
         currange = [-pi,pi];
      case 'orientation'
         currange = [-pi,pi]/2;
      case 'log'
         udata = set_udata_computed_lin(udata,cdata);
         currange = udata.computed.lin;
         params.mode = 'log';
      case 'base'
         if ~isfield(udata,'computed') || ~isfield(udata.computed,'lin')
            udata = set_udata_computed_lin(udata,cdata);
         end
         currange = udata.computed.lin;
         params.mode = 'based';
      case 'percentile'
         udata = set_udata_computed_percentile(udata,cdata);
         currange = udata.computed.percentile;
      case 'lin'
         udata = set_udata_computed_lin(udata,cdata);
         currange = udata.computed.lin;
      case 'unit'
         currange = [0,1];
      case 'normal'
         currange = [0,255];
      case '12bit'
         currange = [0,4095];
      case '16bit'
         currange = [0,65535];
      case 's8bit'
         currange = [-128,127];
      case 's12bit'
         currange = [-2048,2047];
      case 's16bit'
         currange = [-32768,32767];
      case {'','manual'}
         % use existing currange
         currange = udata.currange;
         if length(currange)~=2
            currange = [0,255];
            mappingmode = 'normal';
         end
      otherwise % not supposed to happen
         currange = [0,255];
         mappingmode = 'normal';
   end
   if currange(1) == currange(2)
      currange = currange + [-1,1];
   elseif currange(1) > currange(2)
      currange = currange([2,1]);
   end
   params.lowerBound = currange(1);
   params.upperBound = currange(2);
   %if iscol
      %g = dipgetpref('Gamma');
   %else
      %g = dipgetpref('GammaGrey');
   %end
   % TODO: use `g` in `imagedisplay`.
   cdata = dip_array(imagedisplay(cdata,[],0,1,params));
end
set(imh,'cdata',cdata);
udata.mappingmode = mappingmode;
udata.currange = currange;


%
% Fill-in udata.computed elements, if they don't exist yet.
%
function udata = set_udata_computed_lin(udata,cdata)
if ~isfield(udata,'computed') || ~isfield(udata.computed,'lin')
   if length(udata.imsize)>=3 && udata.globalstretch
      if ~isempty(udata.colspace) && ~strcmp(udata.colspace,'RGB')
         warning('Cannot perform global stretching on non-RGB color images.')
         % Don't worry about this. It cannot happen.
      else
         cdata = udata.slices;
         cdata = mapcomplexdata(cdata,udata.complexmapping);
      end
   end
   udata.computed.lin = getmaximumandminimum(cdata);
end

function udata = set_udata_computed_percentile(udata,cdata)
if ~isfield(udata,'computed') || ~isfield(udata.computed,'percentile')
   if length(udata.imsize)>=3 && udata.globalstretch
      if ~isempty(udata.colspace) && ~strcmp(udata.colspace,'RGB')
         warning('Cannot perform global stretching on non-RGB color images.')
         % Don't worry about this. It cannot happen.
      else
         cdata = udata.slices;
         cdata = mapcomplexdata(cdata,udata.complexmapping);
      end
   end
   udata.computed.percentile = [percentile(cdata,5),percentile(cdata,95)];
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
   'ResizeFcn','dipshow DIP_callback ResizeFcn',...
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
uimenu(h,'Label','Save display ...','Tag','save','Callback','dipshow DIP_callback menu_file_save_cb',...
       'Accelerator','s');
if ~isunix
   uimenu(h,'Label','Copy display','Tag','copy','Callback','print(gcbf,''-dbitmap'')','Accelerator','c');
end
uimenu(h,'Label','Clear','Tag','clear','Callback','dipclf(gcbf)','Separator','on','Accelerator','x');
uimenu(h,'Label','Close','Tag','close','Callback','close(gcbf)','Separator','on','Accelerator','w');
% Create 'Sizes' menu
h = uimenu(fig,'Label','Sizes','Tag','sizes');
uimenu(h,'Label','10%','Tag','10','Callback','diptruesize(gcbf,10)');
uimenu(h,'Label','25%','Tag','25','Callback','diptruesize(gcbf,25)');
uimenu(h,'Label','50%','Tag','50','Callback','diptruesize(gcbf,50)','Accelerator','5');
uimenu(h,'Label','100%','Tag','100','Callback','diptruesize(gcbf,100)','Accelerator','1');
uimenu(h,'Label','200%','Tag','200','Callback','diptruesize(gcbf,200)','Accelerator','2');
uimenu(h,'Label','400%','Tag','400','Callback','diptruesize(gcbf,400)');
uimenu(h,'Label','1000%','Tag','1000','Callback','diptruesize(gcbf,1000)');
uimenu(h,'Label','Stretch to fill','Tag','off','Callback','diptruesize(gcbf,''off'')','Accelerator','0');
uimenu(h,'Label','Default window size','Tag','defaultsize','Separator','on','Callback',...
         ['set(gcbf,''Units'',''pixels'');DIP_figure_CB_pos=get(gcbf,''position'');',...
          'DIP_figure_CB_pos(2)=DIP_figure_CB_pos(2)+DIP_figure_CB_pos(4);',...
          'DIP_figure_CB_pos(3:4)=[dipgetpref(''DefaultFigureWidth''),dipgetpref(''DefaultFigureHeight'')];',...
          'DIP_figure_CB_pos(2)=DIP_figure_CB_pos(2)-DIP_figure_CB_pos(4);',...
          'set(gcbf,''position'',DIP_figure_CB_pos);clear DIP_figure_CB_pos']);
% Create 'Mappings' menu
h = uimenu(fig,'Label','Mappings','Tag','mappings');
if isbin
   uimenu(h,'Label','Normal','Tag','normal','Callback',...
          'dipshow(gcbf,''ch_mappingmode'',''normal'')');
else
   uimenu(h,'Label','Unit [0,1]','Tag','unit','Callback',...
          'dipshow(gcbf,''ch_mappingmode'',''unit'')');
   uimenu(h,'Label','Normal [0,255]','Tag','normal','Callback',...
          'dipshow(gcbf,''ch_mappingmode'',''normal'')');
   uimenu(h,'Label','12-bit [0,4095]','Tag','12bit','Callback',...
          'dipshow(gcbf,''ch_mappingmode'',''12bit'')');
   uimenu(h,'Label','16-bit [0,65535]','Tag','16bit','Callback',...
          'dipshow(gcbf,''ch_mappingmode'',''16bit'')');
   uimenu(h,'Label','Linear stretch','Tag','lin','Callback',...
          'dipshow(gcbf,''ch_mappingmode'',''lin'')','Accelerator','L');
   uimenu(h,'Label','Percentile stretch','Tag','percentile','Callback',...
          'dipshow(gcbf,''ch_mappingmode'',''percentile'')');
   uimenu(h,'Label','Log stretch','Tag','log','Callback',...
          'dipshow(gcbf,''ch_mappingmode'',''log'')');
   uimenu(h,'Label','Based at 0','Tag','base','Callback',...
          'dipshow(gcbf,''ch_mappingmode'',''base'')');
   uimenu(h,'Label','Angle','Tag','angle','Callback',...
          'dipshow(gcbf,''ch_mappingmode'',''angle'')');
   uimenu(h,'Label','Orientation','Tag','orientation','Callback',...
          'dipshow(gcbf,''ch_mappingmode'',''orientation'')');
   uimenu(h,'Label','Manual ...','Tag','manual','Callback',...
          'dipmapping(gcbf,''manual'')');
   if nD>=3
      uimenu(h,'Label','Global stretch','Tag','globalstretch','Callback',...
             'dipshow(gcbf,''ch_globalstretch'',1)','Separator','on');
   end
   if ~iscol && nD>1
      uimenu(h,'Label','Grey','Tag','grey','Callback',...
             'dipshow(gcbf,''ch_colormap'',''grey'')','Separator','on');
      uimenu(h,'Label','Saturation','Tag','saturation','Callback',...
             'dipshow(gcbf,''ch_colormap'',''saturation'')');
      uimenu(h,'Label','Zero-based','Tag','zerobased','Callback',...
             'dipshow(gcbf,''ch_colormap'',''zerobased'')');
      uimenu(h,'Label','Periodic','Tag','periodic','Callback',...
             'dipshow(gcbf,''ch_colormap'',''periodic'')');
      uimenu(h,'Label','Labels','Tag','labels','Callback',...
             'dipshow(gcbf,''ch_mappingmode'',''labels'')');
      uimenu(h,'Label','Custom...','Tag','custom','Callback',...
             'dipmapping(gcbf,''custom'')');
   end
   if iscomp
      uimenu(h,'Label','Magnitude','Tag','abs','Callback',...
             'dipshow(gcbf,''ch_complexmapping'',''abs'')','Separator','on');
      uimenu(h,'Label','Phase','Tag','phase','Callback',...
             'dipshow(gcbf,''ch_complexmapping'',''phase'')');
      uimenu(h,'Label','Real part','Tag','real','Callback',...
             'dipshow(gcbf,''ch_complexmapping'',''real'')');
      uimenu(h,'Label','Imaginary part','Tag','imag','Callback',...
             'dipshow(gcbf,''ch_complexmapping'',''imag'')');
   end
end
if nD>=3
   uimenu(h,'Label','X-Y slice','Tag','xy','Callback',...
          'dipshow(gcbf,''ch_slicing'',''xy'')','Separator','on');
   uimenu(h,'Label','X-Z slice','Tag','xz','Callback',...
          'dipshow(gcbf,''ch_slicing'',''xz'')');
   uimenu(h,'Label','Y-Z slice','Tag','yz','Callback',...
          'dipshow(gcbf,''ch_slicing'',''yz'')');
end
if nD==4
   uimenu(h,'Label','X-T slice','Tag','xt','Callback',...
          'dipshow(gcbf,''ch_slicing'',''xt'')');
   uimenu(h,'Label','Y-T slice','Tag','yt','Callback',...
          'dipshow(gcbf,''ch_slicing'',''yt'')');
   uimenu(h,'Label','Z-T slice','Tag','zt','Callback',...
          'dipshow(gcbf,''ch_slicing'',''zt'')');
end
% TODO: add slice / max projection / mean projection as options for nD>=3

% Create 'Actions' menu
h = uimenu(fig,'Label','Actions','Tag','actions');
uimenu(h,'Label','None','Tag','mouse_none','Callback',...
       'dipshow DIP_callback menu_actions_none_cb');
uimenu(h,'Label','Pixel testing','Tag','mouse_diptest','Callback','diptest(gcbf,''on'')',...
       'Accelerator','i');
uimenu(h,'Label','Zoom','Tag','mouse_dipzoom','Callback','dipzoom(gcbf,''on'')',...
       'Accelerator','z');
uimenu(h,'Label','Looking Glass','Tag','mouse_diplooking','Callback','diplooking(gcbf,''on'')');
uimenu(h,'Label','Pan','Tag','mouse_dippan','Callback','dippan(gcbf,''on'')',...
       'Accelerator','p');
if nD>=2
      uimenu(h,'Label','Link displays ...','Tag','diplink','Callback','diplink(gcbf,''on'')',...
          'Separator','on');
   % too lazy to also include it for 1D images (BR)
end
if nD>=3
   uimenu(h,'Label','Step through slices','Tag','mouse_dipstep','Callback','dipstep(gcbf,''on'')',...
          'Accelerator','o');
   uimenu(h,'Label','Animate','Tag','dipanimate','Callback','dipanimate(gcbf)');
   if nD==3 && ~iscol
      uimenu(h,'Label','Isosurface plot ...','Tag','dipiso','Callback','dipisosurface(gcbf)');
   end

end
if nD > 1 && exist('javachk','file') && isempty(javachk('jvm'))
   uimenu(h,'Label','View5d','Tag','viewer5d','Callback','view5d(gcbf);');
end
uimenu(h,'Label','Enable keyboard','Tag','keyboard','Separator','on',...
         'Callback','dipshow DIP_callback menu_actions_keyboard_cb','Accelerator','k');
% Work around a bug in MATLAB 7.1 (R14SP3) (solution # 1-1XPN82).
h = uimenu(fig);drawnow;delete(h);


%
% Find the correct action state for the current image
%
function state = find_action_state(state,nD,iscomp,iscol)
switch state
   case 'diptest'
   case 'diporien'
      if nD~=2 || iscol, state = 'none'; end
   case 'dipzoom'
   case 'dippan'
   case 'dipstep'
      if nD~=3 && nD~=4
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
   set(findobj(m,'Tag',slicing),'Checked','on');
end


%
% Set the checkmark to the global stretch menu item
% (always call right after set_mode_check, which resets this one...)
%
function set_global_check(fig,globalstretch)
m = findobj(get(fig,'Children'),'Tag','globalstretch');
if globalstretch
   set(m,'Checked','on','Callback','dipshow(gcbf,''ch_globalstretch'',0)');
else
   set(m,'Checked','off','Callback','dipshow(gcbf,''ch_globalstretch'',1)');
end


%
% Set the mapping modes (does some user-input parsing too)
%
function change_mapping(fig,varargin)
N = nargin-1;
currange = [];
complexmapping = [];
slicing = [];
newtime =[];
globalstretch = [];
newslice = [];
colmap = [];
ii = 1;

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
      case 'ch_colormap'
         colmap = varargin{ii};
         if ~ischar(colmap) && ~isnumeric(colmap)
            error('Illegal argument for colormap')
         end
      case 'ch_complexmapping'
         complexmapping = varargin{ii};
         if ~ischar(complexmapping) || ~any(strcmp(complexmapping,{'abs','real','imag','phase'}))
            error('Illegal argument for complexmapping')
         end
      case 'ch_slicing'
         slicing = varargin{ii};
         if ~ischar(slicing) || ...
            ~any(strcmp(slicing,{'xy','xz','yz','xt','yt','zt'}))
            error('Illegal argument for slice mapping.')
         end
      case 'ch_globalstretch'
         globalstretch = varargin{ii};
         if isnumeric(globalstretch) && length(globalstretch)==1
            if globalstretch
               globalstretch = 1;
            else
               globalstretch = 0;
            end
         elseif ischar(globalstretch)
            switch lower(globalstretch)
            case {'on','yes'}
               globalstretch = 1;
            case {'off','no'}
               globalstretch = 0;
            otherwise
               globalstretch = [];
            end
         end
         if isempty(globalstretch)
            error('Illegal argument for global stretch')
         end
      case 'ch_slice'
         newslice = varargin{ii};
         if ~isnumeric(newslice) || length(newslice)>1
            error('Illegal argument for slice number selction')
         end
      case  'ch_time'
         %cannot just do this in ch_slice as switching is not possible
         %on array [slice time]
         newtime = varargin{ii};
         if ~isnumeric(newtime) || length(newtime)>1
            error('Illegal argument for time number selction')
         end
      case 'updatelinked'
         udata = get(fig,'UserData');
         slice = [];
         nD = length(udata.imsize);
         if nD>=3
            slice = udata.curslice;
            if nD>=4
               slice(2) = udata.curtime;
            end
         end
         udata.linkdisplay = update_linked(fig,udata.linkdisplay,slice,udata.slicing,udata.zoom);
         return;
      otherwise
         error('Illegal argument to change mapping in dipshow.')
   end
   ii = ii+1;
end

disp1D = 0;
disp3D = 0;    % re-desplay 3D data
disp3Dx = 0;   % re-desplay 3D data and adjust axes
udata = get(fig,'UserData');
if ~isempty(currange)
   if isempty(colmap)
      [udata.currange,udata.mappingmode,colmap] = parse_rangestr(currange);
      if strcmp(colmap,udata.colmap)
         colmap = [];
      elseif strcmp(colmap,'grey') && ~strcmp(udata.colmap,'labels')
         colmap = [];
      end
   else
      [udata.currange,udata.mappingmode] = parse_rangestr(currange);
   end
end
if ~isempty(colmap)
   [udata.colmap,colmap] = parse_colmapstr(colmap);
end
if ~isempty(complexmapping)
   if ~strcmp(udata.complexmapping,complexmapping)
      if ~isreal(udata.imagedata)
         % Switch udata.computed around for different complexmapping modes.
         if isfield(udata,'computed')
            udata = setfield(udata,['computed_',udata.complexmapping],udata.computed);
         end
         if isfield(udata,['computed_',complexmapping])
            udata.computed = getfield(udata,['computed_',complexmapping]);
         elseif isfield(udata,'computed')
            udata = rmfield(udata,'computed');
         end
         disp1D = 1;
      end
      udata.complexmapping = complexmapping; % We set this value even if the image is real.
   end
end
if ~isempty(slicing)
   if ~strcmp(udata.slicing,slicing)
      udata.slicing = slicing;
      disp3D = 1;
      disp3Dx = 1;
   end
end
if ~isempty(globalstretch)
   if length(udata.imsize)<3
      globalstretch = [];
   elseif ~isempty(udata.colspace) && ~strcmp(udata.colspace,'RGB')
      % weird set of elseifs: this way the warning is fired at the right moment.
      warning('Cannot perform global stretching on non-RGB color images.')
      globalstretch = [];
   elseif udata.globalstretch == globalstretch;
      globalstretch = [];
   end
end
if ~isempty(newslice) || ~isempty(newtime)
   disp3D = 1;
end

if length(udata.imsize)==1 %1D display
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
   imh = findobj(fig,'Type','image');
   if length(imh)~=1, return, end
   if ~isempty(globalstretch)
      % Changing globalstretch
      udata.globalstretch = globalstretch;
      if globalstretch
         % Turning on: get globally computed values if available, delete previous local ones
         if isreal(udata.imagedata)
            if isfield(udata,'globalcomputed')
               udata.computed = udata.globalcomputed.computed;
            elseif isfield(udata,'computed')
               udata = rmfield(udata,'computed');
            end
         else
            fn = fieldnames(udata);
            fn = fn(find(strncmp(fn,'computed',8)));
            for ii=1:length(fn)
               udata = rmfield(udata,fn{ii});
            end
            if isfield(udata,'globalcomputed')
               fn = fieldnames(udata.globalcomputed);
               for ii=1:length(fn)
                  udata = setfield(udata,fn{ii},getfield(udata.globalcomputed,fn{ii}));
               end
            end
            if isfield(udata,['computed_',udata.complexmapping])
               udata.computed = getfield(udata,['computed_',udata.complexmapping]);
            end
         end
      else
         % Turning off: store globally computed values & delete locally
         if isreal(udata.imagedata)
            if isfield(udata,'computed')
               udata.globalcomputed.computed = udata.computed;
               udata = rmfield(udata,'computed');
            end
         else
            if isfield(udata,'computed')
               udata = setfield(udata,['computed_',udata.complexmapping],udata.computed);
               udata = rmfield(udata,'computed');
            end
            fn = fieldnames(udata);
            fn = fn(find(strncmp(fn,'computed',8)));
            udata.globalcomputed = [];
            for ii=1:length(fn)
               udata.globalcomputed = setfield(udata.globalcomputed,fn{ii},getfield(udata,fn{ii}));
               udata = rmfield(udata,fn{ii});
            end
         end
      end
   end

   if disp3D && length(udata.imsize)==3
      % 3D slice change
      axh = get(imh,'Parent');
      sz = imsize(udata.slices);
      switch udata.slicing
         case 'yz'
            udata.maxslice = sz(1)-1;
            udata.imsize = sz([2,3,1]);
         case 'xz'
            udata.maxslice = sz(2)-1;
            udata.imsize = sz([1,3,2]);
         otherwise % case 'xy', ''
            slicing = 'xy';
            udata.maxslice = sz(3)-1;
            udata.imsize = sz([1,2,3]);
      end
      if isempty(newslice)
         newslice = udata.curslice;
      end
      udata.curslice = -1;  % else UPDATE_SLICE might not update
      udata = update_slice(fig,imh,udata,newslice);
      if disp3Dx
         set(imh,'XData',[0,udata.imsize(1)-1],'YData',[0,udata.imsize(2)-1]);
         set(axh,'XLim',[0,udata.imsize(1)]-0.5,'Ylim',[0,udata.imsize(2)]-0.5);
      end
   elseif disp3D && length(udata.imsize)==4
      axh = get(imh,'Parent');
      sz = imsize(udata.slices);
      switch udata.slicing
      case 'yz'
         udata.maxslice = sz(1)-1;%step n/p
         udata.maxtime  = sz(4)-1;%step b/f
         udata.imsize = sz([2,3,1,4]);
      case 'xz'
         udata.maxslice = sz(2)-1;
         udata.maxtime  = sz(4)-1;
         udata.imsize = sz([1,3,2,4]);
      case 'xt'
         udata.maxslice = sz(2)-1;
         udata.maxtime  = sz(3)-1;
         udata.imsize = sz([1,4,3,2]);
      case 'yt'
         udata.maxslice = sz(1)-1;
         udata.maxtime  = sz(3)-1;
         udata.imsize = sz([2,4,1,3]);
      case 'zt'
         udata.maxslice = sz(1)-1;
         udata.maxtime  = sz(2)-1;
         udata.imsize = sz([3,4,1,2]);
      otherwise % case 'xy', ''
         slicing = 'xy';
         udata.maxslice = sz(3)-1;
         udata.imsize = sz([1,2,3,4]);
         udata.maxtime = sz(4)-1;
      end
      if isempty(newslice)
         newslice = udata.curslice;
      end
      if isempty(newtime)
         newtime = udata.curtime;
      end
      udata.curslice = -1;  % else UPDATE_SLICE might not update
      udata.curtime = -1;
      udata = update_slice(fig,imh,udata,[newslice newtime]);
      if disp3Dx
         set(imh,'XData',[0,udata.imsize(1)-1],'YData',[0,udata.imsize(2)-1]);
         set(axh,'XLim',[0,udata.imsize(1)]-0.5,'Ylim',[0,udata.imsize(2)]-0.5);
      end
   else
      % 2D/3D range change
      udata = display_data(fig,imh,udata);
   end
   if ~isempty(colmap)
      set(fig,'Colormap',colmap);
   end
end
set_mode_check(fig,udata.mappingmode,udata.colmap,udata.complexmapping,udata.slicing);
if length(udata.imsize)>=3
   set_global_check(fig,udata.globalstretch)
end
set(fig,'UserData',[]);    % Solve MATLAB bug!
set(fig,'UserData',udata);
if disp3Dx && ~isempty(udata.zoom)
   diptruesize(fig,udata.zoom*100);
else
   dipfig_titlebar(fig,udata);
end
if disp3D && length(udata.imsize)==3 && ~isempty(udata.linkdisplay)
   newlinks = update_linked(fig,udata.linkdisplay,udata.curslice,udata.slicing,udata.zoom);
   if ~isequal(newlinks,udata.linkdisplay)
      udata.linkdisplay = newlinks;
      set(fig,'UserData',[]);    % Solve MATLAB bug!
      set(fig,'UserData',udata);
   end
end
if disp3D && length(udata.imsize)==4 && ~isempty(udata.linkdisplay)
   newlinks = update_linked(fig,udata.linkdisplay,[udata.curslice udata.curtime],udata.slicing,udata.zoom);
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
   set(fig,'KeyPressFcn','dipshow DIP_callback KeyPressFcn');
   set(findobj(fig,'tag','keyboard'),'Checked','on');
else
   set(fig,'KeyPressFcn','');
   set(findobj(fig,'tag','keyboard'),'Checked','off');
end


%
% Callback functions for File->Save... menu item
%
function save_figure_window(fig)
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
   [tmp,tmp,ext] = fileparts(filename);
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
function dipzoomWindowButtonDownFcn(fig)
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
   if length(udata.imsize)==1
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
   set(fig,'WindowButtonMotionFcn','dipshow DIP_callback dipzoomWindowButtonMotionFcn',...
           'WindowButtonUpFcn','dipshow DIP_callback dipzoomWindowButtonUpFcn',...
           'NumberTitle','off',...
           'UserData',[]);   % Solve MATLAB bug!
   set(fig,'UserData',udata);
   dipzoomUpdateDisplay(fig,ax,udata);
end

function dipzoomWindowButtonMotionFcn(fig)
if strncmp(get(fig,'Tag'),'DIP_Image',9)
   udata = get(fig,'UserData');
   dipzoomUpdateDisplay(fig,udata.ax,udata);
end

function dipzoomWindowButtonUpFcn(fig)
if strncmp(get(fig,'Tag'),'DIP_Image',9)
   udata = get(fig,'UserData');
   delete(udata.recth);
   pt = dipfig_getcurpos(udata.ax);
   if length(udata.imsize)==1
      pt = pt(1);
      udata.coords = udata.coords(1);
   end
   if abs(pt-udata.coords) > 2
      % Dragged a rectangle
      axpos = get(udata.ax,'Position');
      if length(udata.imsize)==1
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
            %if length(udata.imsize)==1
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
if length(udata.imsize) == 1
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
pt = min(pt,udata.imsize(1:2)-1);
% Again constrain proportions, this time take smallest rectangle
direction = sign(pt-udata.coords);
delta = abs(pt-udata.coords)+1;
delta = ceil(min(delta./udata.figsz)*udata.figsz);
pt = udata.coords + (delta-1).*direction;

function dipzoomZoom(zoom,pt,ax,udata,winsize)
axpos = get(ax,'Position');
dispsize = winsize;
if length(udata.imsize) == 1
   curxlim = get(ax,'XLim');
   pelsize = axpos(3)/diff(curxlim);
   imsize = udata.imsize;
   winsize = winsize(1);
else
   axsize = axpos([3,4]);
   axpos = axpos([1,2]);
   curxlim = get(ax,'XLim'); curxrange = diff(curxlim);
   curylim = get(ax,'YLim'); curyrange = diff(curylim);
   pelsize = [(axsize(1)/curxrange),(axsize(2)/curyrange)];
   imsize = udata.imsize([1,2]);
end
newpelsize = pelsize*zoom;
if zoom==0
   newpelsize(:) = 1;
end
sz = min(imsize,ceil(winsize./(newpelsize)));
sz = max(sz,1); % Minimum image size: 1 pixel.
pt = round(pt-sz/2);
pt = max(pt,0);
pt = min(pt,imsize-sz);
if length(udata.imsize) == 1
   set(ax,'XLim',pt+[0,sz]-0.5);
else
   set(ax,'XLim',pt(1)+[0,sz(1)]-0.5,'YLim',pt(2)+[0,sz(2)]-0.5);
end
position_axes(ax,newpelsize,sz,dispsize);


%
% Callback function for DIPSTEP
%
function dipstepWindowButtonDownFcn(fig)
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
   udata.startslice = udata.curslice;
   if length(udata.imsize)==4
      udata.starttime = udata.curtime;
   end
   udata.moved = 0;
   set(fig,'WindowButtonMotionFcn','dipshow DIP_callback dipstepWindowButtonMotionFcn',...
           'WindowButtonUpFcn','dipshow DIP_callback dipstepWindowButtonUpFcn',...
           'UserData',[]);   % Solve MATLAB bug!
   set(fig,'UserData',udata);
end

function dipstepWindowButtonMotionFcn(fig)
if strncmp(get(fig,'Tag'),'DIP_Image_3D',12) || strncmp(get(fig,'Tag'),'DIP_Image_4D',12)
   udata = get(fig,'UserData');
   pt = get(0,'PointerLocation');
   pt = [pt(1,1),-pt(1,2)];
   delta = (pt-udata.coords)/3; % move one slice for each 3 pixel cursor movement
   [dir,dir] = max(abs(delta));
   delta = round(delta(dir));
   udata.moved = 1;
   if length(udata.imsize)==4
      switch get(fig,'SelectionType')
         case 'alt'
            newslice = [udata.startslice,udata.starttime+delta];
         otherwise
            newslice = [udata.startslice+delta,udata.starttime];
      end
   else
      newslice = udata.startslice+delta;
   end
   udata = update_slice(fig,udata.imh,udata,newslice);
   dipfig_titlebar(fig,udata);
   set(fig,'UserData',[]);   % Solve MATLAB bug!
   set(fig,'UserData',udata);
end

function dipstepWindowButtonUpFcn(fig)
udata = get(fig,'UserData');
if strncmp(get(fig,'Tag'),'DIP_Image_3D',12) || strncmp(get(fig,'Tag'),'DIP_Image_4D',12)
   if ~udata.moved
      switch get(fig,'SelectionType')
         case {'normal','extend'}
            newslice = udata.curslice+1;
            udata.prevclick = 1;
         case 'alt'
            newslice = udata.curslice-1;
            udata.prevclick = -1;
         case 'open' %double-click: repeat last click
            newslice = udata.curslice+udata.prevclick;
         otherwise
            return
      end
      if length(udata.imsize)==4
         newslice = [newslice,udata.curtime];
      end
      udata = update_slice(fig,udata.imh,udata,newslice);
      dipfig_titlebar(fig,udata);
   end
   if ~isempty(udata.linkdisplay)
      if length(udata.imsize)==3
         udata.linkdisplay = update_linked(fig,udata.linkdisplay,udata.curslice,udata.slicing,udata.zoom);
      else
         udata.linkdisplay = update_linked(fig,udata.linkdisplay,[udata.curslice udata.curtime],udata.slicing,udata.zoom);
      end
   end
   % Clean up
   set(udata.ax,'Units',udata.oldAxesUnits);
   set(fig,'WindowButtonMotionFcn','','WindowButtonUpFcn','');
   udata = rmfield(udata,{'ax','imh','oldAxesUnits','moved','coords','startslice'});
   if isfield(udata,'starttime')
      udata = rmfield(udata,'starttime');
   end
   set(fig,'UserData',[]);    % Solve MATLAB bug!
   set(fig,'UserData',udata);
end


%
% Callback function for keyboard event
%
function KeyPressFcn(fig)
if strncmp(get(fig,'Tag'),'DIP_Image',9)
   udata = get(fig,'UserData');
   nD = length(udata.imsize);
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
         case {char(13)} % Enter: go to selected slice
            if (nD>=3) && isfield(udata,'nextslice') && ~isempty(udata.nextslice)
               newslice = str2double(udata.nextslice);
               if ~isnan(newslice)
                  imh = findobj(fig,'Type','image');
                  if nD>=4
                     newslice(2) = udata.curtime;
                  end
                  udata = update_slice(fig,imh,udata,newslice);
                  dipfig_titlebar(fig,udata);
                  if ~isempty(udata.linkdisplay)
                     udata.linkdisplay = update_linked(fig,udata.linkdisplay,newslice,udata.slicing,udata.zoom);
                  end
               end
               udata.nextslice = '';
               set(fig,'UserData',[]);    % Solve MATLAB bug!
               set(fig,'UserData',udata);
            end
         % ; for advance in time direction with 'number + ;
         case {';'} % Enter: go to selected slice
            if (nD>=4) && isfield(udata,'nextslice') && ~isempty(udata.nextslice)
               tmp = str2double(udata.nextslice);
               if ~isnan(tmp)
                  imh = findobj(fig,'Type','image');
                  newslice = [udata.curslice,tmp];
                  udata = update_slice(fig,imh,udata,newslice);
                  dipfig_titlebar(fig,udata);
                  if ~isempty(udata.linkdisplay)
                     udata.linkdisplay = update_linked(fig,udata.linkdisplay,newslice,udata.slicing,udata.zoom);
                  end
               end
               udata.nextslice = '';
               set(fig,'UserData',[]);    % Solve MATLAB bug!
               set(fig,'UserData',udata);
            end
         case {'p','P','n','N'} % Previous/next slice
            if nD>=3
               imh = findobj(fig,'Type','image');
               if upper(ch)=='P'
                  newslice = udata.curslice-1;
               else
                  newslice = udata.curslice+1;
               end
               if nD>=4
                  newslice(2) = udata.curtime;
               end
               udata = update_slice(fig,imh,udata,newslice);
               dipfig_titlebar(fig,udata);
               if ~isempty(udata.linkdisplay)
                  udata.linkdisplay = update_linked(fig,udata.linkdisplay,newslice,udata.slicing,udata.zoom);
               end
               udata.nextslice = '';
               set(fig,'UserData',[]);    % Solve MATLAB bug!
               set(fig,'UserData',udata);
            end
         case {'f','b','F','B'}
            if nD>=4
               imh = findobj(fig,'Type','image');
               newslice = udata.curslice;
               if upper(ch)=='B'
                  newslice(2) = udata.curtime-1;
               else
                  newslice(2) = udata.curtime+1;
               end
               %newslice
               udata = update_slice(fig,imh,udata,newslice);
               dipfig_titlebar(fig,udata);
               if ~isempty(udata.linkdisplay)
                  udata.linkdisplay = update_linked(fig,udata.linkdisplay,newslice,udata.slicing,udata.zoom);
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
function ResizeFcn(fig)
udata = get(fig,'UserData');
% 27-10-2006 MvG -- ResizeFcn gets called while the "construction" of the
% display data is still ongoing. This was originally not the case, because
% the figure's visibility was 'off' during this construction. Unfortunately,
% the visibility off-on cycle implies that the window comes to front. With
% 'BringToFrontOnDisplay' off, we cannot turn off the window's visibility
% and therefore this callback gets called. We can simply test whether udata
% is empty or not to ignore the callback during the construction phase.
%fprintf(1,'.\n');
if isempty(udata)
   return;
end
%fprintf(1,'ResizeFcn\n');
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
