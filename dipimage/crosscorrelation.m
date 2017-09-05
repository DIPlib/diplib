%CROSSCORRELATION   Computes the cross-correlation between two images
%
% SYNOPSIS:
%  image_out = crosscorrelation(image1, image2)

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

function out = crosscorrelation(in1,in2)
out = ift(ft(in1)*conj(ft(in2)));
if isreal(in1) && isreal(in2)
   out = real(out);
end
