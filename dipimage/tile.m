%TILE   Tiles tensor components
%  Tiles tensor component images along the first two image dimensions.
%
% SYNOPSIS:
%  out = tile(in,stretch)
%
% PARAMETERS:
%  stretch: should the individual images be stretched to [0,255] before tiling?
%
% DEFAULTS:
%  stretch = false
%
% EXAMPLE:
%  a = readim;
%  g = gradient(a);
%  G = g*g.';
%  tile(G)
%
% SEE ALSO:
%   arrangeslices, detile

% (c)2017, Cris Luengo.
% Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
% Originally written by Bernd Rieger.
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

function out = tile(in,st)
if nargin<2
   st = false;
elseif ischar(st)
   st = strcmp(st,{y,'yes'});
end
% Stretch
if st
   in = iterate(@stretch,in);
end
% Copy tensor components into a cell array
sz = tensorsize(in);
tmp = cell(sz(1),1);
for ii=1:sz(1)
   % Catenate vertically the tensor components
   try
      tmp{ii} = in{ii,1};
   catch
      tmp{ii} = clone(in,'tensorsize',1);
   end
   for jj=2:sz(2)
      try
         tmp2 = in{ii,jj};
      catch
         tmp2 = clone(in,'tensorsize',1);
      end
      tmp{ii} = cat(1,tmp{ii},tmp2);
   end
end
% Cat cell array into a single image
out = cat(2,tmp{:});
