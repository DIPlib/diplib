%ENTROPY   Computes the entropy (in bits) of an image
%
% SYNOPSIS:
%  image_out = entropy(image_in, nBin)
%
% DEFAULT:
%  nBin = 256;
%
% DIPlib:
%  This function calls the DIPlib function <a href="https://diplib.org/diplib-docs/math_error.html#dip-Entropy-Image-CL-Image-CL-dip-uint-">dip::Entropy</a>.

% (c)2017-2018, Cris Luengo.
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

function out = entropy(varargin)
out = dip_math('entropy',varargin{:});
