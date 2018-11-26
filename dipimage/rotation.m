%ROTATION   Rotate an image within an orthogonal plane
%
% SYNOPSIS:
%  image_out = rotation(image_in, angle, interpolation_method, boundary_condition) % (for 2D images only)
%  image_out = rotation(image_in, angle, axis, interpolation_method, boundary_condition) % (for 3D images only)
%  image_out = rotation(image_in, angle, dimension1, dimension2, interpolation_method, boundary_condition) % (the general case)
%
% PARAMETERS:
%  angle: Angle (in radian) to rotate over
%  axis: Axis to rotate around (for 2D or 3D images only, value ignored for a 2D image).
%  dimension1: First dimension defining the orthogonal plane
%  dimension2: Second dimension defining the orthogonal plane
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
%  dimension1 = 1
%  dimension2 = dimension1 + 1  (that is: axis = 3)
%  interpolation_method = '3-cubic'
%  boundary_condition = 'add zeros'
%
% The image is rotated around its center, using three skews.
%
% NOTES:
%  Sign of the 2D rotation: implementation in the mathemetical sense, but
%  note the y-axis is positive downwards! Thus: left turning has negative
%  sign, and right positive.
%
%  For exact rotations of multiples of 90 degrees, use the rot90 method
%  instead. Note that rot90 reverses the sign of the angle as compared to
%  this function.
%
% SEE ALSO:
%  skew, affine_trans
%
% DIPlib:
%  This function calls the DIPlib function dip::Rotation.

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

function out = rotation(varargin)
out = dip_geometry('rotation',varargin{:});
