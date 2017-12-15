%OVERLAY_VECTOR  Overlay an image with a vector image
%
% SYNOPSIS:
%  out = overlay_vector(in,vector,subsample,scaling)
%
% PARAMETERS:
%  in: a grey-value or color image, 2D
%  vector: a vector image, same size as IN, 2D vectors
%  subsampling: subsampling for ploting the vectors
%  scaling: length of the average vector
%
%  OUT is an RGB image of the same size as IN.
%
% DEFAULTS:
%  subsample = 4
%  scaling = 2
%
% EXAMPLE:
%  a = readim('trui')
%  overlay_vector(a,gradient(a))
%
% NOTE:
%  Green means the vector is pointing up, red down.
%  We plot lines only for those points where the gradient is larger
%  than the average gradient.

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

function out = overlay_vector(in,vector,subsample,scale)
if ~isa(in,'dip_image')
   in = dip_image(in);
end
if ndims(in)~=2
   error('IN must be a 2D image')
end
if ~isscalar(in) && ~iscolor(in)
   error('IN must be grey-value or color')
end
if ~isa(vector,'dip_image') || numtensorel(vector)~=2
   error('VECTOR must be a 2D vector image')
end
if ~isequal(imsize(in),imsize(vector))
   error('IN and VECTOR must have the same sizes')
end
if nargin<3
   subsample = 4;
elseif ~isnumeric(subsample) || ~isscalar(subsample) || subsample <= 0
   error('SUBSAMPLING must be a positive number')
end
if nargin<4
   scale = 2;
elseif ~isnumeric(scale) || ~isscalar(scale) || scale <= 0
   error('SCALING must be a positive number')
end

% Normalize the vectors
ng = norm(vector);
mg = mean(ng);
vector = vector./mg;

% Sampling the vectors
m = newim(in,'bin');
start = round(subsample/2);
m(start:subsample:end,start:subsample:end) = 1;
m = m & (ng >= mg);
ang_m = atan2(vector{2},vector{1})>0;
mr = m & ang_m;
mg = m & ~ang_m;

% Create output image
out = colorspace(in,'rgb'); % converts to RGB if it's any other color space
u = max(max(out)); % first max is over all pixels, second one over tensor elements
l = min(min(out));
out = (out - l) * (255 / (u - l));

% Draw the red vectors
sc = findcoord(mr);
ec = sc + squeeze(double(scale*vector(mr)))';
ec = round(ec);
ec = bsxfun(@min,ec,imsize(in)-1);
ec = max(ec,0);
for ii = 1:length(sc)
   out = drawline(out,sc(ii,:),ec(ii,:),[255,0,0]);
end

% Draw the green vectors
sc = findcoord(mg);
ec = sc + squeeze(double(scale*vector(mg)))';
ec = round(ec);
ec = bsxfun(@min,ec,imsize(in)-1);
ec = max(ec,0);
for ii = 1:length(sc)
   out = drawline(out,sc(ii,:),ec(ii,:),[0,255,0]);
end
