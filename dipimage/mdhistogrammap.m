%MDHISTOGRAMMAP   Reverse map a multi-dimensional histogram
%
% Reverse maps a histogram computed by mdhistogrammap to image space. The
% first step is to give interesting regions in the histogram a label.
% Invoking MDHISTOGRAMMAP then shows you which pixels in the original image
% space contributed to which region in the histogram by mapping the labels
% in the labeled histogram to image space.
%
% SYNOPSIS:
%  image_out = mdhistogrammap(image_in,histogram,bins,exclude_out_of_bounds_values)
%
% PARAMETERS:
%  image_in:  An image similar to the one that HISTOGRAM was derived from.
%             It must have the same number of tensor elements (channels) as
%             dimensions in HISTOGRAM.
%  histogram: A multi-dimensional histogram, preferably labeled using CLUSTER
%             or similar. Must be of an unsigned type (binary or unsigned
%             integer).
%  bins:      The cell array of bin centers as returned by MDHISTOGRAM.
%  exclude_out_of_bounds_values: Specifies what to do with input pixels that
%             fall out of the histogram bounds: if true, assigns 0 to them,
%             if false, uses the edge bin value. Can be an array with one
%             value per histogram dimension.
%
% DEFAULTS:
%  exclude_out_of_bounds_values = false
%
% EXAMPLE:
%  image = readim('flamingo');
%  [histogram,bins] = mdhistogram(image);
%  hist_labeled = cluster(histogram,5);
%  labeled = mdhistogrammap(image,hist_labeled,bins);
%
% SEE ALSO:
%  mdhistogram
%
% DIPlib:
%  This function uses the DIPlib function dip::Histogram::ReverseLookup.

% (c)2006, 2017, Michael van Ginkel.
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

function out = mdhistogrammap(varargin)
out = dip_math('mdhistogrammap',varargin{:});
