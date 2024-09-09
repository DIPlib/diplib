%CELL2IM   Converts a cell array of images to a dip_image
%
% SYNOPSIS:
%  A = CELL2IM(B)
%  A = CELL2IM(B,COLSPACE)
%
%  Converts the cell array B to a dip_image. Each cell of B is considered a
%  tensor component, and must contain a scalar image (or a matrix) of the same
%  size. A will have SIZE(B) for tensor sizes, and be a full column-major
%  matrix.
%
%  If COLSPACE is given, will assign the given color space.
%
% EXAMPLES:
%  a = readim('flamingo');
%  b = im2cell(a);
%  c = cell2im(b,'srgb');  % c is identical to a.
%
%  a = cell(2,2);
%  a{1,1} = readim('trui');
%  a{1,2} = gaussf(a{1,1});
%  a{2,1} = a{1,2};
%  a{2,2} = medif(a{1,1});
%  b = cell2im(a);         % b is a 2x2 tensor image
%
% SEE ALSO:
%  im2cell, mat2im, joinchannels, dip_image.dip_image

% (c)2018, Cris Luengo.
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

function in = cell2im(in,colspace)
if nargin>1
   in = joinchannels(colspace,in);
else
   in = dip_image(in);
end
