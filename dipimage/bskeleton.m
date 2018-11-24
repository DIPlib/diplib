%BSKELETON   Binary skeleton
%
% SYNOPSIS:
%  image_out = bskeleton(image_in,edgeCondition,endPixelCondition)
%
% PARAMETERS:
%  edgeCondition: the value of pixels outside the image bounds,
%      can be 'background' or 'object', or equivalently 0 or 1.
%  endPixelCondition: 'loose ends away', 'natural', 'one neighbor'
%      'two neighbors', 'three neighbors'
%
% DEFAULTS:
%  edgeCondition = 'background'
%  endPixelCondition = 'natural'
%
% NOTE:
%  endPixelCondition knows the aliases 'looseendsaway', '1neighbor',
%  '2neighbors' and '3neighbors' for backwards compatibility.
%
% WARNINGS
%  The algorithm uses a 2-pixel thick border at the edges of the image. This
%  affects results on objects within this area. EXTEND the image with a dummy
%  border if this is a problem for you.
%
%  This function is only defined for 2D and 3D images.
%
%  The 3D version of this algorithm is buggy, 'looseendsaway', '1neighbor' and
%  '3neighbors' all produce the same result. Wrong skeletons are returned by
%  the different modes under various circumstances.
%
% DIPlib:
%  This function calls the DIPlib function dip::EuclideanSkeleton

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

function out = bskeleton(varargin)
out = dip_morphology('bskeleton',varargin{:});
