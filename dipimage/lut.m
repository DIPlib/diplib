%LUT   Look-up Table (with interpolation)
%
% SYNOPSIS:
%  image_out = lut(image_in,table)
%
% PARAMETERS:
%  table: an array with 1 or 3 columns (respectively produce a grey-value
%         image and an RGB image). MATLAB has many commands to create
%         colormaps (see 'help graph3d'). These should be used in this
%         way: lut(image,jet(256)*255)
%
% NOTES:
%  Indexing into the table starts at zero: pixel values should be
%  between 0 and LENGTH(TABLE)-1.
%
%  For integer images, the pixel values are table indices. For floating-
%  point images, the table-lookup is done with interpolation.
%
%  The output image has the same data type as TABLE.
%
%  TABLE can also be a 1D tensor image, where the output image will have the
%  same number of tensor elements, or a 2D scalar image of the same sizes
%  as that of the TABLE array input described above.
%
% DIPlib:
%  This function calls the DIPlib function dip::LookupTable::Apply.

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

% TODO: lut(image_in,table,indices) adds indices to the table.
% TODO: lut(...,interpolation_mode,out_of_bounds): interpolation_mode is string, out_of_bounds is string or scalar or [scalar,scalar]
