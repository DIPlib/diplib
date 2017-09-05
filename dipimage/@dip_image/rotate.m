%ROTATE   Rotate the vector field in a vector image around an axis
%   ROTATE(V,AXIS,ANGLE) rotates the 3-vectors in image V about ANGLE
%   around the AXIS given by a second vector image or a vector.
%
%   ROTATE(V,ANGLE) rotates the 2-vectors in image V about ANGLE.
%
%   The ANGLE should be given between [0,2pi]
%
%   LITERATURE: Computer Graphics, D. Hearn and M.P. Baker, Prentice
%               Hall, p.408-419

% (c)2017, Cris Luengo.
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

function out=rotate(v,axis,theta)
if ~isa(v,'dip_image') || ~isvector(v)
   error('First input argument should be a vector image.');
end
switch(numtensorel(v))
   case 2
      if nargin~=2
         error('Two input arguments expected');
      end
      theta = axis;
      if ~isnumeric(theta)
         error('ANGLE input argument must be numeric');
      end
      theta = mod(theta, 2*pi);
      s = sin(theta);
      c = cos(theta);
      M = dip_image([c;-s;s;c],[2,2]);
      out = M*v;
      
   case 3
      if nargin~=3
         error('Three input arguments expected');
      end
      if ~isnumeric(theta) || numel(theta)~=1
         error('ANGLE input argument must be numeric scalar');
      end
      theta = mod(theta, 2*pi);
      if isnumeric(axis)
         axis = dip_image(axis(:),numel(axis));
      end
      if ~isa(axis,'dip_image') || ~isvector(axis) || numtensorel(axis)~=3
         error('AXIS needs to be 3-vector');
      end
      axis = axis./norm(axis).*sin(theta/2);
      a = subsref(axis,substruct('{}',{1}));
      b = subsref(axis,substruct('{}',{2}));
      c = subsref(axis,substruct('{}',{3}));
      aa = 2*a*a;
      ab = 2*a*b;
      ac = 2*a*c;
      bb = 2*b*b;
      bc = 2*b*c;
      cc = 2*c*c;
      s = cos(theta/2);
      sa = 2*s*a;
      sb = 2*s*b;
      sc = 2*s*c;
      M = cell(3,3);
      M{1,1} = 1-bb-cc;
      M{1,2} = ab-sc;
      M{1,3} = ac+sb;
      M{2,1} = ab+sc;
      M{2,2} = 1-aa-cc;
      M{2,3} = bc-sa;
      M{3,1} = ac-sb;
      M{3,2} = bc+sa;
      M{3,3} = 1-aa-bb;
      M = dip_image(M);
      out = M*v;
      
   otherwise
      error('First input argument should be a 2- or 3-vector image.');
end
