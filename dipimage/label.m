%LABEL   Label objects in a binary image
%
% SYNOPSIS:
%  image_out = label(image_in,connectivity,minSize,maxSize,boundary_condition)
%
% PARAMETERS:
%  connectivity: defines the metric, that is, the shape of the structuring
%     element.
%     * 1 indicates city-block metric, or a diamond-shaped S.E in 2D.
%     * 2 indicates chessboard metric, or a square structuring element in 2D.
%     For 3D images use 1, 2 or 3.
%  minSize, maxSize: minimum and maximum size of objects to be labeled. Set
%     either to zero to mean no limit.
%  boundary_condition: Defines how the boundary of the image is handled.
%     Either a single string or a cell array of strings. When giving only one
%     value, that value is applied to all image dimensions. Otherwise, provide a
%     value for each image dimension. An empty array indicates the default
%     behavior. Possible values are:
%     * '' or 'mirror': the default behavior, causing the labeling to simply
%       stop at the edges.
%     * 'periodic': imposing a periodic boundary condition, such that objects
%     touching opposite edges of the image are considered the same object.
%     * 'remove': causing objects that touch the image edge to be removed.
%
% DEFAULTS:
%  connectivity = 0 (equivalent to ndims(image_in))
%  minSize = 0
%  maxSize = 0
%  boundary_condition = {}
%
% DIPlib:
%  This function calls the DIPlib function <a href="https://diplib.org/diplib-docs/regions.html#dip-Label-Image-CL-Image-L-dip-uint--dip-uint--dip-uint--StringArray--String-CL">dip::Label</a>.

% (c)2017-2019, Cris Luengo.
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

function out = label(varargin)
out = dip_segmentation('label',varargin{:});
