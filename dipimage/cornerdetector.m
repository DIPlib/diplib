%CORNERDETECTOR   Corner detector
%
% SYNOPSIS:
%  error = cornerdetector(image_in,method,sigmas,parameter)
%
% PARAMETERS:
%  method:     Method to use. One of:
%    - 'Harris':     Harris corner detector. PARAMETER is the kappa.
%    - 'ShiTomasi':  Shi-Tomasi corner detector.
%    - 'Noble':      Noble's corner detector.
%    - 'WangBrady':  Wang-Brady corner detector. PARAMETER is the threshold.
%  sigmas:     The sigmas for the Gaussian smoothing applied.
%  parameter:  The additional parameter used in the method, some methods
%              don't use this.
%
% DEFAULTS:
%  method = 'ShiTomasi'
%  sigmas = 2
%  parameter = 0.04 for 'Harris' and 0.1 for 'WangBrady'.
%
% DIPlib:
%  This function calls the DIPlib functions dip::HarrisCornerDetector,
%  dip::ShiTomasiCornerDetector, dip::NobleCornerDetector,
%  dip::WangBradyCornerDetector.

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

function out = cornerdetector(varargin)
out = dip_segmentation('cornerdetector',varargin{:});
