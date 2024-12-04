%SUPERPIXELS   Generates superpixels (oversegmentation)
%
% SYNOPSIS:
%  image_out = superpixels(image_in,density,compactness,method,flags)
%
% PARAMETERS:
%  density: determines the number of superpixels (average size is 1/density).
%  compactness: determines compactness of superpixels.
%  method: currently only 'CW' is supported (compact watershed superpixels).
%  flags: a cell array of strings, choose from:
%     - 'rectangular' or 'hexagonal': the shape of the superpixels. By default
%       they are rectangular.
%     - 'no gaps': superpixels cover the full image. By default, a 1-pixel
%       gap is left between superpixels.
%
% DEFAULTS:
%  density = 0.005
%  compactness = 1
%  method = 'CW'
%  flags = {}
%
% DIPlib:
%  This function calls the DIPlib function <a href="https://diplib.org/diplib-docs/segmentation.html#dip-Superpixels-Image-CL-Image-L-Random-L-dfloat--dfloat--String-CL-StringSet-CL">dip::Superpixels</a>.

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

function out = superpixels(varargin)
out = dip_segmentation('superpixels',varargin{:});
