%EVALBOOL   Evaluate a value to a boolean
%   Accepted values are:
%    - scalar numbers, where 0 is FALSE and any non-zero is TRUE.
%    - a string, where 'n', 'no', 'f', 'false' and 'off' are FALSE,
%      and 'y', 'yes', 't', 'true' and 'on' are TRUE.

% (c)2017-2026, Cris Luengo.
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

function bool = evalbool(value)
value = string2char(value);
if ischar(value)
   switch lower(value)
      case {'y','yes','t','true','on'}
         bool = true;
      case {'n','no','f','false','off'}
         bool  = false;
      otherwise
         error('Boolean value expected.')
   end
elseif ( isnumeric(value) || islogical(value) ) && numel(value)==1
   bool = logical(value);
else
   error('Boolean value expected.')
end
