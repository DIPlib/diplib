%PT = DIPFIG_GETCURPOS(AX)
%    Returns the coordinates of the mouse over the image in axes AX.
%    PT is [x,y], so that indexing into the dip_image object is done
%    by (pt(1),pt(2)).
%
%    If the mouse is outside the window, returns the closest coordinates
%    within the window.
%
%    The axes 'units' are expected to be 'pixels'.
%
%    PT = DIPFIG_GETCURPOS(AX,CROP) is the same thing if CROP is non-zero.
%    For CROP==0, the coordinates are not compared to the axis limits, so
%    the returned value can be outside the window, and outside the image.

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

function pt = dipfig_getcurpos(ax,crop)
pt = get(ax,'CurrentPoint');
pt = [pt(1,1),pt(1,2)];
pt = round(pt);
if nargin==1 | crop
   curxlim = get(ax,'XLim');
   if pt(1) < curxlim(1)
      pt(1) = ceil(curxlim(1));
   elseif pt(1) > curxlim(2)
      pt(1) = floor(curxlim(2));
   end
   curylim = get(ax,'YLim');
   if pt(2) < curylim(1)
      pt(2) = ceil(curylim(1));
   elseif pt(2) > curylim(2)
      pt(2) = floor(curylim(2));
   end
end
