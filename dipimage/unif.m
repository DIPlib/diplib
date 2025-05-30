%UNIF   Uniform filter, mean filter, convolution with uniform weights
%
% SYNOPSIS:
%  image_out = unif(image_in,filterSize,filterShape,boundary_condition)
%  image_out = unif(image_in,neighborhood,boundary_condition)
%
% PARAMETERS:
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
%  The NEIGHBORHOOD is applied as-is as a neighborhood, without mirroring. To compute
%  a convolution, mirror NEIGHBORHOOD first.
%
% DEFAULTS:
%  filterSize = 7
%  filterShape = 'elliptic'
%  boundary_condition = 'mirror'
%
% DIPlib:
%  This function calls the DIPlib function <a href="https://diplib.org/diplib-docs/linear.html#dip-Uniform-Image-CL-Image-L-Kernel-CL-StringArray-CL">dip::Uniform</a>.

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

function out = unif(varargin)
out = dip_filtering('unif',varargin{:});
