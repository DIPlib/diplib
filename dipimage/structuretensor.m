%STRUCTURETENSOR   Computes the structure tensor
%
% SYNOPSIS:
%  varargout = structuretensor(image_in,dsigma,tsigma,outputs,method,...
%                              boundary_condition,truncation)
%
% PARAMETERS:
%  dsigma: Gaussian parameter for each dimension, used to compute derivatives.
%  tsigma: Gaussian parameter for each dimension, used to smooth the tensor.
%  outputs: which outputs to produce, see below.
%  method: Method used to compute the derivatives and smoothing. One of:
%    - 'gaussfir':     Finite Impulse Resonse filter (convolution with a kernel).
%    - 'gaussiir':     Infinte Impulse Response filter (recursive filtering).
%    - 'gaussft':      Convolution via a multiplication in the Fourier Domain.
%    - 'finitediff':   Finite difference derivative approximation.
%    - 'best':         Chooses the best option above for your kernel.
%  boundary_condition: Defines how the boundary of the image is handled.
%                      See HELP BOUNDARY_CONDITION.
%  truncation: Determines the size of the Gaussian filters.
%
% DEFAULTS:
%  dsigma = 1
%  tsigma = 5
%  outputs = {}
%  metod = 'best'
%  bounary_condition = 'mirror'
%  truncation = 3
%
% OUTPUTS:
%  The OUTPUTS argument determines what outputs the function produces. If it
%  is an empty array, the structure tensor itself will be returned. This works
%  for images with any number of dimensions. Otherwise, a single string or a
%  cell array with strings is expected. The function will produce as many
%  outputs as elements in the string, and in the same order. The strings are
%  as follows for a 2D input image:
%   - 'l1': The largest eigenvalue.
%   - 'l2': The smallest eigenvalue.
%   - 'orientation': Orientation. Lies in the interval (-pi/2, pi/2).
%   - 'energy': Sum of the two eigenvalues l1 and l2.
%   - 'anisotropy1': Measure for local anisotropy: ( l1 - l2 ) / ( l1 + l2 ).
%   - 'anisotropy2': Measure for local anisotropy: 1 - l2 / l1.
%   - 'curvature': Signed curvature (rate of change of local orientation).
%  Or for a 3D input image:
%   - 'l1': The largest eigenvalue.
%   - 'phi1': First component of the orientation of the eigenvector for l1.
%   - 'theta1': Second component of the orientation of the eigenvector for l1.
%   - 'l2': The middle eigenvalue.
%   - 'phi2': First component of the orientation of the eigenvector for l2.
%   - 'theta2': Second component of the orientation of the eigenvector for l2.
%   - 'l3': The smallest eigenvalue.
%   - 'phi3': First component of the orientation of the eigenvector for l3.
%   - 'theta3': Second component of the orientation of the eigenvector for l3.
%   - 'energy': Sum of the three eigenvalues l1, l2 and l3.
%   - 'cylindrical': Measure for local anisotropy: ( l2 - l3 ) / ( l2 + l3 ).
%   - 'planar': Measure for local anisotropy: ( l1 - l2 ) / ( l1 + l2 ).
%
% Note that 'anisotropy' is an alias for 'anisotropy1' for backwards compatibility.
%
% DIPlib:
%  This function calls the DIPlib functions dip::StructureTensor, and
%  dip::StructureTensorAnalysis.

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

function out = structuretensor(varargin)
out = dip_analysis('structuretensor',varargin{:});
