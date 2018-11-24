%READICS   Read an ICS file into an image
%
% SYNOPSIS:
%  [image,metadata] = readics(filename,origin,sizes,spacing)
%
% PARAMETERS:
%  filename: the name for the file, including path. ".ics" will be
%            appended if the file could not be found.
%  origin:   coordinates for the first pixel to read.
%  sizes:    size of the ROI to read.
%  spacing:  step size for reading a down-sampled ROI.
%
% DEFAULTS:
%  origin = [] (reads from the top-left corner)
%  sizes = [] (reads up to the bottom-right corner)
%  spacing = [] (does not subsample)
%
% SEE ALSO:
%  writeics, readim, writeim
%
% DIPlib:
%  This function calls the DIPlib function dip::ImageReadICS.

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

function varargout = readtiff(varargin)
varargout = cell(1,max(nargout,1));
[varargout{:}] = dip_fileio('readtiff',varargin{:});
