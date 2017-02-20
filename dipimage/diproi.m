%DIPROI   Interactive ROI selection
%   Returns the mask image it allows the user to create over an existing
%   image display. It is formed by a single polygon or spline, created by
%   clicking on the image.
%
% SYNOPSIS:
%   [roi, v] = diproi(figure_handle, interpolation)
%   [roi, v] = diproi(interpolation)
%
% PARAMETERS:
%   figure_handle: 
%   interpolation: 'polygon','spline'
%
% RETURNS
%   v: vertices of polygon or spline
%
% DEFAULTS:
%   figure_handle: current window (GCF).
%   interpolation: 'polygon'
%
%   DIPROI is only available for 2D figure windows.
%
%   To create the polygon, use the left mouse button to add vertices.
%   A double-click adds a last vertex and closes the shape. 'Enter'
%   closes the shape without adding a vertex. To remove vertices, use
%   the 'Backspace' or 'Delete' keys, or the right mouse button. 'Esc'
%   aborts the operation. Shift-click will add a vertex constrained to
%   a horizontal or vertical location with respect to the previous vertex.
%
%   Note that you need to select at least three vertices. If you
%   don't, an error will be generated.
%
%   It is still possible to use all the menus in the victim figure
%   window, but you won't be able to access any of the tools (like
%   zooming and testing). The regular key-binding is also disabled.
%
%   Note: If you feel the need to interrupt this function with Ctrl-C,
%   you will need to refresh the display (by re-displaying the image
%   or changing the 'Actions' state).
%
%   See also DIPSHOW, DIPGETCOORDS, DIPCROP, DIPPROFILE.

% (c)1999-2014, Delft University of Technology
% (c)2017, Cris Luengo

function [varargout] = diproi(fig,intertype)

if nargin == 1 && ischar(fig)
    intertype = fig;
    fig = [];
end

coords = dipdrawpolygon(fig);

if size(coords,1)<3
   error('You need to select at least three vertices.')
end

udata = get(fig,'userdata');
mask = newim(udata.imsize,'uint8');
switch intertype
   case 'polygon'
      mask = drawpolygon(mask,coords,1,'closed');
      mask = dip_image(mask,'bin');
      mask = ~bpropagation(mask&0,~mask,0,1,1);
      varargout{1} = mask;
      if nargout==2
        varargout{2} = coords; 
      end
   case 'spline'
      coords(end+1,:)=coords(1,:);
      n=length(coords);
      xs = spline(1:n,coords(:,1),1:.2:n);
      ys = spline(1:n,coords(:,2),1:.2:n);
      co = [xs' ys'];
      co(end,:) = [];
      co = floor(co);
      m2 = drawpolygon(mask,co,1,'closed');
      m2 = dip_image(m2,'bin');
      m2 = ~bpropagation(m2&0,~m2,0,1,1);
      varargout{1} = m2;
      if nargout==2
        varargout{2} = co; 
      end
   otherwise
      error('Unkown interpolation type.');
end
