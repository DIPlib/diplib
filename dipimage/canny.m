%CANNY   Canny edge detector
%
% SYNOPSIS:
%  image_out = canny(image_in,sigmas,lower,upper)
%
% PARAMETERS:
%  sigmas: sigmas of Gaussian derivatives
%  lower:  lower threshold fraction (between 0 and 1)
%  upper:  upper threshold fraction (between 0 and 1)
%
% DEFAULTS:
%  sigmas = 1
%  lower = 0.5
%  upper = 0.9
%
% LITERATURE:
%  J. Canny, A Computational Approach to Edge Detection,
%  IEEE Transactions on Pattern Analysis and Machine Intelligence,
%  8(6):679-697, 1986.
%
% DIPlib:
%  This function calls the DIPlib function dip::Canny.

% (c)2017, Cris Luengo.
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
