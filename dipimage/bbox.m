%BBOX   Bounding box of an n-D binary image
%
% SYNOPSIS:
%  [out,bb,outg] = bbox(mask,grey)
%
% PARAMETERS:
%  mask: binary mask image
%  grey: optional, another image to crop to the new size
%
% OUTPUTS:
%  out:  the image MASK cut to the bounding box
%  bb:   the bounding box coordinates
%  outg: the optional image GREY croped to the bounding box
%
% EXAMPLE:
%  a = readim('nuts_bolts2');
%  b = label(a<200);
%  [c,~,d] = bbox(b==3,a)

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

function [out,bb,outg] = bbox(in,ingrey)
if ~isa(in,'dip_image')
   in = dip_image(in);
end
if ~islogical(in)
   error('Input must be binary.');
end
if nargin>1
   if ~isa(ingrey,'dip_image')
      ingrey = dip_image(ingrey);
   end
   if ~isequal(imsize(in),imsize(ingrey))
      error('Images not the same size');
   end
end
msr = measure(+in,[],{'Minimum','Maximum'});
N = ndims(in);
s = cell(N,1);
bb = zeros(2,2);
for ii = 1:N
   bb(ii,:) = [msr.Minimum(ii),msr.Maximum(ii)];
   s{ii} = msr.Minimum(ii):msr.Maximum(ii);
end
out = in(s{:});
if nargin>1 && nargout>2
   outg = ingrey(s{:});
end
