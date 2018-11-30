%QUANTIZE   Quantize intensity values or colors
%
% SYNOPSIS:
%  out = quantize(in,levels,method)
%
% PARAMETERS:
%  in:      input image
%  levels:  number of intensity levels or colors in output image
%  method:  one of: 'uniform', 'kmeans' or 'minvariance'.
%
% The 'uniform' method quantizes each channel to LEVELS levels,
% leading to LEVELS^3 levels for an RGB image.
%
% The 'kmeans' and 'minvariance' methos cluster the histogram
% using the given method (see CLUSTER for more details), then paints
% each pixel with the color of the cluster center. For these methods,
% LEVELS different colors are generated.
%
% NOTE that 'kmeans' can get very slow for larger number of LEVELS.
%
% DEFAULTS:
%  levels = 25
%  method = 'minvariance'
%
% SEE ALSO:
%  mdhistogram, mdhistogrammap, cluster

% (c)2018, Cris Luengo.
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

function out = quantize(in,levels,method)
if nargin<2
   levels = 25;
end
if nargin<3
   method = 'minvariance';
end
if strcmp(method,'uniform')
   mn = dip_image(min(in).',in.TensorShape);
   mx = dip_image(max(in).',in.TensorShape);
   out = round((in-mn)./(mx-mn)*levels)/levels.*(mx-mn)+mn;
else
   [hist,bins] = mdhistogram(in,[],{'lower',0,'upper',100,'bins',64});
   [hist,centers] = cluster(hist,levels,method);
   labs = mdhistogrammap(in,hist,bins,false);
   N = 1;
   if iscell(bins)
      N = numel(bins);
      for ii=1:N
         centers(:,ii) = bins{ii}(centers(:,ii)+1);
      end
   else
      centers(:,1) = bins(centers+1);
   end
   centers = [zeros(1,N);centers];
   out = lut(labs,centers);
end
