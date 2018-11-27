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
%     See HELP BOUNDARY_CONDITION. It is generally ignored, except if
%     'periodic' is given for any dimension, in which case objects on either
%     side of the image along that dimension are considered one and receive
%     the same label.
%
% DEFAULTS:
%  connectivity = 0 (equivalent to ndims(image_in))
%  minSize = 0
%  maxSize = 0
%  boundary_condition = {}
%
% DIPlib:
%  This function calls the DIPlib function dip::Label.

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

function out = label(varargin)
out = dip_segmentation('label',varargin{:});
