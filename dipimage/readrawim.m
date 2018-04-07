%READRAWIM   Read image from RAW format file
%
% SYNOPSIS:
%  image_out = readrawim(filename,size,datatype)
%
% PARAMETERS:
%  filename: string with name of file (including extension), optionally
%            with path.
%  size:     array with image dimensions.
%  datatype: string with the name of one of the valid DIP_IMAGE data type
%            strings (see HELP DIP_IMAGE) or a MATLAB numeric class string.
%
% DEFAULTS:
%  datatype: 'uint8'
%
% NOTES:
%  READRAWIM will attempt to read the requested number of bytes from the
%  given file. If the file does not have enough data, an error will be
%  returned. However, if the file has more data, this additional data will
%  simply be ignored.
%
%  If the dimensions and data type given by the user are not correct, the
%  output will be giberish. Consider using any of the existing image file
%  formats to store your images in (e.g. ICS or TIFF).

% (c)2011, 2018, Cris Luengo.
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

function out = readrawim(filename,sz,dt)

if nargin<3
   dt = 'uint8';
else
   % Convert DIPimage type string to MATLAB type string
   if strcmp(dt,'sfloat')
      dt = 'single';
   elseif strcmp(dt,'dfloat')
      dt = 'double';
   elseif strcmp(dt(1:4),'sint')
      dt = dt(2:end);
   end
end

% Look for the file
[fid,errmsg] = fopen(filename,'rb');
if fid<0 && isempty(fileparts(filename)) % The file name has no path.
   p = convertpath(dipgetpref('imagefilepath'));
   for ii=1:length(p)
      [fid,errmsg] = fopen(fullfile(p{ii},filename),'rb');
      if fid>0
         break;
      end
   end
end
if fid<0
   error(errmsg);
end

% Read the file
[out,count] = fread(fid,prod(sz),['*',dt]);
fclose(fid);
if count<prod(sz)
   error('Insufficient data in the file to read an image of requested size and data type.');
end
if numel(sz)>1
   out = reshape(out,sz);
   out = permute(out,[2,1,3:numel(sz)]);
end
out = dip_image(out);
