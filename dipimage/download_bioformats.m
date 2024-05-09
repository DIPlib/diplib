%DOWNLOAD_BIOFORMATS   Download and install Bio-Formats for use in READIM
%
% Will fetch a Bio-Formats package and save it to the DIPimage/private
% directory, where it will be used by READIM to read a large assortment of
% image files. See READIM for more information.
%
% You might need to restart MATLAB when updating Bio-Formats.
%
% If there exists an installation of Bio-Formats outside of the DIPimage
% directory, and this other installation is on the Java class path, then the
% newly downloaded file might not be used.
%
% SYNOPSIS:
%  download_bioformats(update)
%  download_bioformats(version,update)
%
% PARAMETERS:
%  version: string with version to install, or 'latest'
%  update:  Boolean, whether to overwrite an existing installation or not
%
% DEFAULTS:
%  version = 'latest'
%  update = false
%
% NOTES:
%  This function will download the file
%     https://downloads.openmicroscopy.org/bio-formats/{version}/artifacts/bioformats_package.jar
%  and store it in the directory given by
%     fileparts(which('bfGetReader','in','readim'))
%
%  Note that Bio-Formats is licensed under GLPv2
%
% SEE ALSO:
%  readim

% (c)2024, Cris Luengo.
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

function download_bioformats(version, update)
if nargin < 2
   update = false;
   if nargin < 1
      version = 'latest';
   elseif ~isstring(version) && ~ischar(version)
      update = version;
      version = 'latest';
   end
end
if isstring(version)
   version = char(version);
end
if ~ischar(version)
   error('VERSION must be a string or character array')
end
if ~islogical(update)
   error('UPDATE must be TRUE or FALSE')
end

filename = fullfile(fileparts(mfilename('fullpath')), 'private', 'bioformats_package.jar');
if update || ~exist(filename, 'file')
   url = ['https://downloads.openmicroscopy.org/bio-formats/', version, '/artifacts/bioformats_package.jar'];
   disp(['Retrieving ', url])
   websave(filename, url);
else
   disp('Bio-Formats already present')
end
