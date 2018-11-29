%GAUSSIANEDGECLIP   Clips/maps grey-values to produce a Gaussian edge
%
% SYNOPSIS:
%  image_out = gaussianedgeclip(image_in,sigma,truncation)
%
% DEFAULTS:
%  sigma =  1
%  truncation = 3
%
% DESCRIPTION:
%  The input is assumed to represent a running coordinate, as produced
%  by the functions XX, YY, RR, et cetera. This function clips
%  the coordinate values around zero to produce a Gaussian edge
%  using the following recipe:
%
%  image_out = 0.5+0.5*erf(image_in/(sqrt(2)*sigma))
%
%  Where abs(image_in) is larger than truncation*sigma, the above formula is
%  not computed, setting those points to 0 or 1. This avoids a lot of
%  computation. Set truncation to Inf to avoid this.
%
% EXAMPLE:
%  a = gaussianedgeclip(40-rr)
%
% DIPlib:
%  This function calls the DIPlib function dip::GaussianEdgeClip.

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

function out = gaussianedgeclip(varargin)
out = dip_generation('gaussianedgeclip',varargin{:});
