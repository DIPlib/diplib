%IM2ARRAY   Convert an image stack to a vector image
%
% SYNOPSIS:
%  out = im2array(in)
%
%  The last spatial dimension of IN is converted to the tensor dimension.
%  IN must be a scalar image. OUT will have one fewer dimensions than IN.
%  Data is copied.
%
% EXAMPLE:
%  a = readim
%  g = gradient(a)
%  b = array2im(g)
%  h = im2array(b) % h is identical to g
%
% NOTE:
%  SPATIALTOTENSOR does the same thing, but the first spatial dimension
%  will converted to the tensor dimension, such that data does not need to
%  be copied.
%
% SEE ALSO:
%  array2im, dip_image.spatialtotensor, dip_image.tensortospatial

% (c)2017, Cris Luengo.
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

function in = im2array(in)
if ~isa(in,'dip_image')
   in = dip_image(in);
end
if isempty(in) || ndims(in) == 0
   error('Input image does not have enough dimensions')
end
in = spatialtotensor(in,ndims(in));
