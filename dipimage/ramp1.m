%RAMP1   Creates a 1D image with one cartesian coordinate
%   RAMP1(SIZE,DIM) returns an image of size SIZE(DIM) along dimension
%   DIM, and size 1 along other dimensions, with the value of the
%   DIM dimension's coordinate as the pixel values.
%
%   Singleton expansion of RAMP1(SIZE,DIM) to SIZE yields the same
%   result as RAMP(SIZE,DIM). Because of implicit singleton expansion
%   in arithmetic, it is often faster and more memory-efficient to
%   use RAMP1 over RAMP.
%
%   RAMP1(IMG,DIM) is the same as RAMP1(SIZE(IMG),DIM).
%
%   RAMP1(...,ORIGIN,OPTIONS) allows specifying the origin and additional
%   options, see COORDINATES for details.
%
%   SEE ALSO: coordinates, ramp, xx1, yy1, zz1, rr, phiphi, thetatheta

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

function out = ramp1(sz,dim,varargin)
if nargin<1
   sz = [256,256];
end
if nargin<2
   dim = 1;
end
if isa(sz,'dip_image')
   sz = imsize(sz);
elseif ~isvector(sz)
   error('First input argument expected to be a dip_image or a size array')
end
nd = length(sz);
I = true(1,nd);
if nd >= dim
    I(dim) = false;
end
sz(I) = 1;
out = coordinates(sz,dim,varargin{:});
