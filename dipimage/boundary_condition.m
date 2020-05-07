%BOUNDARY_CONDITION   Information on the BOUNDARY_CONDITION parameter (not a function)
%
%   Many filters require a BOUNDARY_CONDITION. This parameter determines how the
%   pixel data is extended beyond the image boundary during filtering. The
%   parameter is either a single string or a cell array of strings. When giving
%   only one value, that value is applied to all image dimensions. Otherwise,
%   provide a value for each image dimension. An empty array indicates the
%   default behavior, which is equivalent to 'mirror'. Other possible values
%   are:
%
%   'mirror'        The data is mirrored, with the value at -1 equal to the
%                   value at 1, at -2 equal to at 2, etc. The value at 0 is
%                   not replicated
%   'asym mirror'   The data is mirrored and inverted.
%   'periodic'      The data is repeated periodically, with the value at -1
%                   equal to the value of the last pixel.
%   'asym periodic' The data is repeated periodically and inverted.
%   'add zeros'     The boundary is filled with zeros.
%   'add max'       The boundary is filled with the max value for the data type.
%   'add min'       The boundary is filled with the min value for the data type.
%   'zero order'    The value at the border is repeated indefinitely.
%   'first order'   A linear function is defined based on the value closest
%                   to the border, the function reaches zero at the end of the
%                   extended boundary.
%   'second order'  A quadratic function is defined based on the two values
%                   closest to the border, the function reaches zero at the end
%                   of the extended boundary.
%   'third order'   A cubic function is defined based on the two values closest
%                   to the border, the function reaches zero with a zero
%                   derivative at the end of the extended boundary.

% (c)2016-2017, Cris Luengo.
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
