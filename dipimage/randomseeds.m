%RANDOMSEEDS   Create an image with randomly placed points
%
% SYNOPSIS:
%  image_out = randomseeds(sizes,density,type,mode)
%
% PARAMETERS:
%  sizes:      Output image sizes. If an image is given, the sizes of the
%              image are used. The output image is always binary.
%  density:    Density of the points.
%  type:       Determines how the points are placed. One of:
%              - 'rectangular': rectangular grid, any number of dimensions.
%              - 'hexagonal': hexagonal grid, 2D only.
%              - 'fcc': face centered cubic grid, 3D only.
%              - 'bcc': body centered cubic grid, 3D only.
%              - 'poisson': Poisson point process (MODE is ignored).
%  mode:       How the grid is randomly placed. One of:
%              - 'translation': the grid is randomly translated.
%              - 'rotation': the grid is randomly translated and rotated, 2D
%                and 3D only.
%
% DEFAULTS:
%  sizes = [256,256]
%  density = 0.01
%  type = 'rectangular'
%  mode = 'translation'
%
% DIPlib:
%  This function calls the DIPlib functions <a href="https://diplib.org/diplib-docs/generation_test.html#dip-CreateRandomGrid-Image-L-UnsignedArray-CL-Random-L-dfloat--String-CL-String-CL">dip::CreateRandomGrid</a> and
%  <a href="https://diplib.org/diplib-docs/generation_test.html#dip-CreatePoissonPointProcess-Image-L-UnsignedArray-CL-Random-L-dfloat-">dip::CreatePoissonPointProcess</a>.

% (c)2019, Cris Luengo.
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

function out = randomseeds(varargin)
out = dip_generation('randomseeds',varargin{:});
