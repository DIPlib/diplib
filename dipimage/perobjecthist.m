%PEROBJECTHIST   Compute a histogram for each object in a labeled image
%
% SYNOPSIS:
%  distribution = perobjecthist(image_in,labels,specs,mode,background)
%
% PARAMETERS:
%  specs: cell array as described in MDHISTOGRAM
%  mode:  one of 'count' or 'fraction'
%  background:  one of 'include' or 'exclude'
%
%  The output DISTRIBUTION is an array where the first column is the histogram
%  bin centers. Subsequent columns represent the histograms for each of the
%  labels and each of the channels (tensor components) of IMAGE_IN. That is, for
%  non-scalar images, the output is not a joint histogram but the marginal
%  histograms. SPECS describes a single-channel histogram, the settings are
%  applied to all channels simultaneously.
%
% DEFAULTS:
%  specs = {{'lower',0,'upper',100,'bins',100}}
%  mode = 'fraction'
%  background = 'exclude'
%
% EXAMPLE:
%  a = readim('cermet');
%  b = label(a<120);
%  d = perobjecthist(a,b);
%
% SEE ALSO:
%  diphist, mdhistogram
%
% DIPlib:
%  This function calls the DIPlib functions dip::PerObjectHistogram.

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

function out = perobjecthist(varargin)
out = dip_analysis('perobjecthist',varargin{:});
