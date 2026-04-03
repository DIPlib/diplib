%NNZ   Count all non-zero pixels in an image.
%   VALUE = NNZ(B) counts the number of pixels with a value different from zero.
%   The image must be scalar.
%
%   VALUE = NNZ(B,M) does the same but only within the mask image M (binary),
%   and is equivalent to NNZ(B(M)).
%
%  This function calls the DIPlib function <a href="https://diplib.org/diplib-docs/math_statistics.html#dip-Count-Image-CL-Image-CL">dip::Count</a>.

% (c)2026, Cris Luengo.
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

function out = nnz(varargin)
out = dip_imagemath('nnz',varargin{:});
