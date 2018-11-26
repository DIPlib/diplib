%DETILE   Splits an image in subimages
%  Revertes the function TILE.
%
% SYNOPSIS:
%  out = detile(in,array)
%
% PARAMETERS:
%  out:   Image with tensor of size ARRAY
%  array: Number of subimages into which IN is broken up per dimension.
%         array=N means Nx1 array. array=[N,M] means NxM array.
%
% DEFAULTS:
%  array = 2
%
% EXAMPLE:
%  a = readim;
%  g = gradient(a);
%  G = g*g.';
%  G1 = tile(G);
%  detile(G1,[2,2]) % identical to G
%
% SEE ALSO:
%  tile, arrangeslices

% (c)2017, Cris Luengo.
% Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
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

function out = detile(in,ts)
if nargin<2
   ts = 2;
end
if ~isa(in,'dip_image')
   in = dip_image(in);
end
if ~isscalar(in)
   error('Input image must be a scalar image');
end
% Figure out how many tensor components
if length(ts)==1
   ts = [ts,1];
elseif numel(ts)~=2
	error('Array to detile the image must have no more than two elements');
end
dd = imsize(in);
dd(1:2) = dd(1:2)./ts([2,1]);
if any(mod(dd,1))
   error('The array must divide the image size');
end
% Create OUT
out = clone(in,'imsize',dd,'tensorsize',ts,'tensorshape','column-major matrix');
stx = 0;
s = repmat({':'},ndims(in)-2,1);
for jj=1:ts(2)
    edx = stx+dd(1);
    sty = 0;
    for ii=1:ts(1)
        edy = sty+dd(2);
        out{ii,jj} = in(stx:edx-1,sty:edy-1,s{:});
        sty = edy;
    end
    stx = edx;
end
