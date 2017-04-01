%EIG   Eigenvalues and eigenvectors of a tensor image
%  E = EIG(A) is a vector image containing the eigenvalues of the
%  square tensors in image A. The values in E are sorted in descending
%  order, so that E{1} contains the largest eigenvalue.
%
%  [V,D] = EIG(A) returns a diagonal matrix D of eigenvalues and a matrix
%  V whose columns are the corresponding eigenvectors, so that A*V = V*D.
%
%  See also: EIG_LARGEST, DIP_IMAGE/SVD

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
