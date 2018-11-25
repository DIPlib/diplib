%INTEGRAL_IMAGE   Compute the integral image.
%
% SYNOPSIS:
%  image_out = integral_image(image_in,mask,process)
%
% PARAMETERS:
%  mask:    Any pixels not selected by the mask are presumed to be zero.
%  process: Array specifying over which dimensions the integral is computed.
%
% DEFAULTS:
%  mask = []
%  process = [] (all dimensions are processed)
%
% EXAMPLE:
%  img = readim;
%  intim = integral_image(img);
%  x = 10; y = 63;
%  w = 92; h = 8;
%  s1 = sum(img(x+(0:w-1),y+(0:h-1)))
%  s2 = intim(x+w-1,y+h-1) + intim(x-1,y-1) - intim(x+w-1,y-1) - intim(x-1,y+h-1)
%
% DIPlib:
%  This function calls the DIPlib function dip::CumulativeSum.

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

function out = integral_image(varargin)
out = dip_math('integral_image',varargin{:});
