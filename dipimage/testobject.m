%TESTOBJECT   Creates bandlimited test objects
%  The test object is in the center of the output image. It can optionally
%  be modulated using a sine function, blurred, and have noise added.
%
% SYNOPSIS:
%  image_out = testobject(object,imgSizes,objSizes,[name-value pairs])
%  image_out = testobject(image,[name-value pairs])
%
% PARAMETER:
%  object:   'ellipsoid', 'box', 'ellipsoid shell', 'box shell'.
%  imgSizes: a vector indicating sizes for the output image, or an image
%            whose sizes will be taken.
%  objSizes: a vector indicating sizes of the object.
%  image:    an image of an object that will be used as-is.
%  [name-value pairs]: see below
%
% DEFAULTS:
%  object   = 'ellipsoid'
%  imgSizes = [256,256]
%  objSizes = [128,128]
%
% NAME-VALUE PAIRS:
%  Properties of the generated image can be controlled through the following
%  name-value pairs:
%
%  Object:
%    - 'generationMethod':    'gaussian' (default) or 'fourier'. See below.
%    - 'objectAmplitude':     Scalar, determines brightness of the object.
%                             Default is 1.
%    - 'randomShift':         Boolean (true/'yes'/1/false/'no'/0) decides
%                             whether to add a random sub-pixel shift to the
%                             object. Default is false.
%
%  Cosine modulation (additive, per axis):
%    - 'modulationDepth':     Strenght of the modulation; default = 0 for no
%                             modulation.
%    - 'modulationFrequency': Array, the frequency along each image axis.
%                             Units are periods per pixel, and should be
%                             smaller than 0.5 to prevent aliasing.
%
%  Blurring:
%    - 'pointSpreadFunction': 'gaussian', 'incoherent' or 'none' (default).
%                             Adds smoothing. 'incoherent' is a 2D, in-focus
%                             diffraction limited incoherent PSF.
%    - 'oversampling':        Determines the size of the PSF. 1 (default) is
%                             the minimum for a band-limited PSF.
%
%  Noise:
%    - 'backgroundValue':     Background intensity added to the image. Default
%                             value is 0.01.
%    - 'signalNoiseRatio':    Determines the SNR, which we define as the
%                             average object energy divided by average noise
%                             power. Default is 0 (no noise added).
%    - 'gaussianNoise':       Relative amount of Gaussian noise used.
%    - 'poissonNoise':        Relative amount of Poisson noise used. Both
%                             Gaussian and Poisson amounts default to 1,
%                             meaning that the SNR is divided equally among
%                             the two noise types.
%
% NOTE:
%  The 'generationMethod' parameter controls whether the object is generated
%  in the spatial ('gaussian') or in the frequency domain ('frequency'). In
%  the spatial domain, object edges have a Gaussian profile. In the frequency
%  domain, they are truly bandlimited. With the second syntax (first input
%  argument is an image), it determines whether that image is in the spatial
%  or in the frequency domain.
%
% LIMITATIONS:
%  The 'ellipsoid' and 'ellipsoid shell' objects must be isotropic (same sizes
%  along each dimension) when generated in the spatial domain, and cannot be
%  generated with more than 3 dimensions in the frequency domain.
%
% DIPlib:
%  This function calls the DIPlib function dip::TestObject.

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

function out = testobject(varargin)
out = dip_generation('testobject',varargin{:});
