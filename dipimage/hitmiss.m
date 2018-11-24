%HITMISS   Hit-and-miss transform -- morphological template matching
%
% SYNOPSIS:
%  image_out = hitmiss(image_in,hit,miss,mode,boundary_condition)
%  image_out = hitmiss(image_in,se,mode,boundary_condition)
%
% PARAMETERS:
%  hit:  Image with the foreground mask
%  miss: Image with the background mask
%  se:   Image with the hit-and-miss mask, translated as HIT = SE==1,
%        and MISS = SE==0. Any other value is considered a "don't care".
%  mode: Defines the definition used for grey-value images, see below.
%  boundary_condition: Defines how the boundary of the image is handled.
%                      See HELP BOUNDARY_CONDITION
%
% DEFAULTS:
%  mode = 'unconstrained'
%  bounary_condition = 'mirror'
%
% NOTES:
%  For binary images, MODE is ignored. The definition is always the
%  same:
%     erosion(in,hit) & ~dilation(in,miss)
%
%  For grey-value images, the definition changes according to MODE
%
%  MODE=='unconstrained': this is the simplest definition:
%     erosion(in,hit) - dilation(in,miss) , if the result is positive
%     0                                   , otherwise
%
%  MODE=='constrained':
%     in - dilation(in,miss) , if the result is positive and in == erosion(in,hit)
%     erosion(in,hit) - in   , if the result is positive and in == dilation(in,miss)
%     0                      , otherwise
%
% DIPlib:
%  This function calls the DIPlib function dip::HitAndMiss.

% (c)2017-2018, Cris Luengo.
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

function out = hitmiss(varargin)
out = dip_morphology('hitmiss',varargin{:});
