%MDHISTOGRAM   Compute a multi-dimensional histogram
%
% A traditional histogram computes the grey-value distribution of an
% image, the result being a one-dimensional histogram. This function
% extends that notion to multi-valued data. Each tensor element adds a
% histogram dimension. A scalar image yields the classical 1D histogram.
%
% SYNOPSIS:
%  [histogram,bins] = mdhistogram(image,mask,specs)
%
% PARAMETERS:
%  specs: cell array as described below
%
% The SPECS argument is used to specify the limits and number of bins in
% each dimension of the histogram. It is a cell array of cell arrays. It
% should contain as many cell arrays as there are tensor elements.
%
% Each cell array in the SPECS argument is a set of keyword-value pairs
% and individual keywords. It should contain three out of the four
% following keyword-value pairs:
%  'lower':   lower limit value.
%  'upper':   upper limit value.
%  'bins':    the number of bins.
%  'binsize': the size of these bins.
% Note that the fourth value can always be computed from the three given
% values. By default, 'lower' and 'upper' indicate percentiles, meaning
% that the lower and upper bounds of the histogram are given in relation
% to the image data. Setting them to 0 and 100 causes the histogram range
% to be stretched to the image range.
%
% Additionally, the following keywords can be given to change how the
% histogram is computed:
%  'lower_abs': Changes the meaning of the argument of 'lower'. Instead of
%               a percentile it is directly interpreted as the lower limit
%               of the data range.
%  'upper_abs': Same as 'lower_abs', but for 'upper'.
%  'exclude_out_of_bounds_values': Values below 'lower' and above 'upper'
%               are excluded from the histogram. By default, they are put
%               into the first or last bin.
%
% The HISTOGRAM output argument is an image where the first dimension (x)
% corresponds to the first tensor element of IMAGE, etc.
%
% The BINS output argument contains the bin centers. It is a cell array
% with one vector for each histogram dimension. In case of a 1D histogram,
% it is just a numeric vector.
%
% DEFAULTS:
%  mask = [] (all pixels are used)
%  specs = {{'lower',0,'upper',100,'bins',100}}
%
% EXAMPLE:
%  mchim1 = xx([256 256],'corner');
%  mchim2 = newim(mchim1) + 100;
%  mchim = newtensorim(mchim1,mchim2);
%  histim = mdhistogram(mchim,[],...
%     {{'lower',0,'upper',256,'bins',256,'lower_abs','upper_abs'},...
%      {'lower',0,'upper',256,'bins',256,'lower_abs','upper_abs'}});
%
% SEE ALSO:
%  diphist, diphist2d, mdhistogrammap
%
% DIPlib:
%  This function uses the DIPlib class dip::Histogram.

% TODO: I've removed some options and syntax from the original version of
% this function. Should these be added back in?

% (c)2001, Michael van Ginkel.
% (c)2017-2018, Cris Luengo.
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
