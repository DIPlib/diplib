%VFC   Computes an external force using Vector Field Convolution
%
% SYNOPSIS:
%  f = vfc(edge_image,fsize,gamma)
%
% PARAMETERS:
%  edge_image : scalar image that has large values at the locations you
%               want the snake to end up ( e.g.: gradmag(a) ), only 2D
%               images are supported
%  fsize :      size, in pixels, of the square support of the
%               convolution filter
%  gamma :      kernel parameter
%
% DEFAULTS:
%  fsize = 41
%  gamma = 2
%
% EXAMPLE:
%  a = noise(50+100*gaussf(rr>85,2),'gaussian',20)
%  f = vfc(gradmag(a,5),81);
%  x = 120+50*cos(0:0.1:2*pi); y = 140+60*sin(0:0.1:2*pi);
%  s = [x.',y.'];
%  snakedraw(s);
%  s = snakeminimize(s,f,0.2,0.4,1,0,60);
%  snakedraw(s);
%
% LITERATURE:
%  B. Li, S.T. Acton, "Active Contour External Force Using Vector Field
%     Convolution for Image Segmentation", IEEE-TIP 16(8):2096-2106 (2007)
%
% SEE ALSO:
%  snakeminimize, gvf, gradientvector

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

function out = vfc(edge_image,fsize,gamma)

if ~isa(edge_image,'dip_image')
   edge_image = dip_image(edge_image);
end
if ~isscalar(edge_image) || iscomplex(edge_image) || ndims(edge_image)~=2
   error('EDGE_IMAGE must be a 2D real-valued scalar image')
end
if nargin < 2
   fsize = 41;
else
   validateattributes(fsize,{'double'},{'scalar','integer','positive'})
end
if nargin < 3
   gamma = 2;
else
   validateattributes(gamma,{'double'},{'scalar'})
end

K = coordinates([fsize,fsize]);
rr = max(norm(K),1e-10);
K = -K ./ (rr.^(gamma+1));
out = newtensorim(convolve(edge_image,K{1}),convolve(edge_image,K{2}));
out = out ./ max(norm(out),1e-10);
