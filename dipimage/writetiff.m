%WRITETIFF   Write an image as a TIFF file
%
% SYNOPSIS:
%  writetiff(image,filename,compression,jpeg_level)
%
% PARAMETERS:
%  filename: the name for the file, including path. ".tif" will be appended
%        if there is no extension.
%  compression: a string indicating the compression method to use:
%        - 'none': no compression
%        - 'deflate': the gzip compression algorithm
%        - 'LZW': the LZW compression algorithm
%        - 'PackBits': run-length encoding
%        - 'JPEG': JPEG lossy compression
%  jpeg_level: if `compression` is 'JPEG', specifies the compression level as an
%        integer between 1 and 100, with 100 yielding the largest files and fewest
%        compression artifacts.
%
% DEFAULTS:
%  compression = '' (equal to 'deflate')
%  jpeg_level = 80
%
% NOTE:
%  The TIFF file format only writes 2D image data (3D will be implemented at some
%  point), and is only able to preserve pixel sizes if they are in length units
%  (meters). The tensor shape is lost, though all tensor components are written to
%  the file. Complex data is not supported, and color space information is lost
%  unless it is 'RGB', 'CMY' or 'CMYK'.
%
% NOTE:
%  TIFF files that are not binary, 8-bit or 16-bit unsigned integer are often
%  not recognized by other software.
%
% NOTE:
%  Compression method 'PackBits' should only be used with images with lots of
%  flat zones, where many pixels along one image line have the exact same value.
%  With a regular, non-noise-free image, this compression method results in
%  files that are larger than uncompressed files.
%
% SEE ALSO:
%  readtiff, writeim, readim
%
% DIPlib:
%  This function calls the DIPlib function dip::ImageWriteTIFF.

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

function writetiff(varargin)
dip_fileio('writetiff',varargin{:});
