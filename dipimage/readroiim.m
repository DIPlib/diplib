%READROIIM   Read ROI of an image from file
%
% This function is useful for large image files, if only a smaller region is
% needed for processing, or to extract a thumbnail from it.
%
% SYNOPSIS:
%  [image,metadata] = readroiim(filename,spacing,origin,sizes,format)
%
% PARAMETERS:
%  filename: string with name of file, optionally with path and extension.
%  spacing:  step size for reading a down-sampled ROI.
%  origin:   coordinates for the first pixel to read.
%  sizes:    size of the ROI to read.
%  format:   either 'ICS' or 'TIFF', or an empty string, which will cause
%            the function to search for the correct type.
%
% DEFAULTS:
%  filename = 'erika.ics'
%  spacing = 4
%  origin = []
%  sizes = []
%  format = ''
%
% EXAMPLE:
%  out = readroiim('erika',[2 2],[64 64],[128 128])
%
% NOTE:
%  For TIFF files, only the first image in the file is read. To control
%  which image directroy to read, use the READTIFF function directly.

% (c)2018, Cris Luengo.
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

function [image,file_info] = readroiim(filename,spacing,origin,sizes,format)

if nargin < 1
   filename = 'erika.ics';
end
if nargin < 2
   spacing = 4;
end
if nargin < 3
   origin = [];
end
if nargin < 4
   sizes = [];
end
if nargin < 5
   format = '';
end

% ORIGIN, SIZES and SPACING should be vectors with 0, 1 or more elements.
if ~isempty(spacing) && ~isvector(spacing)
   error('SPACING should be a vector with 0, 1 or NDIMS elements.')
end
if ~isempty(origin) && ~isvector(origin)
   error('ORIGIN should be a vector with 0, 1 or NDIMS elements.')
end
if ~isempty(sizes) && ~isvector(sizes)
   error('SIZES should be a vector with 0, 1 or NDIMS elements.')
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
   format = upper(format);
   switch format
      case {'ICS','ICSV2','ICS2','ICSV1','ICS1'}
         format = 'ICS';
      case {'TIF','TIFF'}
         format = 'TIFF';
      case ''
         % no action
      otherwise
         error('Unkown format, currently only ICS and TIFF files are supported')
   end
end

try
   [image,file_info] = readroiim_core(filename,origin,sizes,spacing,format);
   return
end
if isempty(fileparts(filename)) % The file name has no path.
   p = convertpath(dipgetpref('imagefilepath'));
   for ii=1:length(p)
      try
         [image,file_info] = readroiim_core(fullfile(p{ii},filename),origin,sizes,spacing,format);
         return
      end
   end
end
% TODO: read the file using IMREAD and crop to the requested ROI
error('Could not open the file for reading')

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Core: read the given file

function [image,file_info] = readroiim_core(filename,origin,sizes,spacing,format)
switch format
  case 'ICS'
     [image,file_info] = dip_fileio('readics',filename,origin,sizes,spacing);
  case 'TIFF'
     [image,file_info] = dip_fileio('readtiff',filename,0,origin,sizes,spacing);
  case ''
     % If no format is given, try each of the readers in turn
     try
        [image,file_info] = dip_fileio('readics',filename,origin,sizes,spacing);
        return
     end
     try
        [image,file_info] = dip_fileio('readtiff',filename,0,origin,sizes,spacing);
        return
     end
     error('The file is not an ICS or TIFF file')
  otherwise
     error('Unrecognized format, currently only ICS and TIFF files are supported')
end
