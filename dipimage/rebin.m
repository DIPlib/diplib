%REBIN   Rebinning of an image
%
%  Reduces size of an image by an integer factor, by grouping (binning)
%  pixels together. Each output pixel is the sum of a group of input
%  pixels.
%
% SYNOPSIS:
%  out = rebin(in, binning)
%
% PARAMETERS:
%  in:      input image
%  binning: integer numbers (array) that divides the image without
%           remainder
%
% DEFAULTS:
%  binning = 2
%
% SEE ALSO:
%  resample, subsample, split

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

function out = rebin(in,binning)
if nargin<2
   binning = 2;
end
out = split(in,binning);
out = sum(out,[],ndims(out));
