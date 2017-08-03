%READIM   Read image from file
%
% SYNOPSIS:
%  [image_out,file_info] = readim(filename,format)
%
% OUTPUT:
%  image_out: the image
%  file_info: additional parameters of the image (if avaiable) returned as a
%             struct, see DIPIO_IMAGEFILEGETINFO for more information.
%
% PARAMETERS:
%  filename: string with name of file, optionally with path and extension.
%  format:   string with any of: 'ICS', 'TIFF', a format string recognized by
%            IMREAD, or 'bioformats'.
%
% DEFAULTS:
%  format = ''
%
% NOTES:
%  If FORMAT is '', then the format will be guessed from the file name extension
%  or the file contents. If READICS and READTIFF cannot read the file, this
%  function attempts to read it using Bio-Formats. If that also fails, IMREAD
%  is tried. The FILE_INFO output structure is empty if the file was read through
%  IMREAD.
%
%  If FORMAT is specified, then either READICS, READTIFF or IMREAD are called.
%  See IMREAD for a list of formats it recognizes.
%  If FORMAT is 'bioformats', then only the Bio-Formats reader is used.
%
%  Format 'TIF' is an alias for 'TIFF'. Likewise, 'ICS' is an alias for 'ICSv2'.
%
%  To read multi-page TIFF files use READTIFF. See also READTIMESERIES.
%
%  The functions READICS, READTIFF and IMREAD offer more options, consider
%  calling them directly.
%
% SEE ALSO:
%  writeim, readics, readtiff, imread, readtimeseries

% (c)2017, Cris Luengo.
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
   format = ''
end
if nargin < 1
   error('A file name is required')
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
      case {'ICSV2','ICS2','ICSv1','ICS1'}
         format = 'ICS';
      case 'TIF'
         format = 'TIFF';
   end
end

% Read
switch upper(format)
  case 'ICS'
     [image,file_info] = readics(filename);
  case 'TIFF'
     [image,file_info] = readtiff(filename);
  case 'BIOFORMATS'
     [image,file_info] = bfread(filename);
  case ''
     % If no format is given, try each of the readers in turn
     try
        [image,file_info] = readics(filename);
        return
     end
     try
        [image,file_info] = readtiff(filename);
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
switch file_info.datatype
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
file_info.pixelSize = image.PixelSize;
file_info.numberOfImages = 1;
file_info.history = {};

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Read an image using the Bio-Formats library

function [image,file_info] = bfread(filename)

% TODO!
error('Not yet implemented')
