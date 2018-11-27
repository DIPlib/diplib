%CANNY   Canny edge detector
%
% SYNOPSIS:
%  image_out = canny(image_in,sigmas,lower,upper,selection)
%
% PARAMETERS:
%  sigmas: sigmas of Gaussian derivatives
%  lower:  lower threshold multiplier (between 0 and 1)
%  upper:  upper threshold fraction (between 0 and 1)
%  selection: one of the following strings:
%     - 'all': UPPER is a fraction of all pixels
%     - 'nonzero': UPPER is a fraction of non-zero pixels
%     - 'absolute': UPPER is a threshold value
%
% DEFAULTS:
%  sigmas = 1
%  lower = 0.5
%  upper = 0.9
%  selection = 'all'
%
% NOTE:
%  If SELECTION is 'all' (default), then a threshold T1 is computed as
%  PERCENTILE(NMS,100*(1-UPPER)), with NMS the result of the non-maximum
%  suppression of IMAGE_IN. If SELECTION is 'nonzero', then T1 is computed
%  as PERCENTILE(NMS,100*(1-UPPER),NMS>0). In both cases, T2 is set to
%  T1*LOWER.
%  If SELECTION is 'absolute', then T1 is set to UPPER directly, and T2 to
%  LOWER.
%  T1 and T2 are then used in the two thresholds in the hysteresis
%  threshold.
%
% LITERATURE:
%  J. Canny, A Computational Approach to Edge Detection,
%  IEEE Transactions on Pattern Analysis and Machine Intelligence,
%  8(6):679-697, 1986.
%
% DIPlib:
%  This function calls the DIPlib function dip::Canny.

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

function out = canny(varargin)
out = dip_segmentation('canny',varargin{:});
