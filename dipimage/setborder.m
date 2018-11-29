%SETBORDER   Sets the pixels at the border of the image to a constant value
%
% SYNOPSIS:
%  image_out = setborder(image_in,value,size)
%
% PARAMETERS:
%  value: the value to write, or an array of values if IMAGE_IN is a tensor image.
%  size:  the width of the border in pixels.
%
% DEFAULTS:
%  value = 0
%  size = 1
%
% DIPlib:
%  This function calls the DIPlib function dip::SetBorder.

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

function out = setborder(varargin)
out = dip_generation('setborder',varargin{:});
