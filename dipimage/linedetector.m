%LINEDETECTOR   Line detector
%
% SYNOPSIS:
%  error = linedetector(image_in,method,scale,parameter,polarity)
%
% PARAMETERS:
%  method:     Method to use. One of:
%    - 'Frangi':     Frangi vesselness, 2D and 3D
%    - 'Danielsson': Danielsson's Hessian-based detector, 2D and 3D
%    - 'Matched':    Matched filters, 2D only
%    - 'RORPO':      Based on path openings
%  scale:      The scale parameter.
%  parameter:  The additional parameter(s) used in the method, or none.
%  polarity:   Either 'black' or 'white', indicating the color of lines to
%              find.
%
% DEFAULTS:
%  method = 'Frangi'
%  scale = (see method description)
%  parameter = (see method description)
%  polarity = 'white'
%
% METHOD DESCRIPTION:
%  'Frangi': SCALE is the sigmas for the Gaussian derivatives used to compute
%  the Hessian matrix, and defaults to 2. PARAMETER are the thresholds applied
%  to each component of the vesselness measure, and defaults to [0.5, 15] for
%  2D, and [0.5, 0.5, 500] for 3D.
%
%  'Danielsson': SCALE is the sigmas for the Gaussian derivatives used to
%  compute the Hessian matrix, and defaults to 2. PARAMETER is not used.
%
%  'Matched': SCALE is the sigma (scalar) for the Gaussian (goes perpendicular
%  to the line) and PARAMETER is the length (scalar) for the filter (goes
%  along the line). These defualt to 2 and 10, respectively.
%
%  'RORPO': SCALE is the length of the path opening operator, it must be an
%  integer larger than 1. SCALE defaults to 15. PARAMETER is not used.
%
% DIPlib:
%  This function calls the DIPlib functions dip::FrangiVesselness,
%  dip::DanielssonLineDetector, dip::MatchedFiltersLineDetector2D,
%  dip::RORPOLineDetector.

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

function out = linedetector(varargin)
out = dip_segmentation('linedetector',varargin{:});
