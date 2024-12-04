%BESSELJ   Bessel function of the first kind.
%  OUT = BESSELJ(NU,IN) is the Bessel function of the first kind, of order NU.
%  NU must be a non-negative integer. IN must be real-valued image.
%
%  2*BESSELJ(1,RR)/RR is the Fourier transform of a correctly sampled
%  circle.
%
%  This function calls the DIPlib function <a href="https://diplib.org/diplib-docs/math_trigonometric.html#dip-BesselJN-Image-CL-Image-L-dip-uint-">dip::BesselJN</a>.
%
%  See also: DIP_IMAGE/BESSELY

% (c)2019, Cris Luengo.
% Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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

function out = besselj(varargin)
out = dip_imagemath('besselj',varargin{:});
