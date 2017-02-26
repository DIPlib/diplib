%BORDER = DIPFIG_GETBORDERSIZE(FIGH)
%    Returns the size in pixels of the decorations around figure FIGH.
%    BORDER = [LEFT,BOTTOM,RIGTH,TOP] is the 4 measures of the borders
%    and menu bars and so on. These are the number of pixels used around
%    the area given by the figure's 'Position' property.

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

function border = dipfig_getbordersize(figh)
border = get(figh,'outerposition')-get(figh,'position');
border(1:2) = -border(1:2);
