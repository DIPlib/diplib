%DERIVATIVE   Derivative filters
%
% SYNOPSIS:
%  image_out = derivative(image_in,order,sigma,method,boundary_condition,truncation)
%
% PARAMETERS:
%  order:  The derivative order for each dimension. Order = 0 only applies smoothing.
%  sigma:  Gaussian parameter for each dimension.
%  method: Method used to compute the derivative. One of:
%    - 'gaussfir':     Finite Impulse Resonse filter (convolution with a kernel).
%    - 'gaussiir':     Infinte Impulse Response filter (recursive filtering).
%    - 'gaussft':      Convolution via a multiplication in the Fourier Domain.
%    - 'finitediff':   Finite difference derivative approximation.
%    - 'best':         Chooses the best option above for your kernel.
%    - 'kernel':       The Gaussian convolution kernel is returned, rather than
%                      the result of the convolution.
%  boundary_condition: Defines how the boundary of the image is handled.
%                      See HELP BOUNDARY_CONDITION
%  truncation: Determines the size of the Gaussian filters.
%
% DEFAULTS:
%  order = 0
%  sigma = 1
%  metod = 'best'
%  bounary_condition = 'mirror'
%  truncation = 3
%
% NOTES:
%  'best' selects 'gaussft' if any sigma < 0.8 or any order > 3; 'gaussiir' if any sigma > 10;
%  and 'gaussfir' otherwise.
%
%  If SIGMA==0 and ORDER==0 for a particular dimension, that dimension is not processed.
%
% SEE ALSO:
%  gaussf, dx, dy, etc.
%
% DIPlib:
%  This function calls the DIPlib function <a href="https://diplib.org/diplib-docs/linear.html#dip-Derivative-Image-CL-Image-L-UnsignedArray--FloatArray--String-CL-StringArray-CL-dfloat-">dip::Derivative</a> (which calls <a href="https://diplib.org/diplib-docs/linear.html#dip-GaussFIR-Image-CL-Image-L-FloatArray--UnsignedArray--StringArray-CL-dfloat-">dip::GaussFIR</a>,
%  <a href="https://diplib.org/diplib-docs/linear.html#dip-GaussIIR-Image-CL-Image-L-FloatArray--UnsignedArray--StringArray-CL-UnsignedArray--String-CL-dfloat-">dip::GaussIIR</a>, <a href="https://diplib.org/diplib-docs/linear.html#dip-GaussFT-Image-CL-Image-L-FloatArray--UnsignedArray--dfloat--String-CL-String-CL-StringArray-CL">dip::GaussFT</a> and <a href="https://diplib.org/diplib-docs/linear.html#dip-FiniteDifference-Image-CL-Image-L-UnsignedArray--String-CL-StringArray-CL-BooleanArray-">dip::FiniteDifference</a>) and <a href="https://diplib.org/diplib-docs/generation_test.html#dip-CreateGauss-Image-L-FloatArray-CL-UnsignedArray--dfloat--UnsignedArray--String-CL">dip::CreateGauss</a>.

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

function out = derivative(varargin)
out = dip_filtering('derivative',varargin{:});
