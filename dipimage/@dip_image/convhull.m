%CONVHULL  generates convex hull of a binary image.
%  CONVHULL(B) generates the (solid) convex hull of the binary image B.
%  CONVHULL(B,'no') or CONVHULL(B,0) generate only the outer shell (i.e.
%  the volume is not filled in).

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

function [out,indx] = convhull(in,fill)

if ~isscalar(in), error('Input image must be scalar.'); end
if ~islogical(in), error('Input image must be binary.'); end

if nargin < 2
   fill = true;
elseif ischar(fill)
   fill = strcmpi(fill,'yes');
elseif (~isnumeric(fill) && ~islogical(fill)) || ~isscalar(fill)
   error('Second argument should be ''yes'' or ''no'' (or 0 or 1)')
end

cc = findcoord(in-berosion(in,1,1,0)); % Only use edge pixels...

if ndims(in)==3

   % 3D Case: https://stackoverflow.com/a/2769455/7328782
   sz = imsize(in);
   sz = sz([2,1,3]);
   if exist('delaunayTriangulation','class')
      dt = delaunayTriangulation(cc); % Since R2013a
   else
      dt = DelaunayTri(cc); % No longer recommended
   end
   [X,Y,Z] = meshgrid(0:sz(1)-1,0:sz(2)-1,0:sz(3)-1);
   simplexIndex = pointLocation(dt,X(:),Y(:),Z(:));
   out = dip_image(reshape(isfinite(simplexIndex),sz));

elseif ndims(in)==2

   % 2D Case: simplified from the 3D case, but could make it faster
   % by using Oswaldo Cadenas' method to compute the convex hull, then
   % drawing it with drawpolygon.
   sz = imsize(in);
   sz = sz([2,1]);
   if exist('delaunayTriangulation','class')
      dt = delaunayTriangulation(cc); % Since R2013a
   else
      dt = DelaunayTri(cc); % No longer recommended
   end
   [X,Y] = meshgrid(0:sz(1)-1,0:sz(2)-1);
   simplexIndex = pointLocation(dt,X(:),Y(:));
   out = dip_image(reshape(isfinite(simplexIndex),sz));

   if nargout > 1
      indx = convexHull(dt);
   end

else
   error('Only implemented for 2D and 3D images.');
end

if ~fill
   out = out-berosion(out,1,1,0); % Only return edge pixels...
end
