%JACOBI   Eigenvalues and vectors of a tensor image
%  This function calls DIP_IMAGE/EIG.
%  It exists only for backwards-compatibility.
%
% SYNOPSIS:
%  [lambda, ev] = jacobi(in)
%
%  in:     a tensor image, where the elements may be images of any
%          dimension
%  lambda: the sorted eigenvalues in a vector image, out{1} the largest,
%          out{n} the smallest
%  ev:     matrix image containing the eigenvectors, ev{:,1} is the first
%          eigenvector

% (c)2017, Cris Luengo.
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

function [lambda,ev] = jacobi(in)
if nargout==2
   [ev,lambda] = eig(in);
   lambda = diag(lambda);
else
   lambda = eig(in);
end
