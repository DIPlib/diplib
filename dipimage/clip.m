%CLIP   Grey-value clipping (or clamping)
%
% SYNOPSIS:
%  image_out = clip(image_in,minimum,maximum)
%
% PARAMETERS:
%  minimum: lower bound
%  maximum: upper bound
%
% DEFAULTS:
%  minimum = 0
%  maximum = 255
%
% NOTES:
%  Maximum and minimum may be reversed.
%
%  Set Maximum to Inf or Minimum to -Inf to do a one-sided clip.
%
% DIPlib:
%  This function calls the DIPlib function dip::Clip.

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

function out = clip(varargin)
out = dip_math('clip',varargin{:});
