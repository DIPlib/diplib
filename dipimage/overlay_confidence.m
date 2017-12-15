%OVERLAY_CONFIDENCE   Overlay a grey-value image with a grey-value image
%
% SYNOPSIS:
%  image_out = overlay_confidence(grey,confidence,factor)
%
% PARAMETERS:
%  grey: input grey-value image
%  confidence: grey-value image between [0,1], where 1 is high confidence
%  factor: scaling of the intensity [0,1]
%
% DEFAULTS:
%  factor = .75;
%
% Image areas with 
%   low  confidence: dark
%   high confidence: bright
%   low  signal    : green 
%   high signal    : white
%
% HINT: It maybe useful to scale your input image by square root or similar

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

function out = overlay_confidence(grey,conf,fac)
if ~isa(grey,'dip_image')
   grey = dip_image(grey);
end
if ~isa(conf,'dip_image')
   conf = dip_image(conf);
end
if min(conf)<0 || max(conf)>1
   warning('CONFIDENCE out of bounds. Rescaling to interval [0,1]')
   conf = stretch(conf,0,100,0,1);
end
if ~isequal(imsize(grey),imsize(conf))
   error('GREY and CONFIDENCE must have the same sizes')
end
if ~isscalar(grey) || ~isscalar(conf)
   error('GREY and CONFIDENCE must be scalar images')
end
if nargin<3
   fac = 0.75;
elseif ~isnumeric(fac) || ~isscalar(fac) || fac<0 || fac>1
   error('FACTOR must be a number between 0 and 1')
end

% OLD METHOD:
%a = grey/max(grey)*255*fac;
%g = (1-conf)*255 + conf*a;
%out = joinchannels('RGB',a,g,a);

% NEW METHOD:
%   betrouwbaar is licht, onbetrouwbaar is donker,
%   laag signaal is groen, hoog signaal is wit 
inten = stretch(grey,0,100,0,255); 
g = (255-(1-fac)*inten)*conf;
a = inten*fac*conf;
out = joinchannels('RGB',a,g,a);
