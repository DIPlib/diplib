%LOGGABOR   Log-Gabor filter bank
%
% SYNOPSIS:
%  image_out = loggabor(image_in,wavelengths,bandwidth,nOrientations,...
%                       inRepresentation,outRepresentation)
%
% PARAMETERS:
%  wavelengths:       Frequency scales, selected as wavelengths in pixels
%  bandwidth:         Width of each frequency scale, with 0.75, 0.55 and
%                     0.41 corresponding approximately 1, 2 and 3 octaves
%  nOrientations:     Number of orientations
%  inRepresentation:  One of 'spatial' or 'frequency'
%  outRepresentation: One of 'spatial' or 'frequency'
%
% DEFAULTS:
%  wavelengths = [3.0, 6.0, 12.0, 24.0]
%  bandwidth = 0.75
%  nOrientations = 6
%  inRepresentation = 'spatial'
%  outRepresentation = 'spatial'
%
% NOTES:
%  IMAGE_IN can be a vector of image sizes, the function will behave
%  as if the input image were DELTAIM(IMAGE_IN).
%
%  IMAGE_OUT is a tensor image with nOrientations x length(wavelengths)
%  tensor components. Its data type will be either SFLOAT or SCOMPLEX
%  depending on the input parameters.
%
%  See the DIPlib documentation for more details on this function.
%
% DIPlib:
%  This function calls the DIPlib function dip::LogGaborFilterBank.

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

function out = loggabor(varargin)
out = filtering('loggabor',varargin{:});
