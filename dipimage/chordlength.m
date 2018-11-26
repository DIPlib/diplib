%CHORDLENGTH   Computes the chord lengths of the phases in a labeled image
%
% SYNOPSIS:
%  distribution = chordlength(image_in,mask,probes,length,estimator)
%
% PARAMETERS:
%  image_in:   defines the phases on which the chord lengths are computed.
%              image_in can be a binary or labeled image.
%  mask:       mask image to select the regions in image_in that are used
%              to compute the chord lengths.
%  probes:     the number of lines that are used
%  length:     the maximum chord length (in pixels).
%  estimator:  type of correlation estimator used: 'random' or 'grid'
%
% DEFAULTS:
%  mask = []
%  probes = 100000
%  length = 100
%  estimator = 'random'
%
% DIPlib:
%  This function calls the DIPlib functions dip::ChordLength.

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

function out = chordlength(varargin)
out = dip_analysis('chordlength',varargin{:});
