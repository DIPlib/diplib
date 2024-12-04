%ORIENTATIONSPACE   Orientation space
%
% SYNOPSIS:
%  image_out = orientationspace(image_in,order,radCenter,radSigma,orientations)
%
% PARAMETERS:
%  image_in:     a 2D real-valued scalar image.
%  order:        angular selectivity, larger order means more selective.
%  radCenter:    overall scaling, larger value is smaller filter.
%  radSigma:     bandwidth, smaller value means more selective.
%  orientations: if larger than 0, the number of orientations computed.
%
% DEFAULTS:
%  order = 8
%  radCenter = 0.1
%  radSigma = 0.8
%  orientations = 0 (== 2*order+1)
%
% LITERATURE:
%  - M. van Ginkel, P.W. Verbeek and L.J. van Vliet, "Improved Orientation
%    Selectivity for Orientation Estimation", Proceedings 10th Scandinavian
%    Conference on Image Analysis, pp 533-537, Pattern Recognition Society
%    of Finland, 1997.
%  - M. van Ginkel, "Image Analysis using Orientation Space based on
%    Steerable Filters", PhD Thesis, Delft University of Technology, The
%    Netherlands, 2002.
%
% DIPlib:
%  This function calls the DIPlib function <a href="https://diplib.org/diplib-docs/analysis.html#dip-OrientationSpace-Image-CL-Image-L-dip-uint--dfloat--dfloat--dip-uint-">dip::OrientationSpace</a>.

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

function out = orientationspace(varargin)
out = dip_analysis('orientationspace',varargin{:});
