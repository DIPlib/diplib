%P = CONVERTPATH(P)
%    Converts the string P into a cell array.

% (c)2017, Cris Luengo.
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

function out = convertpath(p)
out = {};
ii = 1;
while ~isempty(p)
   I = find(p==pathsep);
   if isempty(I)
      out{ii} = p;
      break;
   else
      out{ii} = p(1:I-1);
      p = p(I+1:end);
      ii = ii+1;
   end
end
% Remove trailing file separator to avoid problems in DIPimage GUI.
for ii=1:length(out)
   if out{ii}(end) == filesep
      out{ii}(end) = [];
   end
end
