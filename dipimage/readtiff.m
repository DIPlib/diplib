%READTIFF   Read a TIFF file into an image
%
% SYNOPSIS:
%  [image,metadata] = readtiff(filename,indices,origin,sizes,spacing)
%
% PARAMETERS:
%  filename: the name for the file, including path. ".tif" and ".tiff" will be
%            appended if the file could not be found and there is no extension.
%  indices:  a range indicating which images within the multi-page TIFF to read.
%            A range is written as a single integer (indicating a single image),
%            two integers [start,stop] indicating the first and last images to
%            read, or three integers [start,stop,step] indicating also the step
%            size. Start and stop values can be negative to indicate counting
%            from the last one. For example, [0,-1] indicates all images in the
%            file. And [0,-1,2] indicates every other image, starting at the
%            first one.
%  origin:   coordinates for the first pixel to read.
%  sizes:    size of the ROI to read.
%  spacing:  step size for reading a down-sampled ROI.
%
% DEFAULTS:
%  indices = 0
%  origin = [] (reads from the top-left corner)
%  sizes = [] (reads up to the bottom-right corner)
%  spacing = [] (does not subsample)
%
% NOTE:
%  When more than one image is requested, the images must all be of the same size
%  and data type. They will be catenated along the 3rd dimension.
%
% SEE ALSO:
%  writetiff, readim, writeim, readtimeseries
%
% DIPlib:
%  This function calls the DIPlib function dip::ImageReadTIFF.

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
