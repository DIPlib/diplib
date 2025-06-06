%HMINIMA   H-minima transform
%
% SYNOPSIS:
%  image_out = hminima(image_in,h,connectivity)
%
% PARAMETERS:
%  h:            the algorithm removes minima with a depth `h` or less.
%  connectivity: defines the metric, that is, the shape of the structuring
%     element.
%     * 1 indicates city-block metric, or a diamond-shaped S.E in 2D.
%     * 2 indicates chessboard metric, or a square structuring element in 2D.
%     For 3D images use 1, 2 or 3.
%
% DEFAULTS:
%  connectivity = 1
%
% NOTE:
%  See the user guide for the definition of connectivity in DIPimage.
%
% DIPlib:
%  This function calls the DIPlib function <a href="https://diplib.org/diplib-docs/morphology.html#dip-HMinima-Image-CL-Image-L-dfloat--dip-uint-">dip::HMinima</a>.

% (c)2017-2018, Cris Luengo.
% Based on original DIPimage code: (c)2013, Cris Luengo.
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

function out = hminima(varargin)
out = dip_morphology('hminima',varargin{:});
