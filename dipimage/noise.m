%NOISE   Add noise to an image
%
% SYNOPSIS:
%  image_out = noise(image_in, noise_type, parameter1, parameter2)
%
% PARAMETERS:
%  noise_type: 'gaussian', 'uniform', 'poisson', 'binary',
%              'saltpepper', 'brownian', 'pink', 'blue', 'violet'.
%  The parameters vary for the differnt noise types:
%    Gaussian:
%      parameter1 = standard deviation of the noise
%    Uniform:
%      parameter1 = lower bound
%      parameter2 = upper bound
%    Poisson:
%      parameter1 = conversion
%        The intensity of the input image multiplied by the conversion
%        variable is used as mean value for the Poisson distribution.
%        The conversion factor can be used to relate the pixel values
%        with the number of counts.
%    Binary:
%      parameter1 = probability for a 1->0 transition
%      parameter2 = probability for a 0->1 transition
%        IMAGE_IN must be binary.
%    Salt & pepper:
%      parameter1 = probability for a pixel to become 255
%      parameter2 = probability for a pixel to become 0
%    Brownian:
%      parameter1 = standard deviation of the noise
%        The power spectrum of the noise is 1/f^2.
%    Pink:
%      parameter1 = standard deviation of the noise
%      parameter2 = 1 for pink noise, 2 for Brownian noise.
%        The power spectrum of the noise is 1/(f^parameter2).
%        If parameter2<=0, it is taken as 1, so that the default
%        value creates pink noise.
%    Blue:
%      parameter1 = standard deviation of the noise
%      parameter2 = 1 for blue noise, 2 for violet noise.
%        The power spectrum of the noise is f^parameter2.
%        If parameter2<=0, it is taken as 1, so that the default
%        value creates blue noise.
%    Violet:
%      parameter1 = standard deviation of the noise
%        The power spectrum of the noise is f^2.
%
% DEFAULTS:
%  noise_type = 'gaussian'
%  parameter1  = 1
%  parameter2  = 0
%
% DIPlib:
%  This function calls the DIPlib functions dip::UniformNoise,
%  dip::GaussianNoise, dip::PoissonNoise, dip::BinaryNoise,
%  dip::ColoredNoise.

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

function out = noise(varargin)
out = dip_generation('noise',varargin{:});
