%FIND   Find indices of nonzero elements.
%   I = FIND(B) returns the indices of the image B that are
%   non-zero.
%
%   [I,V] = FIND(B) also returns a 1-D image containing the
%   nonzero pixels in B.  Note that find(B) and find(B~=0) will
%   produce the same I, but the latter will produce a V with all
%   1's.
%
%   FIND(B,K) finds at most the first K nonzero indices.
%   FIND(B,K,'first') is the same. FIND(B,K,'last') finds at most
%   the K last indices. This syntax is only valid on versions of
%   MATLAB in which the built-in FIND supports these options.
%
%   See also DIP_IMAGE/FINDCOORD, DIP_IMAGE/SUB2IND, DIP_IMAGE/IND2SUB.

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

function varargout = find(in,k,m)
if ~isa(in,'dip_image') || ~isscalar(in)
   error('First argument must be a scalar image.');
end
if nargin==1
   I = find(in.Data);
   I = I(:);
   varargout{1} = I-1;
   if nargout > 1
      varargout{2} = dip_image(in.Data(I));
   end
else
   if nargin<3
      m = 'first';
   end
   I = find(in.Data,k,m);
   I = I(:);
   varargout{1} = I-1;
   if nargout > 1
      varargout{2} = dip_image(in.Data(I));
   end
end
