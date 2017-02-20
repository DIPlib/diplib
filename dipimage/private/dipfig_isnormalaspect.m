%ZOOM = DIPFIG_ISNORMALASPECT(AXHANDLE)
%    Finds out if the aspect ratio is 1:1 for the image in AXHANDLE, and
%    returns the pixel size. If not, returns [].

% (c)1999-2014, Delft University of Technology
% (c)2017, Cris Luengo

function zoom = dipfig_isnormalaspect(axHandle)
zoom = [];
if ~isempty(findobj(axHandle,'type','image'))
   % 2D image
   imageWidth = diff(get(axHandle,'Xlim'));
   imageHeight = diff(get(axHandle,'Ylim'));
   axunits = get(axHandle,'Units');
   set(axHandle,'Units','pixels');
   pos = get(axHandle,'Position');
   set(axHandle,'Units',axunits);
   figureWidth = pos(3);
   figureHeight = pos(4);
   %if ((imageWidth*figureHeight)/(imageHeight*figureWidth)-1) < 0.001 % Allowed error
   if abs((imageWidth*figureHeight/imageHeight)-(figureWidth)) < 1 % Allowed error
      zoom = figureWidth/imageWidth;
   end
else
   % 1D image
   imageWidth = diff(get(axHandle,'Xlim'));
   axunits = get(axHandle,'Units');
   set(axHandle,'Units','pixels');
   pos = get(axHandle,'Position');
   set(axHandle,'Units',axunits);
   figureWidth = pos(3);
   zoom = figureWidth/imageWidth;
end
