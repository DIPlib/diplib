%RESAMPLE   Shift and a scale an image using interpolation
%
% SYNOPSIS:
%  image_out = resample(image_in,zoom,shift,interpolation_method,boundary_condition)
%
% PARAMETERS:
%  zoom: array containing a zoom for each dimension
%  shift: array containing a shift for each dimension
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
% MATHEMATICAL OPERATION:
%  With ZOOM and SHIFT as vectors, and POS the position vector:
%     image_out(pos) = image_in(pos/zoom-shift);
%
% DEFAULTS:
%  zoom = 2
%  shift = 0
%  interpolation_method = ''
%  boundary_condition = ''
%
% NOTE:
%  By default, the output array will be of a floating-point type, unless the
%  interpolation method is set to 'nearest' (or 'nn'), in which case it will
%  have the same type as the input image.
%
%  The preference 'KeepDataType', when set to on, causes the output image to
%  always have the same type as the input image. Note that his might cause
%  clipping or rounding of interpolated values.
%
% EXAMPLE:
%  a = readim;
%  b = resample(a,[.5,1],[-40,0])
%  c = readim('chromo3d');
%  d = resample(c,[1.2,1,2],[0,30,0])
%
% SEE ALSO:
%  shift, subsample, skew
%
% DIPlib:
%  This function calls the DIPlib function <a href="https://diplib.org/diplib-docs/geometry.html#dip-Resampling-Image-CL-Image-L-FloatArray--FloatArray--String-CL-StringArray-CL">dip::Resampling</a>.

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

function out = resample(varargin)
out = dip_geometry('resample',varargin{:});
