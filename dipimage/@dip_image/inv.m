%INV   Inverse of a square tensor image
%   INV(A) returns the inverse of A in the sense that INV(A)*A is equal
%   to EYE(A). A must be a square matrix image.
%
%   It is possible that the inverse does not exist for a pixel. In that
%   case output values can be Inf or NaN. Use PINV for a more robust
%   inverse, which also works for non-square matrices.
%
%   See also: DIP_IMAGE/PINV

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
