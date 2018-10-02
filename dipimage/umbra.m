%UMBRA   Umbra of an image
%
% SYNOPSIS:
%  [image_out, offset] = umbra(image_in)
%
% LITERATURE:
%  H.J.A.M. Heijmans, A note on the umbra transform in gray-scale morphology
%  Pattern Recognition Letters, 14(11):877-881, 1993
 
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

function [out,offset] = umbra(in)

if ~isa(in,'dip_image')
   in = dip_image(in);
end

d = ndims(in);
out = round(in);
mm = getmaximumandminimum(out);
offset = mm(1);
range = diff(mm);

sz = ones(1,d+1);
sz(d+1) = range+1;
ram = ramp(sz,d+1,'corner');
out = (out - ram) > offset;
