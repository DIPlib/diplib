%NONMAXIMUMSUPPRESSION   Non-maximum suppression
%
% SYNOPSIS:
%  image_out = nonmaximumsuppression(image_in,gradient,mask,mode)
%
% PARAMETERS:
%  image_in: A scalar image. Typically the gradient magnitude.
%  gradient: A vector image of the same size as IMAGE_IN, indicating the
%            direction along which local maxima are examined.
%  mask:     Binary image to select where to apply the filter.
%  mode:     'interpolate' or 'round'.
%
%  The filter kernel can be specified in two ways: through FILTERSIZE
%  and FILTERSHAPE, specifying one of the default shapes, or through NEIGHBORHOOD,
%  providing a custom binary shape.
%
% DEFAULTS:
%  mask = []
%  mode = 'interpolate'
%
% NOTES:
%  If IMAGE_IN is [], the norm of GRADIENT is used.
%
%  GRADIENT must be an n-vector image with n dimensions. That is, it must have
%  the same number of tensor elements as spatial dimensions.
%
%  IMAGE_IN and GRADIENT must both be of the same floating-point type. That is,
%  either they both are SFLOAT or they both are DFLOAT.
%
%  If MASK is given, must be of the same sizes as IMAGE_IN.
%
%  MODE is only considered for 2D images. For other dimensionalities, 'round'
%  is always used. The 'round' mode rounds the gradient direction to point to
%  one of the direct neighbors. 'interpolate' uses interpolation to evaluate
%  the shape of the function along the gradient direction.
%
% DIPlib:
%  This function calls the DIPlib function dip::Kuwahara.

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

function out = nonmaximumsuppression(varargin)
out = dip_filtering('nonmaximumsuppression',varargin{:});
