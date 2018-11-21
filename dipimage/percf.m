%PERCF   Percentile filter
%
% SYNOPSIS:
%  image_out = percf(image_in,percentile,filterSize,filterShape,boundary_condition)
%  image_out = percf(image_in,percentile,neighborhood,boundary_condition)
%
% PARAMETERS:
%  percentile:   parameter for the filter, 50 leads to a median filter
%  filterSize:   sizes of the filter along each image dimension
%  filterShape:  'rectangular', 'elliptic', 'diamond'
%  neighborhood: binary image with the shape for the filtering kernel
%  boundary_condition: Defines how the boundary of the image is handled.
%                      See HELP BOUNDARY_CONDITION
%
%  The filter kernel can be specified in two ways: through FILTERSIZE
%  and FILTERSHAPE, specifying one of the default shapes, or through NEIGHBORHOOD,
%  providing a custom binary shape.
%
% DEFAULTS:
%  percentile = 50
%  filterSize = 7
%  filterShape = 'elliptic'
%  boundary_condition = 'mirror'
%
% DIPlib:
%  This function calls the DIPlib function dip::PercentileFilter.

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

function out = percf(varargin)
out = dip_filtering('percf',varargin{:});
