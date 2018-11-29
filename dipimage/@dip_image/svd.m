%SVD Singular value decomposition.
%  [U,S,V] = SVD(X) 
%
%  S = SVD(X) returns a vector image containing the singular values.
%
%  Computes the "economy size" decomposition. If X is m-by-n with m >= n,
%  then only the first n columns of U are computed and S is n-by-n.
%  For m < n, only the first m columns of V are computed and S is m-by-m.
%
%  See also: DIP_IMAGE/EIG, EIG_LARGEST, DIP_IMAGE/PINV, DIP_IMAGE/RANK

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

function varargout = svd(varargin)
varargout = cell(1,max(nargout,1));
[varargout{:}] = dip_eig_svd('svd',varargin{:});
