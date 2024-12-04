%RNGSEED   Seeds DIPimage's random number generator
%
% SYNOPSIS:
%  rngseed
%  rngseed(seed)
%
%  SEED is a positive integer. Negative values will have the sign removed,
%  and non-integer values will be floored. Integer overflow is undefined.
%
%  If no SEED is given, the random number generator will be seeded with a
%  random value obtained from the system (this process is system-dependent).
%  The random number generator is initialized that way the first time it is
%  used, so in general it is not necessary to use this syntax.
%
% DIPlib:
%  This function calls the DIPlib function <a href="https://diplib.org/diplib-docs/dip-Random.html#dip-Random-Seed">dip::Random::Seed</a>.

% (c)2019, Cris Luengo.
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

function rngseed(varargin)
dip_generation('rngseed',varargin{:});
