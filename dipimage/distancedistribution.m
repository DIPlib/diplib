%DISTANCEDISTRIBUTION   Compute a histogram of distances within each label
%
% SYNOPSIS:
%  distribution = distancedistribution(object,region,length)
%
% PARAMETERS:
%  object: labeled image
%  region: binary image
%  length: maximum distance, in pixels, accounted for
%
%  The output DISTRIBUTION is an array where the first column is the distance
%  sample locations. Subsequent columns represent the distribution of distances
%  to the background of REGION for each of the labels (zero included) of OBJECT.
%  That is, column 2 is the distribution for label 0, column 3 for label 1, etc.
%
% DEFAULTS:
%  length = 100
%
% EXAMPLE:
%  a = readim('cermet');
%  b = label(a<120);
%  d = distancedistribution(b,b,15);
%  figure; hold on
%  plot(d(:,1),d(:,27))
%  plot(d(:,1),d(:,29))
%  plot(d(:,1),d(:,38))
%
% SEE ALSO:
%  diphist, mdhistogram
%
% DIPlib:
%  This function calls the DIPlib functions dip::DistanceDistribution.

% (c)2018, Cris Luengo.
% Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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

function out = distancedistribution(varargin)
out = dip_analysis('distancedistribution',varargin{:});
