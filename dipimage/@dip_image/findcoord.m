%FINDCOORD   Find coordinates of nonzero elements.
%   C = FINDCOORD(B) returns the coordinates of the image B that
%   are non-zero. C(N,:) is a vector with coordinates for non-zero
%   pixel number N. 
%
%   [C,V] = FINDCOORD(B) returns also a 1-D image containing the 
%   values of the non-zero pixels in B.
%   FINDCOORD is similar to FIND, in that it returns the same
%   list of pixels, but in a different form.
%
%   FINDCOORD(B,K) finds at most the first K nonzero pixels.
%   FINDCOORD(B,K,'first') is the same. FINDCOORD(B,K,'last') finds
%   at most the K last pixels. This syntax is only valid on
%   versions of MATLAB in which the built-in FIND supports these
%   options.
%
%   Note that the output of FINDCOORD cannot be directly used to
%   index an image. Each of the coordinates should be used separately,
%   i.e., B(C(N,1),C(N,2)), or by computing the pixel indices,
%   B(SUB2IND(B,C)).
%   SUB2IND(B,C) is the same as what is returned by the function FIND.
%
%   See also DIP_IMAGE/FIND, COORD2IMAGE, DIP_IMAGE/SUB2IND, DIP_IMAGE/IND2SUB.

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

function [C,V] = findcoord(in,varargin)
if nargout==2
   [I,V] = find(in,varargin{:});
else
   I = find(in,varargin{:});
end
C = ind2sub(in,I);
