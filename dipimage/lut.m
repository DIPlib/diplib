%LUT   Look-up Table (with interpolation)
%
% SYNOPSIS:
%  image_out = lut(image_in,table,indices,method,bounds)
%  image_out = lut(image_in,table,method,bounds)
%
% PARAMETERS:
%  table: an array with 1 or 3 columns (respectively produce a grey-value
%        image and an RGB image). MATLAB has many commands to create
%        colormaps (see 'help graph3d'). These should be used in this
%        way: lut(image,jet(256)*255)
%  indices: an array with as many elements as there are rows in TABLE,
%        representing the input value that gets mapped to each row.
%  method: the interpolation method:
%        - 'linear': the default, uses linear interpolation.
%        - 'nearest': rounds the input value to the nearest index.
%        - 'zero order': maps the value to the nearest lower index.
%  bounds: how out-of-bounds input values are handled:
%        - 'clamp': clamp input values to the valid range.
%        - 'keep': keep input values that are out of range.
%        - V: use the value V for each pixel that is out of range.
%        - [L,U]: use the value L for pixels under the range, and U for
%          pixels over the range.
%
% DEFAULTS:
%  indices: [] (equivalent to 0:LENGTH(TABLE)-1)
%  method: 'linear'
%  bounds: 'clamp'
%
% NOTES:
%  If no INDICES are given (or if it's an empty array) and IMAGE_IN is an
%  integer image, no interpolation will be applied, since each pixel exactly
%  matches an index.
%
%  The output image has the same data type as TABLE.
%
%  TABLE can also be a 1D color image, where the output image will have the
%  same color space, or a tensor image. If it is a 2D array or 2D scalar image,
%  then the output will have one tensor element per column.
%
% DIPlib:
%  This function calls the DIPlib function dip::LookupTable::Apply and several
%  other methods of the dip::LookupTable class.

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
