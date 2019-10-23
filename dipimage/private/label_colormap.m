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
% Data same as in /src/display/colormap.cpp
col = [
             255, 0,   0
             0,   255, 0     % green
             0,   0,   255
             255, 255, 0
             0,   255, 255
             255, 0,   255
             255, 182, 0
            %0,   255, 182   % greenish A
             0,   255, 143   % a different greenish color in between A and C
             182, 0,   255
             255, 0,   182
            %182, 255, 0     % greenish B
             143, 255, 0     % a different greenish color in between B and D
             0,   182, 255
             255, 102, 0
            %0,   255, 102   % greenish C
             102, 0,   255
             255, 0,   102
            %102, 255, 0     % greenish D
             0,   102, 255
             182, 182, 0
             0,   182, 182
             182, 0,   182
] / 255;
n = size(col,1);
g = [0,0,0;col(mod((2:m)-2,n)+1,:)];
