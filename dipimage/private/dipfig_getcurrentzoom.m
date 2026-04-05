%ZOOM = DIPFIG_GETCURRENTZOOM(AXHANDLE)
%    Returns the pixel size as a two-element vector [x,y].

% (c)2017-2026, Cris Luengo.
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

function zoom = dipfig_getcurrentzoom(axHandle)
if isempty(findobj(axHandle,'type','image'))
   error('Figure is not an image display')
end
imageWidth = diff(get(axHandle,'Xlim'));
imageHeight = diff(get(axHandle,'Ylim'));
axunits = get(axHandle,'Units');
set(axHandle,'Units','pixels');
pos = get(axHandle,'Position');
set(axHandle,'Units',axunits);
figureWidth = pos(3);
figureHeight = pos(4);
zoom = [figureWidth/imageWidth, figureHeight/imageHeight];
