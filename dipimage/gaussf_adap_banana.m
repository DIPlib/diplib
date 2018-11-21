%GAUSSF_ADAP_BANANA   Adaptive Gaussian filtering in banana like neighborhood in 2D
%
% SYNOPSIS:
%  out = gaussf_adap_banana(in,orien_im,curv_im,sigmas,order,exponents,truncation)
%  out = gaussf_adap_banana(in,orien_im,curv_im,scale_im,sigmas,order,exponents,truncation)
%  out = gaussf_adap_banana(in,parameter_im,sigmas,order,exponents,truncation)
%
% PARAMETERS:
%  orien_im:   Angle of the orientation at each pixel
%  curv_im:    Curvature at each pixel
%  scale_im:   (optional) A tensor image with the local kernel scale
%  parameter_im: A cell array combining the two or three images above
%  sigmas:     Array containing the sigmas for the derivatives: The first
%              value is along the contour, the second perpendicular to it
%              For sigma == 0, no convolution is done is this direction
%  order:      Derivative order
%  exponents:  Moments
%  truncation: Determines the size of the Gaussian filter
%
% DEFAULTS:
%  parameter_im = [] means computed orientation and curvature
%  sigmas = [2,0]
%  order = 0
%  exponents = 0
%  truncation = 2
%
% EXAMPLE:
%  a = readim;
%  [p,c] = structuretensor(a,1,3,{'orientation','curvature'});
%  d = gaussf_adap_banana(a,p,c,[2,0])
%
% LITERATURE:
%  P. Bakker, Image structure analysis for seismic interpretation,
%   PhD Thesis, TU Delft, The Netherlands, 2001
%  L. Haglund, Adaptive Mulitdimensional Filtering,
%   PhD Thesis, Linkoping University, Sweden, 1992
%  W.T. Freeman, Steerable Filters and Local Analysis of Image Structure,
%   PhD Thesis, MIT, USA, 1992
%
% SEE ALSO: gaussf_adap, percf_adap, percf_adap_banana

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

function out = gaussf_adap_banana(varargin)
out = dip_filtering('gaussf_adap_banana',varargin{:});
