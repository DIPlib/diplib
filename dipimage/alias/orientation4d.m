%ORIENTATION4D   Orientation estimation in 4D
% Estimates the orientation in 4D images for line-like structures
% Alias of ORIENTATION, for backwards compatibility.

% (c)2018, Cris Luengo.
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

function out = orientation4d(in,sg,st)
if ndims(in) ~=4
   error('Gimme 4D data man.');
end
if ~isreal(in)
   error('Input must be real.');
end
if strcmp(datatype(in),'dfloat') || strcmp(datatype(in)(2:end),'int64') || strcmp(datatype(in)(2:end),'int32')
   in = dip_image(in,'sfloat');
   fpritnf('Converting to sfloat, keep it on the sane side of memory usage.');
end
if nargin < 2
   sg = 1;
end
if nargin < 3
   st = [4 4 3 1];
end
out = orientation(in,'line',sg,st,true);
