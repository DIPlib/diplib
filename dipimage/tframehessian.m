%TFRAMEHESSIAN   Second derivatives driven by the structure tensor
%
% SYNOPSIS:
%  out = tframehessian(image_in, sg, st, sh)
%
%  Each tensor element of OUT is computed by the Rayleigh quotient
%  of the Hessian matrix and one of the eigenvectors of the stucture
%  tensor. The first component of OUT is equivalent to `DGG(IMAGE_IN)`,
%  but replacing the gradient direction with the main direction of
%  the structure tensor.
%
% PARAMETERS:
%  sg: sigma of derivative
%  st: sigma of tensor
%  sh: sigma of hessian
%
% DEFAULTS:
%  sg = 1
%  st = 3
%  sh = sg
%
% SEE ALSO:
%  dgg, dcc

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

function out = tframehessian(in,sg,st,sh)

if nargin<2
   sg = 1;
end
if nargin<3
   st = 3;
end
if nargin<4
   sh = sg;
end

if ~isa(in,'dip_image')
   in = dip_image(in);
end

if ndims(in)==1
   out = dxx(in,sg);
else
   S = structuretensor(in,sg,st);
   [V,~] = eig(S);
   S = [];
   H = hessian(in,sh);
   %out = newtensorim(ndims(in),imsize(in));
   out = cell(ndims(in),1);
   for ii=1:ndims(in)
      t = V{:,ii};
      out{ii} = t.' * H * t;
   end
   H = [];
   V = [];
   out = dip_image(out); % join all elements into a vector image
end
