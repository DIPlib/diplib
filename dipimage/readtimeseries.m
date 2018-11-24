%READTIMESERIES   Reads a series/stack of TIFF images as a 3D image
%
% SYNOPSIS
%  out = readtimeseries(basefilename,extension,range,color,verbose)
%
%  basefilename : File name without the running number or full file name
%  extension    : File extension
%  range        : First and last index to read [first last]
%  color        : Conserve color information? 'no','yes'
%  verbose      : Show diagnostic messages on which files are read
%
% DEFAULTS:
%  extension    : ''
%  range        : []   % All found images are read
%  color        : no
%  verbose      : no
%
% EXAMPLE:
%  In the directory /data/here, there are 16 files:
%     myfile4.tif,myfile5.tif,myfile006.tif,...myfile020.tif
%  out = readtimeseries('/data/here/myfile','tif')  or equivalent
%  out = readtimeseries('/data/here/myfile012.tif') or equivalent
%  out = readtimeseries('/data/here/myfile*.tif')
%
% SEE ALSO:
%  readim, readtiff
%
% NOTES:
%  For files like myleica_z004_ch01.tif etc. use
%  readtimeseries('myleica_z*_ch01.tif')
%
%  The given file name can have only one wildcard '*'. The directory
%  name or the extension cannot. The wildcard can be anywhere in the
%  name of the file.
%
%  If the file as given is not found, and no path was specified, then the
%  directories listed in the 'ImageFilePath' property (see DIPSETPREF) are
%  searched for the file.
%
% DIPlib:
%  This function calls the DIPlib function dip::ImageReadTIFFSeries.

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

function out=readtimeseries(basefilename,extension,range,color,verbose)

if nargin < 2
   extension = '';
end
if nargin < 3 || isempty(range)
   range = [0,0];
elseif numel(range)~=2
   error('The range parameter must be of the form [start ending]');
end

if nargin < 4
   color = false;
else
   if ischar(color)
      switch color
         case {'yes','on'}
            color = true;
         case {'no','off'}
            color = false;
         otherwise
            error('Cannot parse COLOR parameter')
      end
   end
end

if nargin < 5
   verbose = false;
else
   if ischar(verbose)
      switch verbose
         case {'yes','on'}
            verbose = true;
         case {'no','off'}
            verbose = false;
         otherwise
            error('Cannot parse VERBOSE parameter')
      end
   end
end

fns = find_files(basefilename,extension,range(1),range(2));
if isempty(fns)
   fns = {basefilename};
end
M = length(fns);

% Read in the image files
if M==1
   if verbose
      disp(['Reading multi-page tiff file ',fns{1},' as 3D image.']);
   end
   if range==[0,0]
      range = [0,-1];
   end
   out = dip_fileio('readtiff',fns{1},range);
else
   if verbose
      disp('Reading the following files:');
      disp(char(fns(:)));
   end
   out = dip_fileio('readtiffseries',fns);
end
if ~color
   out = colorspace(out,'grey');
end


%FIND_FILES return sorted files
%
% out = find_files(filebase,ext,start,ending)
%
%  filebase: string containing the base filename
%  ext     : extension
%  start   : first index to read
%  ending  : last index to read
%
% DEFAUTLS:
%  start  = 0 % start with the first found image
%  ending = 0 % read all found images

function out=find_files(filebase,ext,start,ending)

if sum(filebase=='*')>1
   error('Only one ''*'' wildcard accepted.')
elseif sum(filebase=='*')==1
   wc = 1;
else
   wc = 0;
end

[pathstr,basename,extension] = fileparts(filebase);
if ~isempty(ext)
   if ~strcmp(ext,extension) % the extension doesn't match the given one -- it's not the extension
      basename = [basename extension];
      extension = '';
   end
end
if length(extension) > 5 % someone puts dots in the filename
   basename = [basename extension];
   extension = '';
end
if ~isempty(extension) && isempty(ext)
   ext = extension;
end
if wc && ~any(basename=='*')
   error('A wildcard ''*'' is only accepted in the file name, not the extension nor the directory name.')
end

if ~wc
   % remove trailing numbers and substitute them with a '*'
   while ~isempty(basename) && any(basename(end)=='0123456789')
      basename(end) = [];
   end
   basename = [basename,'*'];
end
if ~isempty(ext)
   basename = [basename,ext];
end
fns = dir(fullfile(pathstr,basename));
fns([fns.isdir]) = [];
if isempty(fns) && isempty(pathstr)
   p = convertpath(dipgetpref('imagefilepath'));
   for ii=1:length(p)
      fns = dir(fullfile(p{ii},basename));
      fns([fns.isdir]) = [];
      if ~isempty(fns)
         pathstr = p{ii};
         break;
      end
   end
end
if isempty(fns)
   out = {};
   return;
end

if isempty(ext)
   % No extension given -- let's use the first extension we find!
   [~,~,ext] = fileparts(fns(1).name);
   basename = [basename,ext];
end

jj = find(basename=='*');
if length(jj)~=1
   error('assertion failed!')
end
kk = length(basename)-jj;
xx = length(ext)-1;

Nout =  length(fns);
out = cell(1,Nout);
iout = zeros(1,Nout);
for ii=1:Nout
   out{ii} = fns(ii).name;
   if ~strcmp(out{ii}(end-xx:end),ext)
      iout(ii) = -1;
   else
      iout(ii) = str2double(out{ii}(jj:end-kk));
      if isnan(iout(ii))
         iout(ii) = -1;
      end
   end
   out{ii} = fullfile(pathstr,out{ii});
end

I = iout >= start;
if ending > start
   I = I & (iout <= ending);
end
out = out(I);
iout = iout(I);
[iout,I] = sort(iout);
out = out(I);
