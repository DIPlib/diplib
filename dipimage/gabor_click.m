%GABOR_CLICK  Interactive Gabor filter
%
% SYNOPSIS:
%  image_out = gabor_click(image_in,sigma,figh)
%
% PARAMETERS:
%  image_in: ORIGINAL image
%  sigma:    sigma in the spatial domain
%  figh:     figure handle for FOURIER image in which to select a position
%
%  Click on the the pixel position in the FOURIER image where you want to
%  apply the filter.
%
% DEFAULTS:
%  sigma = 5
%  figh = gcf (the current figure)

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

function image_out = gabor_click(image_in,sigma,figh)

if ~isa(image_in,'dip_image')
   image_in = dip_image(image_in);
end
if ndims(image_in)~=2
   error('Gabor_click only impelemented for 2 dimensional images.');
end

if nargin < 2
   sigma = 5;
end
if nargin < 3
   figh = gcf;
end

x = dipgetcoords(figh,1);
if x(1)<0  % dipgetcoords returns [-1,-1] when right-clicking
   error('Cancelled.');
end
sz = size(image_in);
f = (x-(sz/2))./sz;

fprintf('Selected frequencies: %f %f [cartesian]\n',f);
fprintf('Spatial pattern: distance %f [pix], angle: %f [deg]\n',1/norm(f),atan2(f(2),f(1))*180/pi+90);

% Note: angle is mathematically positive on left turn, but y-axis is down
% and Fourier is 90deg rotated

image_out = gabor(image_in,sigma,f);
