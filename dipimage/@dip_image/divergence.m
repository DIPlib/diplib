%DIVERGENCE  Calculates the divergence of a vector image.
%   DIVERGENCE(A,SIGMA) returns the divergence (scalar) of a vector image.
%   Identical to DIVERGENCEVECTOR, see that function's help for more details
%   on additional parameters.
%
%   See also DIVERGENCEVECTOR.

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

function out = divergence(varargin)
out = divergencevector(varargin{:});
