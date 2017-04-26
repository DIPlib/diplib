%DIPMAPPING   Changes the mapping of an image in a figure window
%   DIPMAPPING(H,MAPPING) changes the mapping setting for figure window
%   H. MAPPING can be any string of the following:
%
%      'unit'                 'labels'          'xy'
%      'normal' or '8bit'     'periodic'        'xz'
%      '12bit'                'grey'            'yz'
%      '16bit'                'saturation'      'xt'
%      's8bit'                'zerobased'       'yt'
%      's12bit'                                 'zt'
%      's16bit'
%      'lin' or 'all'         'abs'
%      'percentile'           'real'            'global'
%      'log'                  'imag'            'nonglobal'
%      'base'                 'phase'
%      'angle'
%      'orientation'
%
%   The strings in the first column change the grey-value mapping. See
%   the help for DIPSHOW for more information on these. The strings in
%   the second column change the colormap. Incidentally, 'labels'
%   implies 'normal'. The strings in the third column set the complex
%   to real mapping. The ones in the third comumn select the orientation
%   of the slices in a 3D display, and the ones in the fourth column set
%   global stretching on or off in 3D displays. You can combine one
%   string from each of these four columns in a single command, the
%   order is irrelevant.
%
%   Additionally, DIPMAPPING(...,'SLICE',N) sets a 3D display to slice
%   number N. These two values must be consecutive, but they can be
%   mixed in with the other strings in any order.
%   DIPMAPPING(...,'COLORMAP',CM) sets the colormap to CM. These two
%   values must be consecutive, but they can be mixed in with the other
%   strings in any order.
%
%   You can also use a two-value vector that sets the range, as in
%   DIPSHOW: DIPMAPPING(H,[LOW,HIGH]).
%
%   H, the figure window handle, can be left out. It will default to
%   the current figure.
%
%   See also DIPSHOW.

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

% Undocumented:
%   DIPMAPPING('manual') brings up a dialog box that allows the user to
%   select an upper and lower bound for the range. It is used as a callback
%   to the 'Manual...' menu item under 'Mappings'.
%   DIPMAPPING('custom') brings up a dialog box that allows the user to
%   select a colormap. It is used as a callback to the 'Custom...' menu
%   item under 'Mappings'.

function dipmapping(varargin)

% Defaults...
fig = [];
currange = [];       hasrange = 0;
colmap = [];         hascolmap = 0;
complexmapping = []; hastoreal = 0;
slicing = [];        hasslicing = 0;
globalstretch = 0;   hasglobal = 0;
manual = 0;
custom = 0;
newslice = [];

% Parse input
N = nargin;
if N==0
   error('At least one argument required.')
end
ii = 1;
while ii<=N
   arg = lower(varargin{ii});
   if ischar(arg)
      switch arg
         case {'normal','unit','lin','all','percentile','angle','orientation','base','log','8bit','12bit','16bit','u8bit','u12bit','u16bit','s8bit','s12bit','s16bit'}
            if hasrange, error('Too many arguments.'); end
            currange = arg;
            hasrange = 1;
         case {'grey','gray','periodic','saturation','zerobased'}
            if hascolmap, error('Too many arguments.'); end
            colmap = arg;
            hascolmap = 1;
         case {'labels'}
            if hascolmap || hasrange, error('Too many arguments.'); end
            colmap = arg;
            hascolmap = 1;
            currange = 'normal';
            hasrange = 1;
         case {'abs','real','imag','phase'}
            if hastoreal, error('Too many arguments.'); end
            complexmapping = arg;
            hastoreal = 1;
         case {'xy','xz','yz','xt','yt','zt'}
            if hasslicing, error('Too many arguments.'); end
            slicing = arg;
            hasslicing = 1;
         case {'global','nonglobal'}
            if hasglobal, error('Too many arguments.'); end
            if strcmp(arg,'global')
               globalstretch = 1;
            end
            hasglobal = 1;
         case 'manual'
            manual = 1;
         case 'custom'
            custom = 1;
         case 'slice'
            if ii>=N, error('Too few arguments.'); end
            if ~isempty(newslice), error('Too many arguments.'); end
            ii = ii+1;
            newslice = varargin{ii};
            if ~isnumeric(newslice) || length(newslice)~=1
               error('Slice number expected.')
            end
         case 'colormap'
            if ii>=N, error('Too few arguments.'); end
            if ~isempty(colmap), error('Too many arguments.'); end
            ii = ii+1;
            colmap = varargin{ii};
            hascolmap = 1;
            if ~isnumeric(colmap) || size(colmap,2)~=3
               error('Colormap expected.')
            end
         otherwise
            if ii==1
               try
                  fig = getfigh(arg);
               catch
                  error('Argument must be a valid figure handle.')
               end
               if ~strncmp(get(fig,'tag'),'DIP_Image',9)
                  error('Figure is not created by DIPSHOW.')
               end
            else
               error('Illegal argument.')
            end
      end
   else
      if ( isnumeric(arg) || ishandle(arg) ) && length(arg)==1 && ii==1 && N>1
         % figure handle parameter?
         try
            fig = getfigh(arg);
         catch
            error('Argument must be a valid figure handle.')
         end
      else
         if ~isnumeric(arg) || ~(isempty(arg) || numel(arg)==2)
            error('Arguments should be strings or two-element vectors.')
         end
         if hasrange
            error('Too many arguments.')
         end
         currange = arg;
         hasrange = 1;
      end
   end
   ii = ii+1;
end
if isempty(fig)
   fig = get(0,'CurrentFigure');
   if isempty(fig)
      error('No figure window open to do operation on.')
   end
end
if ~strncmp(get(fig,'Tag'),'DIP_Image',9)
   error('DIPMAPPING only works on images displayed using DIPSHOW.')
end

if manual

%
% GUI functionality manual stretch
%

   if hasrange || hascolmap || hastoreal || hasslicing || ~isempty(newslice) || custom
      error('Too many arguments.')
   end
   % Get slider range
   udata = get(fig,'UserData');
   computedrange = false;
   currange = [0,255];
   low = 0;
   high = 255;
   if isfield(udata,'handle')
      limits = imagedisplay(udata.handle,'limits_or_nan');
      if ~any(isnan(limits))
         currange = limits;
         if currange(1)==currange(2)
            currange = currange+[-1,1];
         end
         computedrange = true;
      end
      range = imagedisplay(udata.handle,'range');
      low = range(1);
      high = range(2);
      currange(1) = min(currange(1),low);
      currange(2) = max(currange(2),high);
   end
   % Create dialog box
   h = figure('Visible','off',...
              'Color',get(0,'defaultuicontrolbackgroundcolor'),...
              'KeyPressFcn',@rangeKeyPressFcn,...
              'MenuBar','none',...
              'Name','DIPimage',...
              'NumberTitle','off',...
              'Resize','off',...
              'Units','pixels'...
              );
   % Create controls
   okbutton =   uicontrol(h,'Style','pushbutton','String','OK',...
                          'Callback',@(~,~)set(gcbf,'UserData','ok'),'units','pixels');
   textlow =    uicontrol(h,'Style','text','units','pixels','HorizontalAlignment','left',...
                          'String','Lower bound');
   sliderlow =  uicontrol(h,'Style','slider','units','pixels','SliderStep',[0.01,0.1],...
                          'min',currange(1),'max',currange(2),'Value',min(max(low,currange(1)),currange(2)));
   editlow =    uicontrol(h,'Style','edit','units','pixels','HorizontalAlignment','left',...
                          'BackgroundColor',[1,1,1],'String',num2str(low));
   texthigh =   uicontrol(h,'Style','text','units','pixels','HorizontalAlignment','left',...
                          'String','Upper bound');
   sliderhigh = uicontrol(h,'Style','slider','units','pixels','SliderStep',[0.01,0.1],...
                          'min',currange(1),'max',currange(2),'Value',min(max(high,currange(1)),currange(2)));
   edithigh =   uicontrol(h,'Style','edit','units','pixels','HorizontalAlignment','left',...
                          'BackgroundColor',[1,1,1],'String',num2str(high));
   labellow =   uicontrol(h,'Style','text','units','pixels','HorizontalAlignment','left',...
                          'String',num2str(currange(1)));
   labelhigh =  uicontrol(h,'Style','text','units','pixels','HorizontalAlignment','right',...
                          'String',num2str(currange(2)));
   labelline =  uicontrol(h,'Style','frame','units','pixels','BackgroundColor',[0,0,0]);
   if ~computedrange
      computebutton = uicontrol(h,'Style','pushbutton','String','Compute Range',...
                                'Callback',@ComputeRange,'units','pixels');
   end
   udata = [];
   udata.editlow = editlow;
   udata.edithigh = edithigh;
   udata.fig = fig;
   udata.labellow = labellow;
   udata.labelhigh = labelhigh;
   udata.labelline = labelline;
   set(h,'UserData',udata);
   % Callbacks
   set(editlow,'UserData',sliderlow,'Callback',@rangeEditCallback);
   set(edithigh,'UserData',sliderhigh,'Callback',@rangeEditCallback);
   set(sliderlow,'UserData',editlow,'Callback',@rangeSliderCallback);
   set(sliderhigh,'UserData',edithigh,'Callback',@rangeSliderCallback);
   % Set sizes and positions       % (Copied from DIPimage)
   sizes.leftmargin = 20;          % Space between left edge and controls
   sizes.internalmargin = 10;      % Horizontal space between controls
   sizes.rightmargin = 20;         % Space between right edge and controls
   sizes.topmargin = 10;           % Space between top edge and controls
   sizes.bottommargin = 10;        % Space between bottom edge and controls
   sizes.vspacing = 6;             % Space between rows
   sizes.vslack = 4;               % Extra space allotted to controls (around text)
   sizes.hslack = 10;              % Extra space allotted to controls (around text)
   units = get(0,'units');
   set(0,'units','pixels');
   screensz = get(0,'screensize');
   set(0,'units',units);
   textpos = max(get(textlow,'Extent'),get(texthigh,'Extent'));
   set(okbutton,'String','Select All'); % Standard button size in DIPimage...
   buttonpos = get(okbutton,'Extent')+[0,0,sizes.hslack,sizes.vslack];
   set(okbutton,'String','OK');
   sliderpos = get(sliderlow,'Position');
   sliderpos(3) = 256; % Is this OK?
   labelpos = get(labellow,'Extent');
   editpos = buttonpos;
   figpos = [0,0,sizes.leftmargin+sliderpos(3)+sizes.internalmargin+editpos(3)+sizes.rightmargin,...
             sizes.bottommargin+buttonpos(4)+sizes.vspacing+2*textpos(4)+2*sliderpos(4)+labelpos(4)+4*sizes.vslack+sizes.topmargin];
   figpos(1:2) = round(screensz(3:4)-figpos(3:4))/2;
   set(h,'position',figpos);
   buttonpos(1:2) = [figpos(3)-sizes.rightmargin-buttonpos(3),sizes.bottommargin];
   set(okbutton,'position',buttonpos);
   textpos(1:2) = [sizes.leftmargin,sizes.bottommargin+buttonpos(4)+sizes.vspacing];
   set(texthigh,'position',textpos);
   sliderpos(1:2) = [sizes.leftmargin,textpos(2)+textpos(4)+sizes.vslack];
   set(sliderhigh,'position',sliderpos);
   editpos(1:2) = [figpos(3)-sizes.rightmargin-editpos(3),sliderpos(2)+sliderpos(4)/2-editpos(4)/2];
   set(edithigh,'position',editpos);
   labelpos(1:2) = [sizes.leftmargin,sliderpos(2)+sliderpos(4)+sizes.vslack];
   set(labellow,'position',labelpos);
   tmp = get(labelhigh,'Extent');
   linepos = [labelpos(1)+labelpos(3),labelpos(2)+labelpos(4)/2,sliderpos(3)-labelpos(3)-tmp(3),1];
   set(labelline,'position',linepos);
   labelpos(3) = tmp(3);
   labelpos(1) = sizes.leftmargin+sliderpos(3)-labelpos(3);
   set(labelhigh,'position',labelpos);
   sliderpos(2) = labelpos(2)+labelpos(4)+sizes.vslack;
   set(sliderlow,'position',sliderpos);
   editpos(2) = sliderpos(2)+sliderpos(4)/2-editpos(4)/2;
   set(editlow,'position',editpos);
   textpos(2) = sliderpos(2)+sliderpos(4)+sizes.vslack;
   set(textlow,'position',textpos);
   if ~computedrange
      bw = get(computebutton,'extent');
      buttonpos(3) = bw(3)+sizes.hslack;
      buttonpos(1) = buttonpos(1) - sizes.internalmargin - buttonpos(3);
      set(computebutton,'position',buttonpos);
   end
   set(h,'Visible','on')
   % Run
   waitfor(h,'UserData')
   if ishandle(h)
      delete(h)
   end

elseif custom

%
% GUI functionality custom colormap
%

   if hasrange || hascolmap || hastoreal || hasslicing || ~isempty(newslice)
      error('Too many arguments.')
   end
   tag = get(fig,'tag');
   if length(tag)==18 && tag(14:18)=='Color'
      error('Cannot change color map for color image.')
   end
   % Create dialog box
   h = figure('Visible','off',...
              'Color',get(0,'defaultuicontrolbackgroundcolor'),...
              'KeyPressFcn',@colmapKeyPressFcn,...
              'MenuBar','none',...
              'Name','DIPimage',...
              'NumberTitle','off',...
              'Resize','off',...
              'Units','pixels'...
              );
   % Create controls
   okbutton     = uicontrol(h,'Style','pushbutton','String','OK',...
                        'Callback',@(~,~)set(gcbf,'UserData','ok'),'units','pixels');
   cancelbutton = uicontrol(h,'Style','pushbutton','String','Cancel',...
                        'Callback',@(~,~)set(gcbf,'UserData','cancel'),'units','pixels');
   textbox      =  uicontrol(h,'Style','text','units','pixels','HorizontalAlignment','left',...
                        'String','Command to generate colormap (ex: hot, copper, winter, etc.)');
   editbox      =  uicontrol(h,'Style','edit','units','pixels','HorizontalAlignment','left',...
                        'BackgroundColor',[1,1,1],'String','');
   udata = [];
   set(h,'UserData',udata);
   % Callbacks
   set(editbox,'Callback',@colmapKeyPressFcn);
   % Set sizes and positions       % (Copied from DIPimage)
   sizes.leftmargin = 20;          % Space between left edge and controls
   sizes.internalmargin = 10;      % Horizontal space between controls
   sizes.rightmargin = 20;         % Space between right edge and controls
   sizes.topmargin = 10;           % Space between top edge and controls
   sizes.bottommargin = 10;        % Space between bottom edge and controls
   sizes.vspacing = 6;             % Space between rows
   sizes.vslack = 4;               % Extra space allotted to controls (around text)
   sizes.hslack = 10;              % Extra space allotted to controls (around text)
   units = get(0,'units');
   set(0,'units','pixels');
   screensz = get(0,'screensize');
   set(0,'units',units);
   textpos = get(textbox,'Extent');
   set(okbutton,'String','Select All'); % Standard button size in DIPimage...
   buttonpos = get(okbutton,'Extent')+[0,0,sizes.hslack,sizes.vslack];
   set(okbutton,'String','OK');
   editpos = buttonpos;
   editpos(3) = textpos(3);
   figpos = [0,0,sizes.leftmargin+textpos(3)+sizes.rightmargin,...
             sizes.bottommargin+2*buttonpos(4)+2*sizes.vspacing+textpos(4)+sizes.topmargin];
   figpos(1:2) = round(screensz(3:4)-figpos(3:4))/2;
   set(h,'position',figpos);
   buttonpos(1:2) = [figpos(3)-sizes.rightmargin-buttonpos(3),sizes.bottommargin];
   set(okbutton,'position',buttonpos);
   buttonpos(1) = buttonpos(1)-buttonpos(3)-sizes.vspacing;
   set(cancelbutton,'position',buttonpos);
   editpos(1:2) = [sizes.leftmargin,sizes.bottommargin+buttonpos(4)+sizes.vspacing];
   set(editbox,'position',editpos);
   textpos(1:2) = [sizes.leftmargin,editpos(2)+editpos(4)+sizes.vspacing];
   set(textbox,'position',textpos);
   set(h,'Visible','on')
   % Run
   waitfor(h,'UserData')
   if ishandle(h)
      if strcmp(get(h,'UserData'),'ok')
         colmap = get(editbox,'String');
         delete(h)
         if ~isempty(colmap)
            try
               colmap = evalin('base',colmap);
            catch
               error('Evaluation of user string failed.')
            end
            dipshow(fig,'ch_colormap',colmap);
         end
      else
         delete(h)
      end
   end

else

%
% Normal functionality
%

   params = cell(0);
   ii = 1;
   if hasrange
      params{ii} = 'ch_mappingmode';
      params{ii+1} = currange;
      ii = ii+2;
   end
   if hascolmap
      params{ii} = 'ch_colormap';
      params{ii+1} = colmap;
      ii = ii+2;
   end
   if hastoreal
      params{ii} = 'ch_complexmapping';
      params{ii+1} = complexmapping;
      ii = ii+2;
   end
   if hasslicing
      params{ii} = 'ch_slicing';
      params{ii+1} = slicing;
      ii = ii+2;
   end
   if hasglobal
      params{ii} = 'ch_globalstretch';
      params{ii+1} = globalstretch;
      ii = ii+2;
   end
   if ~isempty(newslice)
      params{ii} = 'ch_slice';
      params{ii+1} = newslice;
      %ii = ii+2;
   end
   dipshow(fig,params{:});

end

%
% Manual stretch callbacks
%

function rangeEditCallback(h,~)
slider = get(h,'UserData');
val = str2double(get(h,'String'));
if isfinite(val)
   val = min(max(val,get(slider,'Min')),get(slider,'Max'));
   set(slider,'Value',val);
   updatedisplay(gcbf);
end

function rangeSliderCallback(h,~)
edt = get(h,'UserData');
val = get(h,'Value');
set(edt,'String',num2str(val));
updatedisplay(gcbf);

function updatedisplay(fig)
udata = get(fig,'UserData');
low = str2double(get(udata.editlow,'String'));
high = str2double(get(udata.edithigh,'String'));
dipshow(udata.fig,'ch_mappingmode',[low,high]);

function rangeKeyPressFcn(fig,~)
ch = double(get(fig,'CurrentCharacter'));
if ~isempty(ch)
   switch ch
   case 13
      set(fig,'UserData','ok');
   case 27
      set(fig,'UserData','cancel');
   end
end

function ComputeRange(h,~)
udata = get(gcbf,'UserData');
imgudata = get(udata.fig,'UserData');
if isfield(imgudata,'handle')
   currange = imagedisplay(imgudata.handle,'limits');
   if currange(1)==currange(2)
      currange = currange+[-1,1];
   end
   % read current settings
   editlow = udata.editlow;
   edithigh = udata.edithigh;
   low = str2double(get(editlow,'String'));
   high = str2double(get(edithigh,'String'));
   if isfinite(low)
      low = min(max(low,currange(1)),currange(2));
   else
      low = currange(1);
   end
   if isfinite(high)
      high = min(max(high,currange(1)),currange(2));
   else
      high = currange(2);
   end
   % set sliders & edit boxes to new range & values
   sliderlow = get(editlow,'UserData');
   set(sliderlow,'min',currange(1),'max',currange(2),'Value',low);
   set(editlow,'String',num2str(low));
   sliderhigh = get(edithigh,'UserData');
   set(sliderhigh,'min',currange(1),'max',currange(2),'Value',high);
   set(edithigh,'String',num2str(high));
   % modify labels
   labellow = udata.labellow;
   labelhigh = udata.labelhigh;
   set(labellow,'String',num2str(currange(1)));
   set(labelhigh,'String',num2str(currange(2)));
   labwidth = get(labellow,'extent');
   labwidth = labwidth(3);
   labpos = get(labellow,'position');
   labpos(3) = labwidth;
   set(labellow,'position',labpos);
   linepos = [labpos(1)+labpos(3),labpos(2)+labpos(4)/2,0,1];
   labwidth = get(labelhigh,'extent');
   labwidth = labwidth(3);
   labpos = get(labelhigh,'position');
   labpos(1) = labpos(1)+labpos(3)-labwidth;
   labpos(3) = labwidth;
   set(labelhigh,'position',labpos);
   linepos(3) = labpos(1)-linepos(1);
   set(udata.labelline,'position',linepos);
   % get rid of compute button
   delete(h);
   % update image
   dipshow(udata.fig,'ch_mappingmode',[low,high]);
end

%
% Colormap selection callbacks
%

function colmapKeyPressFcn(~,~)
fig = gcbf;
ch = double(get(fig,'CurrentCharacter'));
if ~isempty(ch)
   switch ch
   case 13
      set(fig,'UserData','ok');
   case 27
      set(fig,'UserData','cancel');
   end
end
