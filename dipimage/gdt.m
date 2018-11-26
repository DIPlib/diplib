%GDT   Grey-weighted distance transform
%
% SYNOPSIS:
%  [out, distance] = gdt(image_in,image_weight,chamfer)
%
% PARAMETERS:
%  chamfer:  Chamfer distance of 1, 3 or 5.
%            1: Only the 6 (8) direct neighbors in a 3x3 (3x3x3) neighbourhood are used.
%            3: All neighbors in in a 3x3 (3x3x3) neighbouhood are used.
%            5: A neighborhood of (5x5) or (5x5x5) is used.
%  out:      Integrated grey-value over least-resistance path from each foreground pixel
%            in IMAGE_IN to the nearest background pixel.
%  distance: Metric distance over least-resistance path.
%
% DEFAULTS:
%  chamfer = 3
%
% DIPlib:
%  This function calls the DIPlib function dip::GreyWeightedDistanceTransform

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

function varargout = gdt(varargin)
varargout = cell(1,max(nargout,1));
[varargout{:}] = dip_analysis('gdt',varargin{:});
