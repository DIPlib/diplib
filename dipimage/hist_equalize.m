%HIST_EQUALIZE   Histogram equalization
%
% SYNOPSIS:
%  image_out = hist_equalize(image_in,histogram_in)
%
% PARAMETERS:
%  histogram_in: Histogram to match. The output will have a histogram
%                as close as possible to the histogram of this image.
%                Give [] to generate a histogram that is as flat as
%                possible (the traditional histogram equalization).
%
% DEFAULTS:
%  histogram_in = [].
%
% EXAMPLE 1: Equalize the histogram of an image
%  a = readim('trui');
%  diphist(a,32,'bar'), title('original histogram')
%  b = hist_equalize(a);
%  diphist(b,32,'bar'), title('modified histogram')
%
% EXAMPLE 2: Match the histogram of another image
%  a = readim('trui');
%  diphist(a,32,'bar'), title('original histogram')
%  b = readim('orka');
%  diphist(b,32,'bar'), title('histogram to match')
%  c = hist_equalize(a,diphist(b));
%  diphist(c,32,'bar'), title('matched histogram')
%
% EXAMPLE 3: Force the histogram of an image to be Gaussian
%  a = readim('trui');
%  diphist(a,32,'bar'), title('original histogram')
%  h = exp(-(((0:255)-128)/60).^2);
%  b = hist_equalize(a,h);
%  diphist(b,32,'bar'), title('modified histogram')
%
% NOTES:
%  The bins of HISTOGRAM_IN are assumed to correspond to integer grey
%  values starting at 0. That is, BINS = 0:LENGTH(HISTOGRAM_IN)-1.
%  These bin numbers correspond to the generated output grey values.
%
%  With no second input argument, a flat histogram with 256 bins is
%  assumed, meaning that the output image is always in the range [0,255].
%
% DIPlib:
%  This function calls the DIPlib functions dip::HistogramEqualization
%  and dip::HistogramMatching.

% (c)2018, Cris Luengo.
% Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
%                                  (c)2013, Patrik Malm & Cris Luengo.
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

function out = hist_equalize(varargin)
out = dip_math('hist_equalize',varargin{:});
