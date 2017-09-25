%THETATHETA   Creates an image with the theta component of spherical coordinates
%   THETATHETA(SIZE) returns an image of size SIZE with the value of the
%   theta component of the spherical coordinate as the pixel values. Theta is the
%   angle to the z-axis. SIZE must have 3 components.
%
%   THETATHETA(IMG) is the same as THETATHETA(SIZE(IMG)).
%
%   THETATHETA(...,ORIGIN) allows specifying where the origin is:
%    - 'left':        The pixel to the left of the true center.
%    - 'right':       The pixel to the right of the true center (default).
%    - 'true':        The true center, between pixels if required.
%    - 'corner':      The pixel on the upper left corner (indexed at (0,0)).
%    - 'frequency':   Uses frequency domain coordinates, in the range [-0.5,0.5),
%                     corresponds to coordinate system used by FT.
%   Note that the first three are identical if the size is odd.
%
%   THETATHETA(...,ORIGIN,OPTIONS) further specifies one or both of these options:
%    - 'radial':      When 'frequency' is selected as the origin, causes it to
%                     use radial frequencies instead, making the range [-pi,pi).
%    - 'math':        Let the Y coordinate increase upwards instead of downwards.
%   To provide both options, join the strings in a cell array
%
%  It is possible to set ORIGIN to 'radfreq', which combines 'frequency' with
%  'radial'.
%
%  Prepending an 'm' to any option for ORIGIN is equivalent to setting the
%  'math' option. That is, THETATHETA(...,'mleft') is equivalent to
%  THETATHETA(...,'left','math').
%
% SEE ALSO:
%  coordinates, ramp, xx, yy, zz, rr, phiphi

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

function out = thetatheta(sz,varargin)
if nargin<1
   sz = [256,256];
elseif ischar(sz)
   varargin = [{sz},varargin];
   sz = [256,256];
end
out = coordinates(sz,'theta',varargin{:});
