%DIPFIG_TITLEBAR(FIG,UDATA)
%    Updates the title bar of FIG.

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

function dipfig_titlebar(fig,udata)
tit = udata.imname;
coords = imagedisplay(udata.handle,'coordinates');
nD = length(coords);
if nD>2
   k = imagedisplay(udata.handle,'orthogonal');
   tit = [tit,' (',num2str(coords(k(1)))];
   if nD>3
      tit = [tit,',',num2str(coords(k(2)))];
   end
   tit = [tit,')'];
end
if isfield(udata,'zoom') && ~isempty(udata.zoom) && udata.zoom~=0
   tit = [tit,' (',num2str(udata.zoom*100),'%)'];
end
set(fig,'Name',tit);
