%GAUSSIANBLOB  Adds Gauss shaped spots to an image
%
% SYNOPSIS:
%  image_out = gaussianblob(image_in,coordinates,sigma,strength,domain,truncation)
%
%  image_in:    Input image, where the spots will be set
%  image_out:   image_in + spots
%  coordinates: NxD array containing the location of the spots 
%               D = NDIMS(IMAGE_IN)
%  sigma:       The sigmas of the gaussians, 1x1, 1xD, Nx1 or NxD array
%  strength:    The integrated intensity of the spots, 1x1, 1xT, Nx1 or NxT array
%               T = 1 or NTENSORELEM(IMAGE_IN)
%  domain:      'spatial' or 'frequency'
%    spatial:   The image is in the spatial domain.
%    frequency: The image is in the frequency domain.
%               COORDINATES is in frequency domain units, in the range (-0.5,0.5).
%               SIGMA and STRENGTH are defined in spatial domain units.
%               That is, the inverse transform of the blob yields a Gaussian with
%               the chosen SIGMA and STRENGTH.
%  truncation:  The blob is computed in a region up to truncation*sigma from the
%               blob origin. For the frequency domain, the sigma is first converted
%               to equivalent frequency domain coordinates.
%
% DEFAULTS:
%  sigma = 2
%  strength = 255
%  domain = 'spatial'
%  truncation = 3
%
% DIPlib:
%  This function calls the DIPlib function dip::DrawBandlimitedPoint

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

function out = gaussianblob(varargin)
out = dip_generation('gaussianblob',varargin{:});
