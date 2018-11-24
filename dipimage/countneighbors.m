%COUNTNEIGHBORS   Counts the number of set neighbors for each pixel
%
% SYNOPSIS:
%  image_out = countneighbors(image_in, mode, connectivity, edgeCondition)
%
%  IMAGE_OUT will contain the number of set neighbors + 1 for each set pixel,
%  and 0 for each non-set pixel. If MODE is 'all', the output will contain
%  the number of set neighbors for each non-set pixel. That is, for each
%  pixel it returns the count of set pixels in the neighborhood including
%  the origin.
%
% PARAMETERS:
%  mode: perform the count for only foreground pixels ('foreground' or 1)
%     or all pixels ('all' or 0)
%  connectivity: defines the neighborhood:
%     * 1 indicates 4-connected neighbors in 2D or 6-connected in 3D.
%     * 2 indicates 8-connected neighbors in 2D
%     * 3 indicates 28-connected neighbors in 3D
%  edgeCondition: the value of pixels outside the image bounds,
%      can be 'background' or 'object', or equivalently 0 or 1.
%
% DEFAULTS:
%  mode = 'foreground'
%  connectivity = 0 (equal to ndims(image_in))
%  edgeCondition = 'background'
%
% DIPlib:
%  This function calls the DIPlib function dip::CountNeighbors

% (c)2017-2018, Cris Luengo.
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

function out = countneighbors(varargin)
out = dip_morphology('countneighbors',varargin{:});
