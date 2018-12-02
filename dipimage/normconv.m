%NORMCONV   Normalized convolution with a Gaussian kernel
%
% SYNOPSIS:
%  image_out = normconv(image_in,confidence,dimension,sigma,method,boundary_condition,truncation)
%
% PARAMETERS:
%  image_in:   An image with missing or uncertain samples.
%  confidence: A scalar, real-valued image indicating the confidence in each
%              sample of IMAGE_IN. All values should be in the range [0,1].
%              A binary image is allowed as well.
%  dimension:  Either [] for smoothing with a Gaussian, or a dimension index
%              for estimating the derivative along that dimension.
%  sigma:      Gaussian parameter for each dimension.
%  method:     Method used to compute the derivative. One of:
%    - 'fir':     Finite Impulse Resonse filter (convolution with a kernel).
%    - 'iir':     Infinte Impulse Response filter (recursive filtering).
%    - 'ft':      Convolution via a multiplication in the Fourier Domain.
%    - 'best':    Chooses the best option above for your kernel.
%  boundary_condition: Defines how the boundary of the image is handled.
%                      See HELP BOUNDARY_CONDITION
%  truncation: Determines the size of the Gaussian filters.
%
% DEFAULTS:
%  dimension = []
%  sigma = 1
%  metod = 'best'
%  bounary_condition = 'mirror'
%  truncation = 3
%
% NOTES:
%  'best' selects 'gaussft' if any sigma < 0.8 or any order > 3; 'gaussiir' if any sigma > 10;
%  and 'gaussfir' otherwise.
%
% SEE ALSO:
%  gaussf, derivative
%
% DIPlib:
%  This function calls the DIPlib functions dip::NormalizedConvolution and
%  dip::NormalizedDifferentialConvolution.

% (c)2018, Cris Luengo.
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

function out = normconv(varargin)
out = dip_filtering('normconv',varargin{:});
