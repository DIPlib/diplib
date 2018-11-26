%SKEW   Geometric transformation
%
% SYNOPSIS:
%  image_out = skew(image_in,shear,skew,axis,interpolation_method,boundary_condition)
%
% PARAMETERS:
%  shear: Angle (in radian) to skew over
%  skew:  Dimension along which to skew
%  axis:  Dimension as function of which the skew is performed
%  interpolation_method: One of the following strings:
%                        - '3-cubic' or '': third order cubic spline interpolation
%                        - '4-cubic': fourth order cubic spline interpolation
%                        - 'linear': linear interpolation
%                        - 'nearest' or 'nn': nearest neighbor interpolation
%                        - 'bspline': B-spline interpolation
%                        - 'ft': interpolation through Fourier domain (not yet implemented)
%                        - 'lanczos8': Lanczos interpolation with a = 8
%                        - 'lanczos6': Lanczos interpolation with a = 6
%                        - 'lanczos4': Lanczos interpolation with a = 4
%                        - 'lanczos3': Lanczos interpolation with a = 3
%                        - 'lanczos2': Lanczos interpolation with a = 2
%  boundary_condition: Defines how the boundary of the image is handled.
%                      See HELP BOUNDARY_CONDITION
%
% DEFAULTS:
%  axis = 1, except when SKEW is 1, then it's 2
%  interpolation_method = '3-cubic'
%  boundary_condition = {} (equivalent to 'mirror')
%
% The image is skewed such that a straight line along dimension AXIS is tilted by an
% angle of SHEAR in the direction of dimension SKEW. Each image line along dimension
% SKEW is shifted by a different amount. The output image has the same dimensions as
% IMAGE_IN, except for dimension SKEW, which will be larger.
%
% If BOUNDARY_CONDITION is 'periodic', a periodic skew is applied. This means that
% image lines are shifted using a periodic boundary condition, and wrap around. The
% output image does not grow along dimension SKEW.
%
% SEE ALSO:
%  shift, resample
%
% DIPlib:
%  This function calls the DIPlib function dip::Skew.

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

function out = skew(varargin)
out = dip_geometry('skew',varargin{:});
