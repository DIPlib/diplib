%GAUSSIANLINECLIP   Clips/maps grey-values to produce a Gaussian line
%
% SYNOPSIS:
%  image_out = gaussianlineclip(image_in,sigma,normalisetoone,truncation)
%
% DEFAULTS:
%  sigma =  1
%  normalisetoone = false
%  truncation = 3
%
% DESCRIPTION:
%  The input is assumed to represent a running coordinate, as produced
%  by the functions XX, YY, RR, et cetera. This function clips
%  the coordinate values around zero to produce a Gaussian line
%  using the following recipe:
%
%  image_out = (1/(sqrt(2*pi)*sigma))*exp(-0.5*image_in^2/sigma^2)
%
%  Where abs(image_in) is larger than truncation*sigma, the above formula is
%  not computed, setting those points to 0. This avoids a lot of
%  computation. Set truncation to Inf to avoid this.
%
%  By default the peak value of the line is 1/(sqrt(2*pi)*sigma). The peak
%  value is set to one if the parameter 'normalisetoone' is set to TRUE.
%
% EXAMPLE:
%  a = gaussianlineclip(rr-40)
%
% DIPlib:
%  This function calls the DIPlib function dip::GaussianLineClip.

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

function out = gaussianlineclip(varargin)
out = dip_generation('gaussianlineclip',varargin{:});
