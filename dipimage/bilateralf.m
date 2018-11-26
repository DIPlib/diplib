%BILATERALF   Bilateral filter with different implementations
%
% SYNOPSIS:
%  image_out = bilateralf(image_in,spatial_sigma,tonal_sigma,truncation,method,boundary_condition)
%
% PARAMETERS:
%  spatial_sigma: sigma of the Gaussian spatial weight
%  tonal_sigma:   sigma of the Gaussian tonal weight
%  truncation:    at how many sigma to truncate the Gaussians
%  method:        one of 'full', 'xysep', 'uvsep', 'arc', 'pwlinear'
%  boundary_condition: Defines how the boundary of the image is handled.
%                      See HELP BOUNDARY_CONDITION
%
% DEFAULTS:
%  spatialSigma = 2
%  tonalSigma = 30
%  truncation = 2
%  method = 'xysep'
%  boundary_condition = 'mirror'
%
% EXPLANATION OF METHODS
%  'full' is de brute-force implementation.
%
%  'xysep' implements a 1D brute-force bilateral filter along each of the
%  image dimensions in sequence. This yields an approximation to the 'full'
%  result (Pham and van Vliet).
%
%  'pwlinear' uses a piece-wise linear approximation to the bilateral
%  filter, is fast for larger spatial sigmas (Durand and Dorsey).
%
%  'uvsep' and 'arc' haven't been implemented yet.
%
% LITERATURE:
%  C. Tomasi and R. Manduchi, "Bilateral filtering for Gray and Color Images," Proceedings of the 1998 IEEE
%    International Conference on Computer Vision, Bombay, India.
%  T.Q. Pham and L.J. van Vliet, "Separable bilateral filter for fast video processing," IEEE International
%    Conference on Multimedia and Expo, 2005.
%  F. Durand and J. Dorsey, "Fast bilateral filtering for the display of high-dynamic-range images,"
%    ACM Transactions on Graphics 21(3), 2002.
%
% SEE ALSO:
%  arcf, pmd
%
% DIPlib:
%  This function calls the DIPlib function dip::BilateralFilter

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

function out = bilateralf(varargin)
out = dip_filtering('bilateralf',varargin{:});
