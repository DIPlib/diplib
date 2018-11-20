%GAUSSF   Gaussian filter
%
% SYNOPSIS:
%  image_out = gaussf(image_in,sigma,method,boundary_condition,truncation)
%
% PARAMETERS:
%  sigma: Gaussian parameter for each dimension
%  method: Method used to compute the Gaussian. One of:
%    - 'fir':    Finite Impulse Resonse filter (convolution with a kernel).
%    - 'iir':    Infinte Impulse Response filter (recursive filtering).
%    - 'ft':     Convolution via a multiplication in the Fourier Domain.
%    - 'best':   Chooses the best option above for your kernel.
%    - 'kernel': The convolution kernel is returned, rather than the result
%                of the convolution.
%  boundary_condition: Defines how the boundary of the image is handled.
%                      See HELP BOUNDARY_CONDITION
%  truncation: Determines the size of the Gaussian filters.
%
% DEFAULTS:
%  sigma = 1
%  metod = 'best'
%  bounary_condition = 'mirror'
%  truncation = 3
%
% NOTES:
%  See DERIVATIVE for an explanation of the option 'best'.
%
%  If SIGMA==0 for a particular dimension, that dimension is not processed.
%
% SEE ALSO:
%  derivative
%
% DIPlib:
%  This function calls the DIPlib function dip::Derivative (which calls dip::GaussFIR,
%  dip::GaussIIR and dip::GaussFT) and dip::CreateGauss.

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

function image_out = gaussf(image_in,sigma,method,varargin)
% The code below looks a little silly, but it's an easy way to not parse input arguments twice.
if nargin < 2
   image_out = filtering('derivative',image_in);
elseif nargin < 3
   image_out = filtering('derivative',image_in,0,sigma);
else
   if ~ischar(method) && ~isvector(method)
      error('METHOD argument must be a string');
   end
   if ~strcmp(method,'best') && ~strcmp(method,'kernel')
      method = ['gauss',method];
   end
   image_out = filtering('derivative',image_in,0,sigma,method,varargin{:});
end
