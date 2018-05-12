%CURVATURE   Curvature in grey value images in nD
%
% SYNOPSIS:
%  out = curvature(in,method,sg,st,subsample)
%
%  method:    'line', 'surface', 'thirion' or 'isophote'; see below
%  sg:        sigma of the gradient derivative
%  st:        sigma of the tensor smoothing (for 'line' and 'surface' only)
%  subsample: a boolean value, the output will be subsampled by a factor
%             FLOOR(ST) if TRUE, but only for 'line' and 'surface' methods,
%             where ST is used for smoothing. Useful for very large images.
%
% DEFAULTS:
%  method = 'line'
%  sg = 1
%  st = 4
%  subsample = false (up to 3D); true (4D and up)
%
% METHODS:
%  The structure tensor method computes the derivatives of the principal
%  direction of the structure tensor. The structure tensor provides a robust
%  estimate of the principal orientations of the local structure. For 'line'
%  it computes the derivative of the principal orientation in the direction
%  of the principal orientation (Ginkel et al, 1999, Rieger and van Vliet,
%  2002). For 'surface' it computes the derivative of the principal
%  orientation in all perpendicular orientations (Rieger et al, 2004). These
%  methods work for any number of dimensions, but the 'line' method requires
%  at least 2 dimensions, and the 'surface' method 3. For 4D and higher, the
%  output will be subsampled by ST.
%
%  The other two methods compute the derivative of the gradient in all
%  directions perpendicular to the gradient. The gradient is less robust than
%  the structure tensor, and disappears at grey-value ridges. These methods
%  only return valid information at locations with a clear gradient (object
%  edges). However, they can determine the sign of the curvature, as oposed
%  to the structure tensor method.
%
%  'thirion' implements the method by Thirion and Gourdon (1995), and
%  'isophote' the method by Verbeek (1985) and van Vliet (1993). For 2D,
%  these methods are the same, but they differ in 3D. Neither is applicable
%  to higher dimensionalities.
%
% LINES:
%  out: magnitude of the curvature (1/bending radius)
%
% SURFACES:
%  out: magnitude of the principal curvatures (1/bending radius)
%
% THIRION:
%  out: image containing the principal curvatures (1/radius)
%       The object is considered white.
%       Positive/negative curvatures for elliptic/hyperbolic structures.
%
% ISOPHOTE:
%  out: image containing the principal curvatures (1/radius)
%       The object is considered white.
%       Positive/negative curvatures for elliptic/hyperbolic structures.
%
% EXAMPLE:
%  a = sin(rr/3)
%  b = curvature(a,'line',1,4)
%  c = curvature(a,'isophote',1)
%
% EXAMPLE:
%  % Curvature of a 3D ball with curvature 0.066 (radius 15)
%  a = drawshape(newim([50,50,50]),15,[25,25,25],'ball',1,1);
%  k = curvature(a,'isophote',1); % also try 'surface' or 'thirion' here
%  dipshow(1,a,'lin');
%  dipshow(2,k{1},'percentile');
%  dipshow(3,k{2},'percentile');
%  diplink(1,[2 3]);
%  dipmapping(1,'slice',25);
%
% SEE ALSO:
%  gradient, orientation, structuretensor, dip_image/eig
%
% LITERATURE:
%  M. van Ginkel, J. van de Weijer, P.W. Verbeek, and L.J. van Vliet.
%   Curvature estimation from orientation fields. In B.K. Ersboll and P.
%   Johansen, editors, SCIA'99, Proc. 11th Scandinavian Conf. on Image
%   Analysis (Kangerlussuaq, Greenland), p. 545-551. Pattern Recognition
%   Society of Denmark, June 7-11 1999.
%
%  B. Rieger, L.J. van Vliet, Curvature of n-dimensional space curves in
%   grey-value images, IEEE Transactions on Image Processing 11(7):738-745,
%   2002.
%
%  B. Rieger, F.J. Timmermans, van L.J. Vliet and P.W. Verbeek, On curvature
%   estimation of surfaces in 3D grey-value images and the computation of
%   shape descriptors, IEEE Transactions on Pattern Analysis and Machine
%   Intelligence 26(8):1088-1094, 2004
%
%  J.P. Thirion and A. Gourdon, Computing the differential characteristics of
%   isointensity surfaces, Computer Vision and Image Understanding
%   61(2):190-202, 1995.
%
%  P.W. Verbeek, A Class of Sampling-error Free Measures in Oversampled
%   Band-Limited Images, Pattern Recognition Letters, 3:287-292, 1985.
%
%  L.J. van Vliet, Grey-Scale Measurements in Multi-Dimensional Digitized
%   Images. PhD thesis, Delft University of Technology, 1993. Chapter 5.

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

function out = curvature(in,opt,sg,st,subs)

if nargin < 2
   opt = 'line';
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
   error('Curvature only makes sense with at least 2 dimensions');
end
if nargin < 5
   subs = n > 3;
end

switch opt
case 'line'
   switch n
      case 2
         out = structuretensor(in,sg,st,{'curvature'});
         if subs
            out = subsample(out,floor(st)); % Subsampling after the fact, this could be more efficient
         end
      case 3
         out = line3D(in,sg,st,subs);
      otherwise
         out = lineND(in,sg,st,subs);
   end
case 'surface'
   if n < 3
      error('SURFACE method requires at least 3 dimensions');
   end
   switch n
      case 3
         out = surface3D(in,sg,st,subs);
      otherwise
         out = surfaceND(in,sg,st,subs);
   end
case 'thirion'
   switch n
      case 2
         out = isophote2D(in,sg);
      case 3
         out = thirion3D(in,sg);
      otherwise
         error('Method not defined for more than 3 dimensions');
   end
case 'isophote'
   switch n
      case 2
         out = isophote2D(in,sg);
      case 3
         out = isophote3D(in,sg);
      otherwise
         error('Method not defined for more than 3 dimensions');
   end
otherwise
   error('Unkown method');
end

function out = line3D(in,sg,st,subs)
% Use analytic solution for GST and 5D Knutsson mapping
[phi,theta] = structuretensor(in,sg,st,{'phi3','theta3'});
if subs
   phi = subsample(phi,floor(st)); % Subsampling after the fact, this could be more efficient
   theta = subsample(theta,floor(st));
end
sintheta = sin(theta);
costheta = cos(theta);
sin2theta = sin(2*theta);
clear theta
sinphi = sin(phi);
cosphi = cos(phi);
V = newtensorim(cosphi*sintheta, sinphi*sintheta, costheta); % smallest eigenvector
T = sintheta.^2 * cos(2*phi);
out = dotGradientSquare(V,T);
T = sintheta.^2 * sin(2*phi);
clear phi sintheta
out = out + dotGradientSquare(V,T);
T = sin2theta * cosphi;
clear cosphi
out = out + dotGradientSquare(V,T);
T = sin2theta * sinphi;
clear sinphi sin2theta
out = out + dotGradientSquare(V,T);
T = sqrt(3) * (costheta.^2 - 1/3);
clear costheta
out = out + dotGradientSquare(V,T);
clear T
out = sqrt(out)*0.5;

function out = lineND(in,sg,st,subs)
if subs
   G = gst_subsample(in,sg,st);
else
   G = structuretensor(in,sg,st);
end
V = eig(G,'smallest'); % smallest eigenvector
clear G
T = V * V.';
out = dotGradientSquare(V,T{1});
for ii = 2:numtensorel(T)
   out = out + dotGradientSquare(V,T{ii});
end
clear T V
out = sqrt(out)*(1/sqrt(2));

function out = surface3D(in,sg,st,subs)
% Use 5D Knutsson mapping
if subs
   G = gst_subsample(in,sg,st);
else
   G = structuretensor(in,sg,st);
end
[V,~] = eig(G);
clear G
N = {V{1,1},V{2,1},V{3,1}}; % largest eigenvector
V = {V{:,2};V{:,3}};
N12 = N{1}.^2;
N22 = N{2}.^2;
T = N12 - N22;
out = dotGradientSquare(V,T);
T = (1/sqrt(3)) * (2*N{3}.^2 - N12 - N22);
clear N12 N22
out = addCells(out,dotGradientSquare(V,T));
T = 2*N{1}*N{2};
out = addCells(out,dotGradientSquare(V,T));
T = 2*N{1}*N{3};
out = addCells(out,dotGradientSquare(V,T));
T = 2*N{2}*N{3};
clear N
out = addCells(out,dotGradientSquare(V,T));
clear T V
out = sqrt(dip_image(out))*0.5;

function out = surfaceND(in,sg,st,subs)
if subs
   G = gst_subsample(in,sg,st);
else
   G = structuretensor(in,sg,st);
end
[T,~] = eig(G);
clear G
V = cell(tensorsize(T,2)-1,1);
for ii = 1:numel(V)
   V{ii} = T{:,ii+1};
end
T = T{:,1}; % largest eigenvector
T = T * T.';
out = dotGradientSquare(V,T{1});
for ii = 2:numtensorel(T)
   out = addCells(out,dotGradientSquare(V,T{ii}));
end
clear V T
out = sqrt(dip_image(out))*(1/sqrt(2));

function out = isophote2D(in,sg)
g = gradient(in,sg);
g = newtensorim(g{2},-g{1}); % perpendicular to the gradient
H = hessian(in,sg);
div = dot(g,g).^(3/2);
out = (-g.'*H*g) ./ div;
out(div<1e-10) = 0;

function out = isophote3D(in,sg)
myeps = 1e-10;
fx = dx(in,sg);
fy = dy(in,sg);
fz = dz(in,sg);
r = fx.^2+fy.^2;
mg = sqrt(r+fz.^2);
r = sqrt(r);
m = r < sqrt(myeps);
cp = fx./r;
sp = fy./r;
ct = r./mg;
st = fz./mg;
cp(m) = 1; % gradient along z axis
st(m) = 1;
sp(m) = 0;
ct(m) = 0;

R = cell(3,3);
R{1,1} = cp*ct;
R{1,2} = -sp;
R{1,3} = -cp*st;
R{2,1} = sp*ct;
R{2,2} = cp;
R{2,3} = -sp*st;
R{3,1} = st;
R{3,2} = newim(size(in));
R{3,3} = ct;
R = dip_image(R);
H = hessian(in,sg);
H = R.'*H*R; % change of basis,  Hessian aligned with g

K = cell(3,1);
K{1} = H{2,2};
K{3} = H{2,3};
K{2} = H{3,3};
clear H
K = dip_image(K,'symmetric matrix');
l = eig(K);
out = l{1:2};
out = -out./mg;
out(mg<myeps) = 0;

function out = thirion3D(in,sg)
fx = dx(in,sg);
fy = dy(in,sg);
fz = dz(in,sg);
fxfx = fx .* fx;
fyfy = fy .* fy;
fzfz = fz .* fz;
fxfy = fx .* fy;
fxfz = fx .* fz;
fyfz = fy .* fz;
fxx = dxx(in,sg);
fyy = dyy(in,sg);
fzz = dzz(in,sg);
fxy = dxy(in,sg);
fxz = dxz(in,sg);
fyz = dyz(in,sg);
h = fxfx + fyfy + fzfz;
mask = h < 1e-6;
h = 1 ./ h;
h(mask) = 0;
K = (   fxfx .* (fyy .* fzz - fyz .* fyz) ...
      + fyfy .* (fxx .* fzz - fxz .* fxz) ...
      + fzfz .* (fxx .* fyy - fxy .* fxy) ...
      + 2 * (   fyfz .* (fxz .* fxy - fxx .* fyz) ...
              + fxfz .* (fyz .* fxy - fyy .* fxz) ...
              + fxfy .* (fxz .* fyz - fzz .* fxy) ...
            ) ...
    ) .* (h .* h);
H = (   fxfx .* (fyy + fzz) ...
      + fyfy .* (fxx + fzz) ...
      + fzfz .* (fxx + fyy) ...
      - 2 * (   fyfz .* fyz ...
              + fxfz .* fxz ...
              + fxfy .* fxy ...
            ) ...
    ) .* 0.5 .* h.^(3/2);
delta = H.^2 - K;
delta(delta<0) = 0;
delta = sqrt(delta);
out = newtensorim(H + delta, H - delta);

function out = dotGradientSquare(V,T)
if iscell(V)
   % V is a cell array of vector images
   T = gradient(T,1);
   out = cell(size(V));
   for ii = 1:numel(V)
      out{ii} = dot(V{ii},T).^2;
   end
else
   % V is a vector image
   out = dot(V,gradient(T,1)).^2;
end

function s = addCells(s,n)
s = cellfun(@plus,s,n,'UniformOutput',false);
