% Grey-value color map with the middle intensity grey, negative
% intensities blue and positive intensities yellow.

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

function g = zerobased_colormap

gv = 0.4;
a = linspace(gv,1.0,128)'; % blue
b = linspace(gv,0.9,128)'; % yellow
c = linspace(gv,0.0,128)'; % black
g = [flipud([c,c,a]);b,b,c]; % blue-yellow

%g = [flipud([b,b,a]);a,b,b]; % blue-red
