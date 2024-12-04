%BCLOSING   Binary closing
%
% SYNOPSIS:
%  image_out = bclosing(image_in,iterations,connectivity,edgeCondition)
%  image_out = bclosing(image_in,distance,'isotropic')
%
% PARAMETERS:
%  iterations: the number of steps taken, defines the size of the
%     structuring element.
%  distance: radius of isotropic structuring element, can be non-integer.
%  connectivity: defines the neighborhood:
%     * 1 indicates 4-connected neighbors in 2D or 6-connected in 3D
%     * 2 indicates 8-connected neighbors in 2D
%     * 3 indicates 28-connected neighbors in 3D
%     * -1 and -2 indicate alternating values leading to a more isotropic
%       operation
%  edgeCondition: the value of pixels outside the image bounds,
%      can be 'background' or 'object', or equivalently 0 or 1; or
%      'special', equivalent to using 'background' for the dilation
%      step and 'object' for the dilation step.
%
% DEFAULTS:
%  iterations = 1
%  connectivity = -1
%  edgeCondition = 'special'
%
% DIPlib:
%  This function calls the DIPlib function <a href="https://diplib.org/diplib-docs/binary.html#dip-BinaryClosing-Image-CL-Image-L-dip-sint--dip-uint--String-CL">dip::BinaryClosing</a> and
%  <a href="https://diplib.org/diplib-docs/binary.html#dip-IsotropicClosing-Image-CL-Image-L-dfloat-">dip::IsotropicClosing</a>.

% (c)2017-2022, Cris Luengo.
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

function out = bclosing(varargin)
out = dip_morphology('bclosing',varargin{:});
