%COMPLEX   Construct complex image from real and imaginary parts.
%   COMPLEX(A,B) returns the complex result A + Bi, where A and B
%   are identically-sized real-valued images.
%
%   COMPLEX(A) returns the complex result A + 0i, where A must
%   be real.

% (c)2017-2019, Cris Luengo.
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

function out = complex(out,im)
if nargin == 1
   if ~iscomplex(out)
      if isa(out.Data,'double') || isa(out.Data,'uint64') || isa(out.Data,'int64') ...
                                || isa(out.Data,'uint32') || isa(out.Data,'int32')
         out.Data = cat(1,double(out.Data),zeros(size(out.Data)));
      else
         out.Data = cat(1,single(out.Data),zeros(size(out.Data),'single'));
      end
   end
else % nargin == 2
   if iscomplex(out) || iscomplex(im)
      error('Expected two real-valued images')
   end
   if isa(out.Data,'double') || isa(out.Data,'uint64') || isa(out.Data,'int64') ...
                             || isa(out.Data,'uint32') || isa(out.Data,'int32') ...
   || isa(im.Data, 'double') || isa(im.Data, 'uint64') || isa(im.Data, 'int64') ...
                             || isa(im.Data, 'uint32') || isa(im.Data, 'int32')
      out.Data = cat(1,double(out.Data),double(im.Data));
   else
      out.Data = cat(1,single(out.Data),single(im.Data));
   end
end
