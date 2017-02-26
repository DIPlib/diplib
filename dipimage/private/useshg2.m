% True if the version of MATLAB uses HG2 (handle graphics version 2) by default.
% The user can change the version of handle graphics used, but testing for it is finicky
% and not compatible across MATLAB versions. If the user chooses to change the HG version,
% bad things will happen.

% (c)2017, Cris Luengo.
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

function res = useshg2
persistent newmatlab;
if isempty(newmatlab)
   newmatlab = [100, 1] * sscanf(version, '%d.', 2) >= 804;
end
res = newmatlab;
