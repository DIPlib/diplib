%CIRCSHIFT   Shift image circularly.
%   B = CIRCSHIFT(A,N) shifts the image by N(D) in each dimension D.
%   N must be an array with integer values, and contain one value
%   per image dimension. Positive shifts are to the right or down,
%   negative shifts the other way. The image is assumed to have
%   periodic boundary conditions.
%
%   See also SHIFT, RESAMPLE

% (c)2017, Cris Luengo.
% Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
% Based on original DIPimage code: (c)2010-2011, Cris Luengo
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
