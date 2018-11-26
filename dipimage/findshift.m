%FINDSHIFT  Finds shift between two images
%
% SYNOPSIS:
%  shiftvector = findshift(in1, in2, method, parameter, maxshift)
%
% PARAMETERS:
%  method: String, one of:
%     'integer only': find integer shift only (nD)
%     'cc': cross-correlation with sub-pixel location of peak (nD)
%     'ncc': normalized cross-correlation with sub-pixel location (nD)
%     'pc': phase correlation with sub-pixel location (nD)
%     'cpf' or 'ffts': phase of normalized cross-correlation (2D only)
%     'mts' or 'grs': gradient based (first order Taylor, 1D-3D)
%     'iter': very accurate (iterative 'mts', 1D-3D)
%     'proj': fast, good for high SNR ('iter' on projections, nD)
%  parameter: Scalar parameter:
%     - For 'cpf': Sets the amount of frequencies used in this estimation.
%       The maximum value that makes sense is sqrt(1/2). Choose smaller values
%       to ignore more high frequencies. The default value is 0.2.
%     - For 'mts', 'iter', 'proj': Sigma for the Gaussian smoothing. Defaults
%       to 1.
%     - For other methods: ignored.
%  maxshift: Array, for the integer shift estimation only shifts up to this
%     range are considered. Values can be larger when using subpixel
%     refinement. Useful for periodic structures
%
% DEFAULTS:
%  method = 'integer only'
%  parameter = 0 (meaning use the default value for the method)
%  maxshift = [] (meaning no range limitation)
%
% EXAMPLE:
%  a = readim('erika');
%  b = shift(a,[-1.4 3.75]);
%  sv1 = findshift(a,b)
%  sv2 = findshift(a,b,'iter')
%  sb = shift(b,-sv2);
%  joinchannels('RGB',a,b)
%  joinchannels('RGB',a,sb)
%
% LITERATURE:
%  - C.L. Luengo Hendriks, "Improved Resolution in Infrared Imaging Using
%    Randomly Shifted Images", M.Sc. Thesis, Delft University of Technology,
%    The Netherlands, 1998.
%  - T.Q. Pham, M. Bezuijen, L.J. van Vliet, K. Schutte, and C.L. Luengo
%    Hendriks, Performance of optimal registration estimators, in: Visual
%    Information Processing XIV, Proc. SPIE, vol. 5817, 2005, p.133-144.
%
% SEE ALSO:
%  shift, affine_trans
%
% DIPlib:
%  This function calls the DIPlib function dip::FindShift.

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

function out = findshift(varargin)
out = dip_geometry('findshift',varargin{:});
