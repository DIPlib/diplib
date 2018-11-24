%TOPHAT   Top-hat
%
% SYNOPSIS:
%  image_out = tophat(image_in,filterSize,filterShape,edgeType,polarity,boundary_condition)
%  image_out = tophat(image_in,image_se,edgeType,polarity,boundary_condition)
%
% PARAMETERS:
%  filterSize:  sizes of the filter along each image dimension
%  filterShape: 'rectangular', 'elliptic', 'diamond', 'parabolic'
%  image_se:    binary or grey-value image with the shape for the structuring element
%  edgeType:    'texture', 'object' or 'both' ('texture' is the common top-hat).
%  polarity:    'black' or 'white' ('black' is for "bottom-hat").
%  boundary_condition: Defines how the boundary of the image is handled.
%                      See HELP BOUNDARY_CONDITION
%
% DEFAULTS:
%  filterSize = 7
%  filterShape = 'elliptic'
%  edgeType = 'texture'  (regular top-hat)
%  polarity = 'white'
%  boundary_condition = {} (equivalent to 'add min' in the dilation and 'add max' in
%                           the erosion)
%
%  The structuring element can be specified in two ways: through FILTERSIZE
%  and FILTERSHAPE, specifying one of the default shapes, or through IMAGE_SE,
%  providing a custom binary or grey-value shape.
%
% EXPLANATION:
%  'texture','white' = in - opening(in)
%  'texture','black' = closing(in) - in
%  'object' ,'white' = opening(in)  - erosion(in)
%  'object' ,'black' = dilation(in) - closing(in)
%  'both'   ,'white' = in - erosion(in)
%  'both'   ,'black' = dilation(in) - in
%
% DIPlib:
%  This function calls the DIPlib function dip::Tophat.

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

function out = tophat(varargin)
out = dip_morphology('tophat',varargin{:});
