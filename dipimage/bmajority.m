%BMAJORITY   Binary majority voting
%
% SYNOPSIS:
%  image_out = bmajority(image_in,connectivity,edgeCondition)
%
% PARAMETERS:
%  connectivity: defines the neighborhood:
%     * 1 indicates 4-connected neighbors in 2D or 6-connected in 3D
%     * 2 indicates 8-connected neighbors in 2D
%     * 3 indicates 28-connected neighbors in 3D
%  edgeCondition: the value of pixels outside the image bounds,
%      can be 'background' or 'object', or equivalently 0 or 1.
%
% DEFAULTS:
%  connectivity = 0 (equal to ndims(image_in))
%  edgeCondition = 'background'

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

function out = bmajority(in,connectivity,edge)
if nargin<3
   edge = 'background';
   if nargin<2
      connectivity = 0;
   end
end
if connectivity == 0
   connectivity = ndims(in);
end
% How many neighbors are there in the neighborhood?
c = coordinates(repmat(3,1,ndims(in)),'cartesian');
c = tensortospatial(c);
c = sum(abs(c),[],2);
n = sum(c <= connectivity);
% Count number of set neighbors, threshold
out = countneighbors(in,'all',connectivity,edge);
out = out > n/2; % n is always an odd number
