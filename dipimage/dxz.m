%DXZ   Second derivative in the XZ-direction
%
% SYNOPSIS:
%  image_out = dxz(image_in,sigma)
%
% DEFAULTS:
%  sigma = 1
%
% SEE ALSO:
%  derivative

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

function image_out = dxz(image_in,sigma)
if nargin < 2, sigma = 1; end
order = zeros(1,ndims(image_in));
order(1) = 1;
if length(order)>=3
   order(3) = 1;
end
image_out = filtering('derivative',image_in,order,sigma);
