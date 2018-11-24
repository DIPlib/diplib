%LEE   Lee operator
%  Implements a morphological edge detector based on the minimum of two complementary
%  morphological operations. These can be chosen through the EDGETYPE parameter.
%
% SYNOPSIS:
%  image_out = lee(image_in,filterSize,filterShape,edgeType,sign,boundary_condition)
%  image_out = lee(image_in,image_se,edgeType,sign,boundary_condition)
%
% PARAMETERS:
%  filterSize:  sizes of the filter along each image dimension
%  filterShape: 'rectangular', 'elliptic', 'diamond', 'parabolic'
%  image_se:    binary or grey-value image with the shape for the structuring element
%  edgeType:    'texture', 'object' or 'both'
%  sign:        'signed' or 'unsigned'
%  boundary_condition: Defines how the boundary of the image is handled.
%                      See HELP BOUNDARY_CONDITION
%
% DEFAULTS:
%  filterSize = 7
%  filterShape = 'elliptic'
%  edgeType = 'texture'
%  sign = 'unsigned'
%  boundary_condition = {} (equivalent to 'add min' in the dilation and 'add max' in
%                           the erosion)
%
%  The structuring element can be specified in two ways: through FILTERSIZE
%  and FILTERSHAPE, specifying one of the default shapes, or through IMAGE_SE,
%  providing a custom binary or grey-value shape.
%
%  When EDGETYPE is 'texture', the response is limited to edges in texture (i.e. scales
%  smaller than the structuring element). When it is 'object', the response is limited
%  to object edges (i.e. scales larger than the structuring element).
%
%  If SIGN is 'unsigned', absolute edge strength is computed. If it is 'signed', a
%  signed edge strength similar to the Laplace operator is computed.
%
% DIPlib:
%  This function calls the DIPlib function dip::Lee.

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

function out = lee(varargin)
out = dip_morphology('lee',varargin{:});
