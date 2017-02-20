%BORDER = DIPFIG_GETBORDERSIZE(FIGH)
%    Returns the size in pixels of the decorations around figure FIGH.
%    BORDER = [LEFT,BOTTOM,RIGTH,TOP] is the 4 measures of the borders
%    and menu bars and so on. These are the number of pixels used around
%    the area given by the figure's 'Position' property.

% (c)1999-2014, Delft University of Technology
% (c)2017, Cris Luengo

function border = dipfig_getbordersize(figh)
border = get(figh,'outerposition')-get(figh,'position');
border(1:2) = -border(1:2);
