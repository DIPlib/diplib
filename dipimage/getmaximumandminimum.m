%GETMAXIMUMANDMINIMUM   Find the minimum and maximum sample value
%
% SYNOPSIS:
%  out = getmaximumandminimum(image_in,mask)
%
%  This is equivalent to
%     out = [min(image_in,mask),max(image_in,mask)]
%  except it's faster.
%
% DEFAULTS:
%  mask = [] (all pixels are examined)
%
% DIPlib:
%  This function calls the DIPlib function dip::MaximumAndMinimum.

% (c)2017-2018, Cris Luengo.
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

function out = getmaximumandminimum(varargin)
out = dip_math('getmaximumandminimum',varargin{:});
