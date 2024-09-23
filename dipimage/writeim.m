%WRITEIM   Write an image to file
%
% SYNOPSIS:
%  writeim(image,filename,format)
%
% PARAMETERS:
%  filename:    string with name of file, optionally with path and extension.
%  format:      any one string of: 'ICSv1', ICSv2', 'TIFF', 'NPY', or a string
%               recognized by IMWRITE.
%
% DEFAULTS:
%  format = ''
%
% NOTES:
%  If FORMAT is '', then the format will be guessed from the file name extension.
%  If there is no extension, then 'ICSv2' is the default. See WRITEICS for more
%  information on the default file format.
%
%  Format 'TIF' is an alias for 'TIFF'. Likewise, 'ICS' is an alias for 'ICSv2'.
%
%  See IMWRITE for a list of formats it recognizes.
%
%  The functions WRITEICS, WRITETIFF and IMWRITE offer more options, consider
%  calling them directly.
%
% SEE ALSO:
%  readim, writeics, writetiff, imwrite

% (c)2017-2022, Cris Luengo.
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


function writeim(image,filename,format)

if nargin < 3
   format = '';
end
if nargin < 2
   error('The first two input arguments are required')
end

if isempty(format)
   % If we don't have a format, look for the file name extension
   [~,~,ext] = fileparts(filename);
   if isempty(ext)
      format = 'ICSv2';
   else
      if ext(1)=='.'
         ext = ext(2:end);
      end
      switch upper(ext)
         case 'ICS'
            format = 'ICSv2';
         case {'TIF','TIFF'}
            format = 'TIFF';
         otherwise
            format = ext;
      end
   end
else
   % Format aliases. Don't check for these if we found format trough the file name extension
   switch upper(format)
      case {'ICS','ICS2'}
         format = 'ICSv2';
      case 'ICS1'
         format = 'ICSv1';
      case 'TIF'
         format = 'TIFF';
   end
end

if ~isa(image,'dip_image')
   image = dip_image(image); % So that DATATYPE and COLORSPACE work
end

% Write
switch upper(format)
   case 'ICSV1'
      dip_fileio('writeics',image,filename,{},0,{'v1','fast'});
   case 'ICSV2'
      dip_fileio('writeics',image,filename);
   case 'TIFF'
      if dipgetpref('FileWriteWarning')
         warning('You are writing a ZIP compressed TIFF. Older image viewers may not be able to read this compression.');
         dt = datatype(image);
         if ~any(strcmp(dt,{'uint8','uint16','bin'}))
            warning(['You are writing a ',dt,' TIFF. This is not supported by most image viewers.'])
         end
      end
      dip_fileio('writetiff',image,filename);
   case 'NPY'
      dip_fileio('writenpy',image,filename);
   otherwise
      % For any other format, relay to MATLAB's built-in file writing
      if dipgetpref('FileWriteWarning')
         if ~strcmp(datatype(image),'uint8')
            warning('Converting the image to uint8 for writing using MATLAB''s imwrite function')
         end
      end
      if iscolor(image)
         image = colorspace(image,'sRGB');
      end
      if ~isscalar(image)
         image = tensortospatial(image,ndims(image)+1);
      end
      image = uint8(image);
      imwrite(image,filename,format);
end
