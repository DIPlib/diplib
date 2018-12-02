%CROSSCORRELATION   Computes the cross-correlation between two images
%
% SYNOPSIS:
%  image_out = crosscorrelation(image1,image2,normalize,rep1,rep2,rep_out)
%
% PARAMETERS:
%  normalize: Choose whether to normalize the cross correlation. One of:
%    - '':          No normalization
%    - 'normalize': A normalization similar to phase correlation, but cheaper.
%    - 'phase':     Computes phase correlation.
%  rep1:      'spatial' or 'frequency', for IMAGE1
%  rep2:      'spatial' or 'frequency', for IMAGE2
%  rep_out:   'spatial' or 'frequency', for IMAGE_OUT
%
% DEFAULTS:
%  normalize = ''
%  rep1 = 'spatial'
%  rep2 = 'spatial'
%  rep_out = 'spatial'
%
% DIPlib:
%  This function calls the DIPlib function dip::CrossCorrelationFT.

% (c)2017-2018, Cris Luengo.
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

function out = crosscorrelation(varargin)
out = dip_geometry('crosscorrelation',varargin{:});
