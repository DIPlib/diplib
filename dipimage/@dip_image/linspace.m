% LINSPACE Linearly spaced 1D image.
%   LINSPACE(X1, X2, N) generates a 1D omage of N pixels,
%   the first pixel's value is X1, the last pixel's value
%   is X2, and the remainder are interpolated linearly in
%   between. The output image data type is always single
%   float.
%
%   N defaults to 256.

% (c)2023, Cris Luengo.
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

function y = linspace(d1, d2, n)
if nargin < 2
   error('At least two input arguments required')
elseif nargin == 2
   n = 256;
else
   if ~isnumeric(n) || ~isscalar(n)
      error('N must be a scalar integer value')
   end
end
y = dip_image(linspace(single(d1), single(d2), n));
