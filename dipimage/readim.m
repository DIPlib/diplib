%READIM   Read image from file
%
% SYNOPSIS:
%  [image,metadata] = readim(filename,format)
%
% PARAMETERS:
%  filename: string with name of file, optionally with path and extension.
%  format:   string with any of: 'ICS', 'TIFF', a format string recognized by
%            IMREAD, or 'bioformats'.
%
% DEFAULTS:
%  filename = 'erika.ics'
%  format = ''
%
% NOTES:
%  If FORMAT is '', then the format will be guessed from the file name extension
%  or the file contents. If READICS and READTIFF cannot read the file, this
%  function attempts to read it using Bio-Formats. If that also fails, IMREAD
%  is tried.
%
%  If FORMAT is specified, then either READICS, READTIFF or IMREAD are called.
%  See IMREAD for a list of formats it recognizes.
%  If FORMAT is 'bioformats', then only the Bio-Formats reader is used.
%
%  If the file name as given is not found (note that READICS and READTIFF will
%  also look for the file with an extension appended if not given), and no path
%  was specified, then process above is repeated after prepending each of the
%  directories listed in the 'ImageFilePath' property (see DIPSETPREF) to the
%  file name. That is, if no directory is specified, first the current directory
%  and then the default image directories are searched for the file.
%
%  Format 'TIF' is an alias for 'TIFF'. Likewise, 'ICS' is an alias for 'ICSv2'.
%
%  To read multi-page TIFF files use READTIFF, or specify the 'bioformats'
%  option. See also READTIMESERIES.
%
%  The functions READICS, READTIFF and IMREAD offer more options, consider
%  calling them directly.
%
% INSTALLING BIO-FORMATS:
%  The complete Bio-Formats is a large binary, and so we do not usually include
%  it in the DIPimage distribution. If you want to use this functionality,
%  please download it separately.
%
%  You will find the latest version here:
%     http://www.openmicroscopy.org/bio-formats/downloads/
%  Select to download the "Bio-Formats Package", which will copy a Java JAR file
%  to your computer. Copy or move this file to the 'private' directory under the
%  DIPimage directory (the directory that contains this file). The following
%  MATLAB command gives you the directory where the JAR file should reside:
%     fileparts(which('bfGetReader','in','readim'))
%
%  Note that Bio-Formats is licensed under GLP-2.0
%
% SEE ALSO:
%  writeim, readics, readtiff, imread, readtimeseries

% (c)2017-2018, Cris Luengo.
% Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
% Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
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

function [image,file_info] = readim(filename,format)

if nargin < 2
   format = '';
end
if nargin < 1
   filename = 'erika.ics';
end

% If we don't have a format, look for the file name extension
ext = '';
if isempty(format)
   [~,~,ext] = fileparts(filename);
   if ~isempty(ext) && ext(1)=='.'
      ext = ext(2:end);
   end
   switch upper(ext)
      case 'ICS'
         format = 'ICS';
      case {'TIF','TIFF'}
         format = 'TIFF';
   end
end

% Format aliases. Don't check for these if we found format trough the file name extension
if isempty(ext)
   switch upper(format)
      case {'ICSV2','ICS2','ICSV1','ICS1'}
         format = 'ICS';
      case 'TIF'
         format = 'TIFF';
   end
end

try
   [image,file_info] = readim_core(filename,format);
   return
end
if isempty(fileparts(filename)) % The file name has no path.
   p = convertpath(dipgetpref('imagefilepath'));
   for ii=1:length(p)
      try
         [image,file_info] = readim_core(fullfile(p{ii},filename),format);
         return
      end
   end
end
error('Could not open the file for reading')

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Core: read the given file

function [image,file_info] = readim_core(filename,format)

switch upper(format)
  case 'ICS'
     [image,file_info] = dip_fileio('readics',filename);
  case 'TIFF'
     [image,file_info] = dip_fileio('readtiff',filename);
  case 'BIOFORMATS'
     [image,file_info] = bfread(filename);
  case ''
     % If no format is given, try each of the readers in turn
     try
        [image,file_info] = dip_fileio('readics',filename);
        return
     end
     try
        [image,file_info] = dip_fileio('readtiff',filename);
        return
     end
     try
        [image,file_info] = bfread(filename);
        return
     end
     try
        [image,file_info] = mlread(filename);
        return
     end
     error('Could not open the file with any method')
  otherwise
     % For any other format, relay to MATLAB's built-in file reading
     [image,file_info] = mlread(filename,format);
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Read an image using MATLAB's IMREAD

function [image,file_info] = mlread(filename,format)

% Read in pixel data
if nargin < 2
   [image,map] = imread(filename);
else
   [image,map] = imread(filename,format);
end

% Convert to dip_image
if ~isempty(map)
   map = single(map*255);
   image = lut(image,map);
elseif ndims(image)==3
   if isa(image,'double')
      image = image*255;
   end
   image = dip_image(image);
   image = joinchannels('RGB',image);
else
   image = dip_image(image);
end

% Reading .png that set the sBit (e.g. exported by LabView) are not correctly
% scaled by matlab reader (at least in version 2008b)
[~,~,ext] = fileparts(filename);
if strcmp(ext,'.png')
   try
      fp = fopen(filename);
      header = fscanf(fp,'%c',200);
      fclose(fp);
      indx = findstr(header,'sBIT');
      bd = uint8(header(indx+4));
      if isnumeric(bd) && bd >0
         image = floor(image/(2^bd)); %equal to bit shift by bd
      end
   end
end

% Fill in the file_into structure
file_info.name = filename;
file_info.fileType = '';
file_info.dataType = datatype(image);
switch file_info.dataType
   case {'uint8','sint8'}
      file_info.significantBits = 8;
   case {'uint16','sint16'}
      file_info.significantBits = 16;
   case {'uint32','sint32'}
      file_info.significantBits = 32;
   otherwise
      file_info.significantBits = 0;
end
file_info.sizes = imsize(image);
file_info.tensorElements = prod(tensorsize(image));
file_info.colorSpace = colorspace(image);
file_info.pixelSize = [];
file_info.numberOfImages = 1;
file_info.history = {};

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Read an image using the Bio-Formats library

function [image,file_info] = bfread(filename)

% Print a warning if Bio-Formats is not installed
persistent haveBioFormats;
if isempty(haveBioFormats)
   haveBioFormats = bfCheckJavaPath();
   if ~haveBioFormats
      warning('Bio-Formats is not installed')
      disp('Bio-Formats is not installed. See HELP READIM for instructions on how to install it.')
      disp(['The JAR file should be installed here: ',fileparts(which('bfGetReader','in','readim'))])
   end
end
% Quick exit if we don't have Bio-Formats
if ~haveBioFormats
   error('Bio-Formats is not installed')
end

% Read image data
reader = bfGetReader(filename);
sz = [reader.getSizeC(),reader.getSizeY(),reader.getSizeX(),reader.getSizeZ(),reader.getSizeT()];
image = zeros(sz);
for t=1:sz(5)
   for c=1:sz(1)
      for z=1:sz(4)
         index = reader.getIndex(z-1,c-1,t-1)+1;
         tmp = bfGetPlane(reader,index);
         image(c,:,:,z,t) = tmp;
      end
   end
end
newsz = sz(2:5);
newsz(newsz==1) = [];
image = reshape(image,[sz(1),newsz]);
image = dip_image(image,sz(1));

% Read image metadata
omeMeta = reader.getMetadataStore();
pixelSize = struct('magnitude',{1,1,1,1},'units','px');
v = omeMeta.getPixelsPhysicalSizeX(0);
if ~isempty(v)
   pixelSize(1).magnitude = v.value(ome.units.UNITS.METER).doubleValue();
   pixelSize(1).units = 'm';
end
v = omeMeta.getPixelsPhysicalSizeY(0);
if ~isempty(v)
   pixelSize(2).magnitude = v.value(ome.units.UNITS.METER).doubleValue();
   pixelSize(2).units = 'm';
end
v = omeMeta.getPixelsPhysicalSizeZ(0);
if ~isempty(v)
   pixelSize(3).magnitude = v.value(ome.units.UNITS.METER).doubleValue();
   pixelSize(3).units = 'm';
end
pixelSize(sz(2:5)==1) = [];
image.PixelSize = pixelSize;
file_info.name = filename;
file_info.fileType = '';
file_info.dataType = datatype(image);
file_info.significantBits = omeMeta.getPixelsSignificantBits(0).getValue();
file_info.sizes = imsize(image);
file_info.tensorElements = prod(tensorsize(image));
file_info.colorSpace = colorspace(image);
file_info.pixelSize = pixelSize;
file_info.numberOfImages = 1;
file_info.history = {}; % TODO: read this from the metadata somehow?
