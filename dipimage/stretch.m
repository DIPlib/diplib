%STRETCH   Grey-value stretching
%
% SYNOPSIS:
%  image_out = stretch(image_in,low,high,minimum,maximum,method,param1,param2)
%
% PARAMETERS:
%  low:     lower percentile of image_in
%  high:    highest percentile of image_in
%  minimum: output lower bound
%  maximum: output upper bound
%  method:  mapping function to use, one of the following strings:
%     - 'linear': linear mapping
%     - 'signed linear': linear mapping with zero at fixed value in the middle of the output range
%     - 'logarithmic': logarithmic mapping
%     - 'signed logarithmic': logarithmic mapping with zero at fixed location in the output range
%     - 'erf': error function mapping
%     - 'decade': decade contrast stretch (uses PARAM1)
%     - 'sigmoid': sigmoid function contrast stretch (uses PARAM1 and PARAM2)
%  param1, param2: parameters to some of the mapping functions
%
% DEFAULTS:
%  low = 0
%  high = 100
%  minimum = 0
%  maximum = 255
%  method = 'linear'
%  param1 = 1
%  param2 = 0
%
% NOTES:
%  The LOW and HIGH percentiles of IMAGE_IN are first computed. The resulting values represent
%  the lower and upper bound of the input range. MINIMUM and MAXIMUM represent the lower and
%  upper bound of the output range. The mapping from the one range to the other is given by
%  METHOD as detailed below.
%
%  Upper and lower bounds may be reversed.
%
% METHODS:
%
%  'linear' applies a linear mapping. 'signed linear' does the same thing, but sets the zero
%  level of the input to the middle of the output range.
%
%  'logarithmic' applies a logarithmic mapping. 'signed logarithmic' applies a separate
%  logarithmic mapping to the positive and negative input values, and gives each half of the
%  output range.
%
%  'erf' applies an error function mapping. This is the only mode that does not clip the input
%  values, as the error function implies a "soft clipping" (see ERFCLIP). The input range
%  defines the parameters to the error function.
%
%  'decade' applies a base 10 logarithmic mapping over PARAM1 decades of input data.
%
%  'sigmoid' applies a sigmoidal mapping: x/(1+abs(x)), where PARAM1 represents the slope of
%  the sigmoid and PARAM2 the origin.
%
% DIPlib:
%  This function calls the DIPlib function dip::ContrastStretch.

% (c)2017-2018, Cris Luengo.
% Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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
