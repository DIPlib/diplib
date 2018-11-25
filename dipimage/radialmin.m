%RADIALMIN   Computes the minimum as a function of the R-coordinate
%
% SYNOPSIS:
%  image_out = radialmin(image_in,mask,binSize,maxRadius,center)
%  image_out = radialmin(image_in,binSize,maxRadius,center)
%
% PARAMETERS:
%  image_in:    The input image
%  mask:        Binary mask image (optional)
%  binSize:     The size of the radial bins
%  maxRadius:   The maximum radius to use: 'inner radius' or 'outer radius'
%               Alternatively, TRUE instead of 'inner radius' or FALSE instead
%               of 'outer radius'.
%  center:      Coordinates of the center. Alternatively, 'right', 'left', 'true'
%               or 'corner'.
%
% DEFAULTS:
%  binSize = 1
%  maxRadius = 'outer radius'
%  center = 'right'
%
% NOTES:
%  The default center ('right') is the same one as defined by the Fourier
%  Transform. That is, on even size, it is to the right of the true center.
%  This is also the default for functions like RR.
%  'left' is left of the true center for even sized images.
%  'true' is the true center, halfway between pixels for even sized images.
%  'corner' is the first pixel of the image, at coordinates [0,0,...].
%
%  'outer radius' selects the maximum radius of the output such that every
%  pixel in the image fits in the projection. 'inner radius' sets it to
%  the nearest edge to the center.
%
% DIPlib:
%  This function calls the DIPlib function dip::RadialMinimum.

% (c)2018, Cris Luengo.
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

function out = radialmin(varargin)
out = dip_math('radialmin',varargin{:});
