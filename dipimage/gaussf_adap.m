%GAUSSF_ADAP   Adaptive Gaussian filtering for 2D and 3D images
%  
% SYNOPSIS:
%  out = gaussf_adap(in,parameter_im,sigmas,order,exponents,truncation)
%
% PARAMETERS:
%  parameter_im: Cell array of images containing:
%      - 2D:
%          - parameter_im{1} is the angle of the orientation
%          - parameter_im{2} (optional) is a tensor image with the local kernel scale
%      - 3D:
%          - parameter_im{1} is the polar coordinate phi of the first orientation
%          - parameter_im{2} is the polar coordinate theta of the first orientation
%          - parameter_im{3} (optional) is the polar coordinate phi of the second orientation
%          - parameter_im{4} (optional) is the polar coordinate theta of the second orientation
%          - parameter_im{3} or {5} (optional) is a tensor image with the local kernel scale
%  sigmas:     Array containing the sigmas for the derivatives:
%      - 2D: The first value is along the contour, the second perpendicular to it
%      - 3D: For intrinsic 2D structures, usage: [0,sg1,sg2]
%            For intrinsic 1D structures, usage: [0,0,sg]
%              For sigma == 0, no convolution is done is this direction
%  order:      Derivative order
%  exponents:  Moments
%  truncation: Determines the size of the Gaussian filter
%
% DEFAULTS:
%  parameter_im = [] means computed orientation along a line
%  sigmas = [2,0] for 2D images and [0,0,2] for 3D images
%  order = 0
%  exponents = 0
%  truncation = 2
%
% EXAMPLES:
%  For a 2D image:
%   a = readim;
%   p = structuretensor(a,1,3,{'orientation'});
%   b = gaussf_adap(a,p,[2,0])
%
%  Smoothing along a 1D line-like object in a 3D image
%   a = readim('chromo3d');
%   [p3,t3] = structuretensor(a,1,[3 3 1],{'phi3','theta3'});
%   b = gaussf_adap(a,{p3,t3},[0,0,2])
%
%  Smoothing in a 2D plane in 3D
%   [p2,t2,p3,t3] = structuretensor(a,1,[3 3 1],{'phi2','theta2','phi3','theta3'});
%   b = gaussf_adap(a,{p2,t2,p3,t3},[0,2,2])
%
% LITERATURE: 
%  P. Bakker, Image structure analysis for seismic interpretation, 
%   PhD Thesis, TU Delft, The Netherlands, 2001 
%  L. Haglund, Adaptive Mulitdimensional Filtering,
%   PhD Thesis, Linkoping University, Sweden, 1992
%  W.T. Freeman, Steerable Filters and Local Analysis of Image Structure, 
%   PhD Thesis, MIT, USA, 1992
%
% SEE ALSO: gaussf_adap_banana, percf_adap, percf_adap_banana

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

function out = gaussf_adap(varargin)
out = filtering('gaussf_adap',varargin{:});
