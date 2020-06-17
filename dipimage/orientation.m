%ORIENTATION   Local structure orientation
% Estimates the orientation in nD images for line-like or surface-like
% structures. Uses the structure tensor to obtain a robust estimate of
% the local structure. For the case of 'surface', this function returns
% the robust equivalent of the gradient, up to mirroring (it is the
% orientation of the structure, not the direction of the normal).
%
% SYNOPSIS:
%  out = orientation(in,method,sg,st,subsample)
%
%  method:    'line', 'surface'; see below
%  sg:        sigma of the gradient derivative
%  st:        sigma of the tensor smoothing (for 'line' and 'surface' only)
%  subsample: a boolean value, the output will be subsampled by a factor
%             FLOOR(ST) if TRUE, but only for 'line' and 'surface' methods,
%             where ST is used for smoothing. Useful for very large images.
%
%  out:       unit vector image along the local structure ('line') or
%             perpendicular to it ('surface'). Thus, in the case of 'surface',
%             it is the surface normal (up to mirroring).
%
% DEFAULTS:
%  method = 'surface'
%  sg = 1
%  st = 4
%  subsample = false (up to 3D); true (4D and up)
%
% SEE ALSO:
%  gradient, curvature, structuretensor, dip_image/eig

% (c)2018, Cris Luengo.
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

function out = orientation(in,opt,sg,st,subs)

if nargin < 2
   opt = 'surface';
end
if nargin < 3
   sg = 1;
end
if nargin < 4
   st = 4;
end
if ~isa(in,'dip_image')
   in = dip_image(in);
end
if ~isscalar(in)
   error('Only implemented for scalar images');
end
n = ndims(in);
if n < 2
   error('Orientation only makes sense with at least 2 dimensions');
end
if nargin < 5
   subs = n > 3;
end

if subs
   G = gst_subsample(in,sg,st);
else
   G = structuretensor(in,sg,st);
end
switch opt
case 'line'
   out = eig(G,'smallest');
case 'surface'
   out = eig(G,'largest');
otherwise
   error('Unkown method');
end
