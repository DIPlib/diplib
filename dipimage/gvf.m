%GVF   Computes an external force using Gradient Vector Flow
%
% SYNOPSIS:
%  f = gvf(edge_image,mu,iterations)
%
% PARAMETERS:
%  edge_image : scalar image that has large values at the locations you
%               want the snake to end up ( e.g.: gradmag(a) ), only 2D
%               images are supported
%  mu :         smoothness parameter
%  iterations : number of iterations computed
%
% DEFAULTS:
%  mu = 0.2
%  iterations = 80
%
% EXAMPLE:
%  a = noise(50+100*gaussf(rr>85,2),'gaussian',20)
%  f = gvf(gradmag(a,8),1);
%  x = 120+50*cos(0:0.1:2*pi); y = 140+60*sin(0:0.1:2*pi);
%  s = [x.',y.'];
%  snakedraw(s);
%  s = snakeminimize(s,f,0.2,0.4,1,0,60);
%  snakedraw(s);
%
% LITERATURE:
%  C. Xu, J.L. Prince, "Snakes, Shapes and Gradient Vector Flow",
%     IEEE-TIP 7(3):359-369 (1998)
%
% SEE ALSO:
%  snakeminimize, vfc, gradientvector

% (c)2009, 2018, Cris Luengo.
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

function f = gvf(edge_image,mu,iterations)

if ~isa(edge_image,'dip_image')
   edge_image = dip_image(edge_image);
end
if ~isscalar(edge_image) || iscomplex(edge_image) || ndims(edge_image)~=2
   error('EDGE_IMAGE must be a 2D real-valued scalar image')
end
if nargin < 2
   mu = 0.2;
else
   validateattributes(mu,{'double'},{'scalar','positive'})
end
if nargin < 3
   iterations = 80;
else
   validateattributes(iterations,{'double'},{'scalar','integer','positive'})
end

f = gradient(edge_image);
b = norm(f);
c = b .* f;
for ii = 1:iterations
   %f{1} = f{1} + mu * laplace(f{1}) - b * f{1} + c{1};
   %f{2} = f{2} + mu * laplace(f{2}) - b * f{2} + c{2};
   f = f + mu .* laplace(f) - b .* f + c;
end
f = f ./ max(norm(f),1e-10);
