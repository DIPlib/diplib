%ARRAY2IM   Convert a tensor image to an image stack
%
% SYNOPSIS:
%  out = array2im(in)
%
%  The tensor dimension of IN is converted to a new spatial
%  dimension, which will be last. That is, OUT will have one
%  more dimension than IN, and be scalar.
%
% EXAMPLE:
%  a = readim
%  g = gradient(a)
%  b = array2im(g)
%  h = im2array(b) % h is identical to g
%
% SEE ALSO:
%  im2array, dip_image.tensortospatial, dip_image.spatialtotensor

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

function in = array2im(in)
if ~isa(in,'dip_image')
   error('Input is not a tensor image');
end
in = tensortospatial(in,ndims(in)+1);
