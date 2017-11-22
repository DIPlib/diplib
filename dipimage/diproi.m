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

function [varargout] = diproi(fig,intertype)
if nargin < 2
   intertype = 'polygon';
end
if nargin == 1 && ischar(fig)
   intertype = fig;
   fig = gcf;
elseif nargin < 1
   fig = gcf;
end

coords = dipdrawpolygon(fig);
if size(coords,1)<3
   error('You need to select at least three vertices.')
end

if strcmp(intertype, 'spline')
   coords(end+1,:)=coords(1,:); % TODO: splines need periodic boundary conditions to look good
   n=length(coords);
   xs = spline(1:n,coords(:,1),1:.2:n);
   ys = spline(1:n,coords(:,2),1:.2:n);
   coords = [xs' ys'];
   coords(end,:) = [];
   coords = floor(coords);
elseif ~strcmp(intertype, 'polygon')
   error('Unkown interpolation type.');
end
udata = get(fig,'userdata');
mask = newim(udata.imsize,'bin');
varargout{1} = drawpolygon(mask,coords,1,'filled');
if nargout==2
   varargout{2} = coords;
end
