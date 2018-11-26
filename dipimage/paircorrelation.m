%PAIRCORRELATION   Computes the pair correlation of the phases in a labeled image
%
% SYNOPSIS:
%  distribution = paircorrelation(image_in,mask,probes,length,estimator,options)
%  distribution = paircorrelation(image_in,mask,probes,length,estimator,...
%              covariance,normalisation)
%
% PARAMETERS:
%  image_in:   defines the phases on which the pair correlation function is computed.
%              image_in can be a binary or labeled image, or it can be a float image.
%  mask:       mask image to select the regions in image_in that are used to compute the
%              correlation function.
%  probes:     the number of random pairs that are generated to compute the correlation
%              function.
%  length:     the maximum length (in pixels) of the correlation function.
%  estimator:  type of correlation estimator used: 'random' or 'grid'.
%  options:    cell array of strings: 'covariance', and/or 'volume_fraction' or
%              'volume_fraction^2', combines the following two parameters.
%  covariance: whether or not to compute the covariance function (else it is correlation).
%  normalisation: type of normalisation to be used on the correlation function:
%                 'none', 'volume_fraction', or 'volume_fraction^2'
%
% EXPLANATION:
%  The pair correlation is the probability that two points at a distance D are in
%  the same phase, as a function of D. D is varied from 0 to LENGTH.
%  The function is estimated by probing PROBES pairs in the image. If the estimator
%  is 'random', pairs are selected randomly. If the estimator is 'grid', pairs are
%  selected using a grid.
%
%  The output DISTRIBUTION is an array where the first column is the distance D, and
%  subsequent columns represent the different components of the pair correlation.
%  If COVARIANCE is true, there are N^2 components, otherwise there are N components.
%  N is the number of phases in IMAGE_IN.
%
%  If IMAGE_IN is labeled, each label (including label 0) is a phase. The image must be
%  scalar. If it is binary, it is interpreted as a labeled image with two phases.
%  If IMAGE_IN is a floating-point image, it is interpreted as represeting the probability
%  of a phase being present in each pixel. Each tensor component represents one phase.
%  Pixel values are expected to be in the range [0,1].
%
% DEFAULTS:
%  mask = []
%  probes = 1000000
%  length = 100
%  estimator = 'random'
%  options = {} (equivalent to covariance = 'no', normalisation = 'none')
%
% DIPlib:
%  This function calls the DIPlib functions dip::PairCorrelation and
%  dip::ProbabilisticPairCorrelation.

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

function out = paircorrelation(varargin)
out = dip_analysis('paircorrelation',varargin{:});
