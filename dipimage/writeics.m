%WRITEICS   Write an image as an ICS file
%
% SYNOPSIS:
%  writeics(image,filename,history,significant_bits,options)
%
% PARAMETERS:
%  filename: the name for the file, including path. ".ics" will be appended
%        if necessary.
%  history: a cell array of strings to be written to the ICS file as history
%        strings.
%  significant_bits: the number of significant bits, or 0 to indicate all
%        bits are significant.
%  options: a string or a cell array of strings containing:
%        - 'v1' or 'v2': the ICS version ('v2' is default)
%        - 'gzip' or 'uncompressed': compression method ('gzip' is default)
%        - 'fast': write samples in same order they are in memory, yielding a
%          file with dimensions ordered as "y,x,z,...".
%
% DEFAULTS:
%  history = {}
%  significant_bits = 0
%  options = {'fast'}
%
% NOTE:
%  ICS is a file format that can store any image representable in DIPimage
%  (any dimensionality, any data type, any tensor representation). The pixel
%  sizes and tensor shape are also stored in the file.
%
% SEE ALSO:
%  readics, writeim, readim
%
% DIPlib:
%  This function calls the DIPlib function dip::ImageWriteICS.

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

function writeics(varargin)
dip_fileio('writeics',varargin{:});
