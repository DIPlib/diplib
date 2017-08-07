% Labeled image color map.
% The first item is [0,0,0] for background.
% Label 1 gets red, matches what happens with binary images.
% The first 6 labels are the purest colors, to maximize
% differences when there's only a few labels.

% (c)2017, Cris Luengo.
% (c)1999-2014, Delft University of Technology.
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

function g = label_colormap(m)
if nargin<1
   m = 256; % length of color map
end
% Greens are closer together perceptually, so we reduce the
% number of green entries in this map.
col = [
    1.0000, 0.0000, 0.0000
    0.0000, 1.0000, 0.0000
    0.0000, 0.0000, 1.0000
    1.0000, 1.0000, 0.0000
    0.0000, 1.0000, 1.0000
    1.0000, 0.0000, 1.0000
    1.0000, 0.3333, 0.0000
    0.6667, 1.0000, 0.0000
    0.0000, 0.6667, 1.0000
    0.3333, 0.0000, 1.0000
    1.0000, 0.0000, 0.6667
    1.0000, 0.6667, 0.0000
    0.0000, 1.0000, 0.5000
    0.0000, 0.3333, 1.0000
    0.6667, 0.0000, 1.0000
    1.0000, 0.0000, 0.3333
];
n = size(col,1);
g = [0,0,0;col(mod((2:m)-2,n)+1,:)];
