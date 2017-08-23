%GETENDPIXEL   Get end-pixels from skeleton
%
% SYNOPSIS:
%  image_out = getendpixel(image_in, connectivity, edgeCondition)
%
% PARAMETERS:
%  connectivity: defines the neighborhood:
%     * 1 indicates 4-connected neighbors in 2D or 6-connected in 3D.
%     * 2 indicates 8-connected neighbors in 2D
%     * 3 indicates 28-connected neighbors in 3D
%  edgeCondition: defines the value of the pixels outside the image. It can
%     be 0/"background" or 1/"foreground".
%
% DEFAULTS:
%  connectivity = 0 (equal to ndims(image_in))
%  edgeCondition = "background"

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

function image_out = getendpixel(image_in, connectivity, edgeCondition)
if nargin < 3
   edgeCondition = "background";
   if nargin < 2
      connectivity = 0;
   end
end
image_out = countneighbors(image_in,"foreground",connectivity,edgeCondition) == 2;
