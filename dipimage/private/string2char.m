%B = STRING2CHAR(A)
%    If A is a STRING type (since MATLAB R2016b), convert it to a char vector
%    (scalar string) or a cell array of char vectors (array of strings).
%    Otherwise, leave the value unchanged.
%
%    Serves the same purpose as convertContainedStringsToChars() (since R2018b),
%    but works in any version of MATLAB.

% (c)2026, Cris Luengo.
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

function value = string2char(value)
if ~has_string
   return
end
if isstring(value)
   if numel(value) == 1
      value = char(value);
   else
      out = cell(size(value));
      for ii = 1:numel(value)
         out{ii} = char(value(ii));
      end
   end
end

function res = has_string
persistent newmatlab;
if isempty(newmatlab)
   newmatlab = [100, 1] * sscanf(version, '%d.', 2) >= 901;
end
res = newmatlab;
