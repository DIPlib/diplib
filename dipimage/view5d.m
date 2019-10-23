%VIEW5D   Start the Java 5D image viewer by Rainer Heintzmann
%
% SYNOPSIS:
%  view5d(in,mode,myViewer,element)
%  view5d(in,ts,mode,myViewer,element)
%
% PARAMETERS:
%  in:       can either be an image or a figure handle
%  ts:       is the input a time series? (true/false)
%  mode:     'direct', 'newElement', 'replaceElement', 'newTime' or 'extern'
%  myViewer: (only for 'newElement' or 'replaceElement') a previously
%            created view5d instance.
%  element:  (only for 'replaceElement'), the element for which to replace
%            the data
%
% RETURNS:
%  a Java instance of the viewer, or an empty array in 'extern' mode.
%
% DEFAULTS:
%  ts:   false
%  mode: 'direct'
%  element: 0
%
% See also DIPSHOW.
%
% NOTES:
%  For documentation and tutorials on the use of the viewer, see
%     http://www.nanoimaging.de/View5D/
%
%  The 'direct' option starts the applet directly. 'newElement' and
%  'replaceElement' add or replace an element (channel) in an existing
%  applet. 'newTime' adds a time step to an existing applet.
%
%  The 'extern' option displays the image in your system browser. On Linux,
%  you can configure which browser MATLAB uses through the Preferences
%  panel, see DOC WEB. Make sure your web browser has Java enabled.
%
%  If there is no Java support in MATLAB, the external mode is always used.
%
%  For larger images you may get java.lang.OutOfMemoryError
%  Increase the memory for the jvm by placing the file java.opts in your
%  MATLAB directory with -Xmx#bytes (Java 1.3) or maxHeapSize=#bytes (older)
%  Set #bytes to something large (600000000).
%
% INSTALLING VIEW5D:
%  The View5D Java applet is not included in the DIPimage distribution. If you
%  want to use this functionality, please download it separately.
%
%  You will find the latest version here:
%     https://github.com/RainerHeintzmann/View5D
%  Copy or move the Java5D.jar file to the 'private' directory under the
%  DIPimage directory (the directory that contains this file). The following
%  MATLAB command gives you the directory where the JAR file should reside:
%     fullfile(fileparts(which('view5d')),'private')
%
%  Note that View5D is licensed under GLPv2

% (c)2019, Rainer Heintzmann, Cris Luengo.
% (c)1999-2014, Delft University of Technology (Bernd Rieger).
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

function myViewer = view5d(in,varargin)

% Parse input
mode = 'direct';
direct = true;
ts = false;
myViewer = [];
ElementNum = 0;

N = numel(varargin);
ii = 1;
if ii <= N && ~ischar(varargin{ii})
   ts = varargin{ii};
   ii = ii + 1;
end
if ii <= N
   mode = varargin{ii};
   if ~ischar(mode)
      error('MODE string expected.')
   end
   needViewer = false;
   switch mode
      case 'direct'
      case {'newElement','newTime','replaceElement'}
         needViewer = true;
      case 'extern'
         direct = false;
      otherwise
         error('MODE string should be ''direct'', ''newElement'', ''newTime'', ''replaceElement'' or ''extern''.')
   end
   ii = ii + 1;
   if needViewer
      if ii <= N
         myViewer = varargin{ii};
         ii = ii + 1;
      end
      if isempty(myViewer) || ~isa(myViewer,'view5d.View5D')
         error(['The option ''',mode,''' requires input argument ''myViewer'' to be a view5d.View5D instance'])
      end
      if ii <= N && strcmp(mode,'replaceElement')
         ElementNum = varargin{ii};
         ii = ii + 1;
      end
   end
end
if ii <= N
   error('Too many input arguments')
end

% Load Java tool
if direct
   if ~isempty(javachk('jvm'))
      disp('Using the external viewer, MATLAB started without Java support.');
      direct = false;
   end
end
if direct
   try
      % Add the path if not known
      jp = javaclasspath('-all');
      jarfile = jarfilename;
      if ~any(strcmp(jp,jarfile))
         javaaddpath(jarfile);
      end
      % Force the loading of the JAR file
      import view5d.*
   catch
      help view5d.m
      error(['Please install the Java5D.jar file as explained in the ''INSTALLING VIEW5D''',10,'section above.'])
   end
end

% Prepare input image
imname = inputname(1);
if ishandle(in)
   in = dipgetimage(in);
elseif isa(in,'cuda')
   in = dip_image_force(in);
else
   in = dip_image(in);
end
sz = imsize(in);
if length(sz)>5 || length(sz)<2
   error('Only available for 2, 3, 4 and 5D images.');
end
if any(sz(1:2)==1)
   error('First two image dimensions must be larger than 1.');
end
if numtensorel(in) > 1
   % The array elements need to go along the 4th dimension, and not become the time axis later on.
   if length(sz)>4
      error('Only available for 2, 3 and 4D tensor images.');
   end
   in = tensortospatial(in,4);
   elements = 1;
else
   elements = 0;
end
in = expanddim(in,5); % make sure the input has 5 dimensions
sz = imsize(in);
if strcmp(datatype(in),'dcomplex')
   in = dip_image(in,'scomplex'); % dcomplex not supported
end
if ts
   % The user asks for a time series -- move the last data dimension to the 5th dimension
   t = find(sz>1); t = t(end);
   if t<3 || t==5
      t = [];
   elseif t==4 && elements
      sz(t) = 1;
      t = find(sz>1); t = t(end);
      if t<3
         t = [];
      end
   end
   if ~isempty(t)
      order = 1:5;
      order(t) = 5;
      order(5) = t;
      in = permute(in,order);
      sz = imsize(in);
   end
end

% Handle iterative modes
if strcmp(mode,'newElement')
   if sz(4) > 1 || sz(5) > 1
      for t = 1:sz(5)
         for e = 1:sz(4)
            % fprintf('Adding Element: %d, Time: %d\n',e,t);
            view5d(in(:,:,:,e-1,t-1),ts,'newElement',myViewer);
         end
      end
      return
   end
elseif strcmp(mode,'newTime')
   if sz(5) > 1
      for t = 1:sz(5)
         for e = 1:sz(4)
            % fprintf('Adding ElemenTime: %d, Time: %d\n',e,t);
            view5d(in(:,:,:,e-1,t-1),ts,'newTime',myViewer);
         end
      end
      return
   end
end

% Start the applet
if direct
   % Make a one dimensional flat input array
   in = permute(in,[2,1,3:numel(sz)]);
   in5d = in.Array;
   in5d = in5d(:);
   % Real or complex data is handled differently
   if isreal(in)
      if isa(in5d,'uint16')
         in5d = char(in5d);  % only this way java understands this type...
      end
      if strcmp(mode,'direct')
         myViewer = View5D.Start5DViewer(in5d,sz(1),sz(2),sz(3),sz(4),sz(5));
         if ~isempty(imname)
            myViewer.NameWindow(imname);
         end
         ElementNum = 0:myViewer.getNumElements()-1;
      elseif strcmp(mode,'newElement')
         myViewer.AddElement(in5d,sz(1),sz(2),sz(3),sz(4),sz(5));
         ElementNum = myViewer.getNumElements()-1;
      elseif strcmp(mode,'newTime')
         myViewer.setTimesLinked(0);
         myViewer.AddTime(in5d,sz(1),sz(2),sz(3),sz(4),sz(5));
         ElementNum = myViewer.getNumElements()-1;
      elseif strcmp(mode,'replaceElement')
         if (ElementNum >= myViewer.getNumElements())
            myViewer.AddElement(in5d,sz(1),sz(2),sz(3),sz(4),sz(5));
            ElementNum = myViewer.getNumElements()-1;
         else
            myViewer.ReplaceData(ElementNum,0,in5d);
         end
      end
   else
      in5d_angle = dip_array(angle(in));
      in5d_angle = in5d_angle(:);
      if strcmp(mode,'direct')
         myViewer = View5D.Start5DViewerC(in5d,sz(1),sz(2),sz(3),sz(4),sz(5));
         myViewer.AddElement(in5d_angle,sz(1),sz(2),sz(3),sz(4),sz(5));
         if ~isempty(imname)
            myViewer.NameWindow(imname);
         end
         ElementNum = 0:myViewer.getNumElements()-1;
         setComplexDisplayMode(myViewer)
         myViewer.UpdatePanels();
      elseif strcmp(mode,'newElement')
         myViewer.AddElementC(in5d,sz(1),sz(2),sz(3),sz(4),sz(5));
         myViewer.AddElement(in5d_angle,sz(1),sz(2),sz(3),sz(4),sz(5));
         ElementNum = myViewer.getNumElements()-1;
         setComplexDisplayMode(myViewer)
      elseif strcmp(mode,'newTime')
         myViewer.setTimesLinked(0);
         myViewer.AddTime(in5d,sz(1),sz(2),sz(3),sz(4),sz(5));
         myViewer.AddElement(in5d_angle,sz(1),sz(2),sz(3),sz(4),sz(5));
         ElementNum = myViewer.getNumElements()-1;
         setComplexDisplayMode(myViewer)
      elseif strcmp(mode,'replaceElement')
         if (ElementNum >= myViewer.getNumElements())
            myViewer.AddElementC(in5d,sz(1),sz(2),sz(3),sz(4),sz(5));
            ElementNum = myViewer.getNumElements()-1;
         else
            myViewer.ReplaceDataC(ElementNum,0,in5d);
         end
         % Repeat for now phase
         ElementNum = ElementNum+1;
         if (ElementNum >= myViewer.getNumElements())
            myViewer.AddElement(in5d_angle,sz(1),sz(2),sz(3),sz(4),sz(5));
            ElementNum = myViewer.getNumElements()-1;
         else
            myViewer.ReplaceData(ElementNum,0,in5d_angle);
         end
      end
   end
   myViewer.UpdatePanels();
   if ~isempty(in.pixelsize)
      AxisNames = {'X','Y','Z','E','T'};
      AxisUnits = [in.pixelunits(1:3),'a.u.','a.u.'];
      for n = 1:numel(ElementNum)
         myViewer.SetAxisScalesAndUnits(ElementNum(n),0,1.0,...
            in.pixelsize(1),in.pixelsize(2),in.pixelsize(1),...
            1.0,1.0,0.0,0.0,0.0,0.0,0.0,0.0,...
            'intensity',AxisNames,'a.u.',AxisUnits);
      end
   end
else
   view5d_image_extern(in);
   myViewer = [];
end

%-----------------------------------------------------------------------
function jarfile = jarfilename
try
   jarfile = fullfile(fileparts(mfilename('fullpath')),'private','View5D.jar');
catch
   jarfile = fullfile(fileparts(which('dipsetpref')),'private','View5D.jar');
end

%-----------------------------------------------------------------------
function setComplexDisplayMode(out)
out.ProcessKeyMainWindow('O'); % logarithmic display mode
out.ProcessKeyMainWindow('e'); % advance an element
for q = 1:12
   out.ProcessKeyMainWindow('c'); % cyclic colormap
end
out.ProcessKeyMainWindow('t'); % min max adjustment
out.ProcessKeyMainWindow('v'); %
out.ProcessKeyMainWindow('V'); %
out.ProcessKeyMainWindow('E'); % Back to previous element
out.ProcessKeyMainWindow('C'); % multicolor on

%-----------------------------------------------------------------------
function view5d_image_extern(in)

jarfile = jarfilename;
base = tempdir; % return OS set tempdir
if ~tempdir
   error('No temp directory given by OS.');
end
fn = [base,'dipimage_view5d'];

mdt = {'uint8',    1, 8,  'Byte'
   'uint16',   2, 16, 'Char'
   'uint32',   4, 32, 'Long'
   'sint8',    1, 8,  'Byte'
   'sint16',   2, 16, 'Short'
   'sint32',   4, 32, 'Long'
   'sfloat',  -1, 32, 'Float'
   'dfloat',  -1, 64, 'Double'
   'scomplex',-1, 32, 'Complex'
   'bin',      1, 8,  'Byte'
   };
ind = find(strcmp(datatype(in),mdt(:,1)));
bytes = mdt{ind,2};
bits  = mdt{ind,3};
dtype = mdt{ind,4};

sz = imsize(in);

% write raw data
writeics(in,fn,{},0,'v1');
delete([fn,'.ics']);

% write html file
content = ['<html><head><title>5D Viewer</title></head><body>' 10 ...
           '<!--Automatically generated by DIPimage.-->' 10 ...
           '<h1>5D Viewer</h1><p>Image Data displayed by View5D, a Java-Applet by Rainer Heintzmann</p>' 10 ...
           '<hr><applet archive=',jarfile,' code=View5D.class width=600 height=700 alt="Please enable Java.">' 10 ...
           '<param name=file value=',fn,'.ids>' 10 ...
           '<param name=sizex value=' num2str(sz(1)) '>' 10 ...
           '<param name=sizey value=' num2str(sz(2)) '>' 10 ...
           '<param name=sizez value=' num2str(sz(3)) '>' 10 ...
           '<param name=times value=' num2str(sz(5)) '>' 10 ...
           '<param name=elements value=' num2str(sz(4)) '>' 10 ...
           '<param name=bytes value=' num2str(bytes) '>' 10 ...
           '<param name=bits value=' num2str(bits) '>' 10 ...
           '<param name=dtype value=''' dtype '''>' 10 ...
           '<param name=unitsx value=''pix''>' 10 ...
           '<param name=scalex value=''1''>' 10 ...
           '<param name=unitsy value=''pix''>' 10 ...
           '<param name=scaley value=''1''>' 10 ...
           '<param name=unitsz value=''pix''>' 10 ...
           '<param name=scalez value=''1''>' 10 ...
           '<param name=unitse value=''color''>' 10 ...
           '<param name=scalee value=''1''>' 10 ...
           '<param name=unitst value=''time''>' 10 ...
           '<param name=scalet value=''1''>' 10 ...
           '<param name=unitsv1 value=''int.''>' 10 ...
           '<param name=unitsv2 value=''int.''>' 10 ...
           '<param name=unitsv3 value=''int.''>' 10 ...
           '<param name=unitsv4 value=''int.''>' 10 ...
           '<p>Your browser does not support Java applets.</p>' 10 ...
           '</applet><hr></body></html>' 10];
fid = fopen([fn '.html'],'w');
if fid <0
   error(['Could not create file' fn '.html']);
end
fprintf(fid,'%s',content);
fclose(fid);

% Start the applet
web(['file://' fn '.html'],'-browser');
