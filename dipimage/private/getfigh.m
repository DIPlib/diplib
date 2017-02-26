%H = GETFIGH(ARG)
%    Parses an input argument and returns a valid figure handle.
%    The argument can be the name of a variable or a figure handle.
%    The figure window must exist. If the argument is invalid, produces
%    an error.

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

function h = getfigh(arg)
if ischar(arg)
   if strcmpi(arg,'other')
      error('String ''other'' no allowed as window handle');
   else
      h = dipfig('-get',arg);
      if h == 0
         error('Variable name not linked to figure window');
      end
   end
else
   h = arg;
end
if ~isfigh(h)
   error('Figure handle expected');
end
