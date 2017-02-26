%DIPFIG_SET_ACTION_CHECK(FIG,STATE)
%    Set the checkmark to the current mouse action.

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

function dipfig_set_action_check(fig,state)
m = get(findobj(get(fig,'Children'),'Tag','actions'),'Children');
tags = get(m,'Tag');
I = strncmp(tags,'mouse_',6);
m = m(I);
set(m,'Checked','off');
if ~isempty(state)
   I = find(strcmp(tags(I),['mouse_',state]));
   if ~isempty(I)
      set(m(I),'Checked','on');
   end
end
