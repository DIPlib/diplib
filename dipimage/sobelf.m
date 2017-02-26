%SOBELF   Sobel derivative filter
%
% SYNOPSIS:
%  image_out = sobelf(image_in,direction)
%
% PARAMETERS:
%  direcion: Dimension to take the derivative along.
%            Dimensions are 1-based (i.e. x = 1, y = 2).
%
% DEFAULTS:
%  direction = 1
%
% SEE ALSO:
%  prewittf, derivative, dx, dy, etc.

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

function out = sobelf(in,dim)
nd = ndims(in);
filter = cell(1,nd);
for ii=1:nd
   if ii==dim
      filter{ii} = [1,0,-1]/2;
   else
      filter{ii} = [1,2,1]/4;
   end
end
out = convolve(in,filter);
