%CROSS   Cross product of two vector images.
%   CROSS(A,B) returns the cross product of the vector images A and B.
%   The cross product results in a vector image, and is only defined
%   for vectors with 2 or 3 components.
%
%   For 2-vectors, we define the cross product as the z component of
%   the cross product of 3-vectors with 0 z-component. That is, we add
%   a 0 as the 3rd vector component, and compute the cross product
%   ignoring the two first components of the result, which are zero.
%
%   Either A or B can be a normal vector such as [1,0,0].
%
%   See also: DIP_IMAGE/DOT, DIP_IMAGE/MTIMES

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
