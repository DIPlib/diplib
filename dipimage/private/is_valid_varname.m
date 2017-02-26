%IS_VALID_VARNAME(NAME)
%    Check variable name for validity.

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

function v = is_valid_varname(name)

start = '_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz';
other = [start,'0123456789'];

v = 0;
if ~ischar(name), return, end
if length(name)<1, return, end
if ~any(start==name(1)), return, end
if ~all(ismember(name(2:end),other)), return, end
v = 1;
