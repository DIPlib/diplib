%RANKMAX_OPENING   Rank-max opening
%
% SYNOPSIS:
%  image_out = rankmax_opening(image_in,rank,filterSize,filterShape,boundary_condition)
%  image_out = rankmax_opening(image_in,rank,image_se,boundary_condition)
%
% PARAMETERS:
%  rank:        the number of samples within the SE to ignore.
%  filterSize:  sizes of the filter along each image dimension
%  filterShape: 'rectangular', 'elliptic', 'diamond', 'parabolic'
%  image_se:    binary or grey-value image with the shape for the structuring element
%  boundary_condition: Defines how the boundary of the image is handled.
%                      See HELP BOUNDARY_CONDITION
%
% DEFAULTS:
%  rank = 2
%  filterSize = 7
%  filterShape = 'elliptic'
%  boundary_condition = {} (equivalent to 'add min' in the dilation and 'add max' in
%                           the erosion)
%
%  The structuring element can be specified in two ways: through FILTERSIZE
%  and FILTERSHAPE, specifying one of the default shapes, or through IMAGE_SE,
%  providing a custom binary or grey-value shape.
%
% EXAMPLE:
%  a = 1 - dip_image(noise((rr(40,40) < 20),'binary',0.04,0),'uint8')
%  b = rankmax_opening(a,2,3,'rectangular')
%
% DIPlib:
%  This function calls the DIPlib function dip::RankMaxOpening.

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

function out = rankmax_opening(varargin)
out = dip_morphology('rankmax_opening',varargin{:});
