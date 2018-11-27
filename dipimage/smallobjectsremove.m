%SMALLOBJECTSREMOVE   Removes small objects from binary or labeled image
%
% SYNOPSIS:
%  image_out = smallobjectsremove(image_in,threshold,connectivity)
%
% PARAMETERS:
%  threshold: the smallest size (area) of the objects to keep.
%  connectivity: defines the metric, that is, the shape of the structuring
%     element.
%     * 1 indicates city-block metric, or a diamond-shaped S.E in 2D.
%     * 2 indicates chessboard metric, or a square structuring element in 2D.
%     For 3D images use 1, 2 or 3.
%
% DEFAULTS:
%  threshold = 10
%  connectivity = 1
%
%  If IMAGE_IN is labeled, the area of each label is measured, and only those
%  labels that have at least `threshold` pixels are kept. CONNECTIVITY is
%  ignored.
%
%  If IMAGE_IN is binary, the function LABEL is called with MINSIZE set to
%  THRESHOLD, and the result is binarized again. CONNECTIVITY is passed to the
%  labeling function.
%
% DIPlib:
%  This function calls the DIPlib function dip::SmallObjectsRemove.

% (c)2017-2018, Cris Luengo.
% Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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

function out = smallobjectsremove(varargin)
out = dip_segmentation('smallobjectsremove',varargin{:});
