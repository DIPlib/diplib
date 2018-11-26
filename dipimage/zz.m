%ZZ   Creates an image with the z-axis cartesian coordinate
%   ZZ(SIZE) returns an image of size SIZE with the value of the z-axis
%   coordinate as the pixel values.
%
%   ZZ(IMG) is the same as ZZ(SIZE(IMG)).
%
%   ZZ(...,ORIGIN,OPTIONS) allows specifying the origin and additional
%   options, see COORDINATES for details.
%
%   SEE ALSO: coordinates, ramp, xx, yy, zz1, rr, phiphi, thetatheta

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

function out = zz(sz,varargin)
if nargin<1
   sz = [256,256];
elseif ischar(sz)
   varargin = [{sz},varargin];
   sz = [256,256];
end
out = coordinates(sz,3,varargin{:});
