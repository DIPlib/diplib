%DIPFIG_SET_ACTION_CHECK(FIG,STATE)
%    Set the checkmark to the current mouse action.

% (c)1999-2014, Delft University of Technology
% (c)2017, Cris Luengo

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
