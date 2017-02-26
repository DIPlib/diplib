%NICELUT   Nice Look-Up Table
%   NICELUT(N) is a color look-up table that looks pretty nice.
%   N defaults to 256. To use it do COLORMAP(NICELUT).
%
%   It is possible to specify the distance between the colors
%   through a second argument: NICELUT(N,W), where W has 12
%   elements, one for each distance between: red, orange, yellow,
%   yellowgreen, green, greencyan, cyan, cyanblue, blue,
%   bluemagenta, magenta, magentared and red.
%
%   NICELUT goes from red to red, with reduced saturation in blue.

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

function out = niceLUT(n,w)

if nargin < 2
   %  red orange yellow yellowgreen green greencyan cyan
   w = [ 8,    12,    10,          5,    5,       10,...
         10,       6,   6,         10,     10,         8    ];
   %  cyan cyanblue blue bluemagenta magenta magentared red
end
if nargin < 1
   n = 256;
end

if length(w) ~= 12
   error('Illegal w array.')
end
if length(n) ~= 1
   n = n(1);
end

% normalize widths so that their sum = n
w = floor(w.*n./sum(w));
% cumulate w's
i = zeros(1,12);
i(2:12) = cumsum(w(1:11));
w(12) = n-i(12); % absorbs rounding errors
i = i+1; % MATLAB coordinates.
j = i+w-1; % Ending coordinates.

red = zeros(n,1);
green = zeros(n,1);
blue = zeros(n,1);

% R->Y
red(i(1):j(1)) = 255;
green(i(1):j(1)) = linspace(0,127,w(1));
%blue(i(1):j(1)) = 0;

red(i(2):j(2)) = 255;
green(i(2):j(2)) = linspace(128,255,w(2));
%blue(i(2):j(2)) = 0;

% Y->G
red(i(3):j(3)) = linspace(255,128,w(3));
green(i(3):j(3)) = 255;
%blue(i(3):j(3)) = 0;

red(i(4):j(4)) = linspace(127,0,w(4));
green(i(4):j(4)) = 255;
%blue(i(4):j(4)) = 0;

% G->C
%red(i(5):j(5)) = 0;
green(i(5):j(5)) = 255;
blue(i(5):j(5)) = linspace(0,127,w(5));

%red(i(6):j(6)) = 0;
green(i(6):j(6)) = 255;
blue(i(6):j(6)) = linspace(128,255,w(6));

% C->B
%red(i(7):j(7)) = 0;
green(i(7):j(7)) = linspace(255,128,w(7));
blue(i(7):j(7)) = 255;

sat = linspace(1,0.8,w(8));
whi = 255*(1-sat);
red(i(8):j(8)) = whi;
green(i(8):j(8)) = sat.*linspace(127,0,w(8)) + whi;
blue(i(8):j(8)) = sat*255 + whi;

% B->M
sat = linspace(0.8,1,w(9));
whi = 255*(1-sat);
red(i(9):j(9)) = sat.*linspace(0,127,w(9)) + whi;
green(i(9):j(9)) = whi;
blue(i(9):j(9)) = sat*255 + whi;

red(i(10):j(10)) = linspace(128,255,w(10));
%green(i(10):j(10)) = 0;
blue(i(10):j(10)) = 255;

% M->R
red(i(11):j(11)) = 255;
%green(i(11):j(11)) = 0;
blue(i(11):j(11)) = linspace(255,128,w(11));

red(i(12):j(12)) = 255;
%green(i(12):j(12)) = 0;
blue(i(12):j(12)) = linspace(127,0,w(12));

% Normalize percieved intensity
%ss = sqrt(4*red.*red + 9*green.*green + blue.*blue);  % Piet's normalization
ss = sqrt(4*red.*red + 8*green.*green + 2*blue.*blue); % Cris' normalization
ss = (1 + 16*sqrt(ss)) ./ (1 + ss);
red = red.*ss;
green = green.*ss;
blue = blue.*ss;

% Create output colormap
out = [red,green,blue];
out = out./max(out(:));
