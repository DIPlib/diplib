%H = GETFIGH(ARG)
%    Parses an input argument and returns a valid figure handle.
%    The argument can be the name of a variable or a figure handle.
%    The figure window must exist. If the argument is invalid, produces
%    an error.

% (c)1999-2014, Delft University of Technology
% (c)2017, Cris Luengo

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
