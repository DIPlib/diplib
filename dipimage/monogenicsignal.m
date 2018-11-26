%MONOGENICSIGNAL   Computes the monogenic signal, phase congruency and phase symmetry
%
% SYNOPSIS:
%  varargout = monogenicsignal(image_in,wavelengths,bandwidth,outputs,...
%                              noiseThreshold,frequencySpreadThreshold,...
%                              sigmoidParameter,deviationGain,polarity)
%
% PARAMETERS:
% Affecting the creation of the monogenic signal:
%  wavelengths:       Frequency scales, selected as wavelengths in pixels
%  bandwidth:         Width of each frequency scale, with 0.75, 0.55 and
%                     0.41 corresponding approximately 1, 2 and 3 octaves
% Affecting what is computed:
%  outputs:           Which outputs to produce, see below
% Affecting both congruency and symmetry computation:
%  noiseThreshold:    Noise threshold to use on the energy images
% Affecting congruency computation:
%  frequencySpreadThreshold:
%  sigmoidParameter:
%  deviationGain:
% Affecting symmetry computation:
%  polarity:          One of 'white', 'black' or 'both'
%
% DEFAULTS:
%  wavelengths = [3.0, 24.0]
%  bandwidth = 0.41
%  outputs = {}
%  noiseThreshold = 0.2
%  frequencySpreadThreshold = 0.5
%  sigmoidParameter = 10
%  deviationGain = 1.5
%  polarity = 'both'
%
% OUTPUTS:
%  The OUTPUTS argument determines what outputs the function produces. If it
%  is an empty array, the monogenic signal itself will be returned. This works
%  for images with any number of dimensions. Otherwise, a single string or a
%  cell array with strings is expected. The function will produce as many
%  outputs as elements in the string, and in the same order. The strings are
%  as follows:
%   - 'congruency': Phase congruency, a measure of line/edge significace
%   - 'symmetry': Phase symmetry, a constrast invariant measure of symmetry
%   - 'orientation': Line/edge orientation (in the range [-pi/2, pi/2]), 2D only
%   - 'phase': Phase angle (pi/2 is a white line, 0 is an edge, -pi/2 is a black line)
%   - 'energy': Raw phase congruency energy
%   - 'symenergy': Raw phase symmetry energy
%  'energy' is a by-products of the computation of the phase congruency. 'symenergy'
%  is a by-product of the computation of the phase symmetry.
%
% NOTES:
%  'congruency' is computed using Kovesi's method if there are more than two
%  WAVELENGTHS given, and Felberg's method if exactly two WAVELENGHTS are given.
%  Kovesi's method works only in 2D.
%
% EXAMPLE:
%  a = newim+255;
%  a = drawshape(a,200,[128,128],'circle',-255*sqrt(2*pi),1);
%  a = drawshape(a,160,[128,128],'disk',-255,1);
%  a = drawshape(a,120,[128,128],'circle',255*sqrt(2*pi),1);
%  a = drawshape(a,80,[128,128],'disk',255,1)
%  b = monogenicsignal(a,[3,24],0.41,'congruency');
%  dipshow(b,'unit')
%
% DIPlib:
%  This function calls the DIPlib function dip::MonogenicSignal, and
%  dip::MonogenicSignalAnalysis.

% (c)2018, Cris Luengo.
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

function out = monogenicsignal(varargin)
out = dip_analysis('monogenicsignal',varargin{:});
