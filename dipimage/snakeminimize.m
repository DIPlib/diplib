%SNAKEMINIMIZE   Minimizes the energy function of a snake
%
% SYNOPSIS:
%  snake_out = snakeminimize(snake_in,Fext,alpha,beta,stepsz,kappa,iterations)
%
% PARAMETERS:
%  snake_in :   initial snake, Nx2 array with coordinates
%  Fext :       external force field, 2D-2D vector image, for example
%               GRADIENTVECTOR, GVF, VFC
%  alpha :      elasticity parameter (membrane)
%  beta :       rigidity parameter (thin plate)
%  stepsz :     step size
%  kappa :      balloon force (negative for inwards force)
%  iterations : number of iterations performed
%
% DEFAULTS:
%  alpha = 0.2
%  beta = 0.4
%  stepsz = 1
%  kappa = 0
%  iterations = 20
%
% NOTE:
%  You can create the initial snake coordinates using, for example, IM2SNAKE or
%  DIPDRAWPOLYGON.
%
% EXAMPLE:
%  a = noise(50+100*gaussf(rr>85,2),'gaussian',20)
%  f = gradient(gradmag(a,5));
%  f = f./max(norm(f));
%  x = 100+30*cos(0:0.1:2*pi); y = 150+40*sin(0:0.1:2*pi);
%  s = [x.',y.'];
%  snakedraw(s);
%  s = snakeminimize(s,f,0.01,100,3,0.3,20);
%  h = snakedraw(s);
%  set(h,'color',[0,0.7,0.7]);
%  s = snakeminimize(s,f,0.01,100,3,0.3,20);
%  snakedraw(s,h);
%  s = snakeminimize(s,f,0.01,100,3,0.3,20);
%  snakedraw(s,h);
%  s = snakeminimize(s,f,0.01,100,3,0.3,20);
%  snakedraw(s,h);
%  s = snakeminimize(s,f,0.01,100,3,0.3,20);
%  snakedraw(s,h);
%
% LITERATURE:
%  M. Kass, A. Witkin, D. Terzopoulos, "Snakes: Active Contour Models",
%     Int. J. of Computer Vision 4(1):321-331 (1988)
%  L.D. Cohen, I. Cohen, "Finite-element methods for active contour models and
%     balloons for 2-D and 3-D images", IEEE TPAMI 15(11):1131-1147 (1993)
%
% SEE ALSO:
%  snakedraw, snake2im, vfc, gvf, gradientvector, im2snake, dipdrawpolygon

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

function s = snakeminimize(s,f,alpha,beta,stepsz,kappa,iterations)

if ~isnumeric(s) || size(s,2)~=2
   error('Input array not valid as snake.');
end
if ~isa(f,'dip_image')
   f = dip_image(f);
end
if iscomplex(f) || ndims(f)~=2
   error('FEXT must be a 2D real-valued image')
end
if numtensorel(f)~=2
   error('FEXT must have 2D vectors ');
end
if nargin < 3
   alpha = 0.2;
else
   validateattributes(alpha,{'double'},{'scalar','positive'})
end
if nargin < 4
   beta = 0.4;
else
   validateattributes(beta,{'double'},{'scalar','positive'})
end
if nargin < 5
   stepsz = 1;
else
   validateattributes(stepsz,{'double'},{'scalar','integer','positive'})
end
if nargin < 6
   kappa = 0;
else
   validateattributes(kappa,{'double'},{'scalar'})
end
if nargin < 7
   iterations = 20;
else
   validateattributes(iterations,{'double'},{'scalar','integer','positive'})
end

[maxx,maxy] = imsize(f);
maxx = maxx-1;
maxy = maxy-1;

x = s(:,1);
y = s(:,2);
if length(x)<3
   error('Snake has too few control points.')
end

% Constrict points to image
x(x<0) = 0;
x(x>maxx) = maxx;
y(y<0) = 0;
y(y>maxy) = maxy;

% Do the snake!
md = 1; % The average distance we want to keep between points.
[x,y] = resample(x,y,md);
%h = snakedraw([x,y]);
P = compute_matrix(length(x),alpha,beta,stepsz);
for ii = 1:iterations
   %disp(['iteration ii=',num2str(ii),', snake length = ',num2str(length(x))])

   % Do we need to resample the snake?
   d = sqrt( diff(x).^2 + diff(y).^2 );
   if any(d<md/3) || any(d>md*3)
      %disp('resampling snake')
      [x,y] = resample(x,y,md);
      P = compute_matrix(length(x),alpha,beta,stepsz);
   end
   
   % Calculate external force
   s = [x,y];
   p = get_subpixel(f,s,'linear');
   fex = p(:,1);
   fey = p(:,2);

   % Calculate balloon force
   if kappa~=0
      b = [s(2:end,:);s(1,:)] - [s(end,:);s(1:end-1,:)];
      m = sqrt(sum(b.^2,2));
      bx =  b(:,2)./m;
      by = -b(:,1)./m;
      % Add balloon force to external force
      fex = fex+kappa*bx;
      fey = fey+kappa*by;
   end

   % Move control points
   x = P*(x+stepsz*fex);
   y = P*(y+stepsz*fey);

   % Constrict points to image
   x(x<0) = 0;
   x(x>maxx) = maxx;
   y(y<0) = 0;
   y(y>maxy) = maxy;
   
   %if mod(ii,10)
   %   snakedraw([x,y],h);
   %   drawnow;
   %   pause(0.5);
   %end

end
s = [x,y];



% The matrix P = (stepsz*A+I)^(-1)
function P = compute_matrix(N,alpha,beta,stepsz)
a = stepsz*(2*alpha+6*beta)+1;
b = stepsz*(-alpha-4*beta);
c = stepsz*beta;
P = diag(repmat(a,1,N));
P = P + diag(repmat(b,1,N-1), 1) + diag(   b, -N+1);
P = P + diag(repmat(b,1,N-1),-1) + diag(   b,  N-1);
P = P + diag(repmat(c,1,N-2), 2) + diag([c,c],-N+2);
P = P + diag(repmat(c,1,N-2),-2) + diag([c,c], N-2);
P = inv(P);



% Resamples the snake
function [x,y] = resample(x,y,d)
mode = 'linear'; r = 1;
% ( Alternative: mode = 'cubic'; r = 3; )
% replicate samples at either end (periodic boundary condition)
x = [x(end-r+1:end);x;x(1:r)];
y = [y(end-r+1:end);y;y(1:r)];
% p is the parameter along the snake
p = [0;cumsum(sqrt( diff(x).^2 + diff(y).^2 ))];
% the first control point should be at p=0
p = p-p(r+1);
% resample snake between first and last+1 control points
x = interp1(p,x,0:d:p(end-r+1),mode);
y = interp1(p,y,0:d:p(end-r+1),mode);
% if the last new point is too close to the first one, remove it
if norm([x(end),y(end)]-[x(1),y(1)]) < d/2
   x(end) = [];
   y(end) = [];
end`
% ensure column vectors
x = x(:);
y = y(:);
if length(x)<3
   error('Snake has become too small!')
end
