%B = ISFIGH(H)
%    Returns true if H is the handle of a figure window.

% (c)1999-2014, Delft University of Technology
% (c)2017, Cris Luengo

function b = isfigh(h)
b = 0;
if length(h)==1 & ishandle(h)
   if strcmp(get(h,'type'),'figure')
      b = 1;
   end
end
