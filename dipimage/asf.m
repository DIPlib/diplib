%ASF   Alternating sequential filters, morphological smoothing
%
%  Alternating sequential filters are two morphological filters opening and
%  closing, applied in sequence, from a small size to a larger size. This
%  provides an effective smoothing that is less biased than applying an
%  opening and closing of a single size.
%
% SYNOPSIS:
%  image_out = asf(image_in,size_range,filter_shape,filter_type,polarity,boundary_condition)
%
% PARAMETERS:
%  size_range:   Sizes of the filter to apply. [start, stop, step].
%  filter_shape: 'rectangular', 'elliptic', 'diamond', 'parabolic'
%  filter_type:  'structural', 'reconstruction', 'area'. Determines which
%                filters are used: structural opening/closing, opening/closing
%                by reconstruction, or area opening/closing.
%  polarity:     'open-close' or 'close-open'. Determines the order of the filters
%  boundary_condition: Defines how the boundary of the image is handled.
%                      See HELP BOUNDARY_CONDITION
%
% DEFAULTS:
%  size_range = [3,7,2]
%  filter_shape = 'elliptic'
%  filter_type = 'structural'
%  polarity = 'open-close'
%  boundary_condition = {} (equivalent to 'add min' in the dilation and 'add max' in
%                           the erosion)
%
% DIPlib:
%  This function calls the DIPlib function dip::AlternatingSequentialFilter.

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

function out = asf(varargin)
out = dip_morphology('asf',varargin{:});
