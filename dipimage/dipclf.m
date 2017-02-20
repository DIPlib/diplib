%DIPCLF   Clears figure windows created by DIPSHOW
%   DIPCLF clears all figure windows created by DIPSHOW, without
%   changing their position or size.
%
%   DIPCLF(H) clears the figure window with handle H. If H is zero,
%   clears all figure windows. H can be a numeric array [H1,H2,H3]
%   or a cell array {H1,H2,'varname1','varname2'} or just a variable
%   name. The window associated with that name is cleared.

% (C) Copyright 1999-2014               Pattern Recognition Group
%     All rights reserved               Faculty of Applied Physics
%                                       Delft University of Technology
%                                       Lorentzweg 1
%                                       2628 CJ Delft
%                                       The Netherlands
%
% Cris Luengo, October 2000.
% 9 November 2000:   We now display an empty image into the figure window.
% 5 April 2001:      Using new, undocumented DIPSHOW option.
% 20 August 2001:    Added optional argument to clear only one window.
% 16-26 August 2001: Changed DIPSHOW. This function changes accordingly.
% 8 March 2002:      Added 'var' and {'var',h'} syntaxes.
% 11 August 2014:    Fix for new graphics in MATLAB 8.4 (R2014b).

function d = dipclf(h)

% Avoid being in menu
if nargin == 1 & ischar(h) & strcmp(h,'DIP_GetParamList')
   d = struct('menu','Display',...
              'display','Clear all image windows');
   return
end

if nargin < 1 | isequal(h,0)
   h = findobj('Type','figure');
else
   if isempty(h)
      return;
   end
   if ~iscell(h)
      if isnumeric(h) | ishandle(h)
         h = num2cell(h);
      elseif ischar(h)
         h = {h};
      else
         error('Array with figure handles expected')
      end
   end
   I = logical(zeros(size(h)));
   for ii=1:prod(size(I))
      if ischar(h{ii})
         h{ii} = dipfig('-get',h{ii});
      end
      if isfigh(h{ii})
         I(ii) = 1;
      end
   end
   h = [h{I}];
end
h = h(strncmp(get(h,'Tag'),'DIP_Image',9)); % They should be created using DIPshow.
for ii=1:length(h)
   dipshow(h(ii),[],'name','');
end
