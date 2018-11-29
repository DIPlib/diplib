%EXTENDREGION   Extends a region in the image
%
% SYNOPSIS:
%  image_out = extendregion(image_in,ranges,boundary_condition)
%  image_out = extendregion(image_in,origin,sizes,boundary_condition)
%
%  RANGES is a cell array with pairs, one for each image dimension,
%  indicating the image region to extend. The first value of each pair is
%  the first pixel in the region, and the second one is the last pixel in
%  the region (both included). For example, [5,10] is equivalent to
%  indexing 5:10 along that dimension. Negative numbers count from the
%  end, with -1 indicating the last pixel.
%
%  ORIGIN and SIZES are an alternative way of specifying the region to
%  extend. ORIGIN is an array for the coordinates of the top-left pixel
%  in the region, and SIZES gives the sizes of the region.
%
%  Note that RANGES(I) = ORIGIN(I)+[0,SIZES(I)-1] .
%
%  BOUNDARY_CONDITION is a string or a cell array of strings (one per image
%  dimension) specifying how the pixel values outside of the original image
%  domain are to be filled. See HELP BOUNDARY_CONDITION.
%
% DEFAULTS:
%  boundary_condition = 'mirror'
%
% SEE ALSO:
%  extend
%
% DIPlib:
%  This function calls the DIPlib function dip::ExtendRegion.

% (c)2017-2018, Cris Luengo.
% Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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

function out = extendregion(varargin)
out = dip_generation('extendregion',varargin{:});
