%CUMSUM   Cumulative sum of pixels.
%   C = CUMSUM(B) returns an image with the same size as B, where each pixel
%   is the cumulative sum of all pixels with equal or smaller indices in image
%   B. That is, C(I,J) = SUM(B(0:I,0:J).
%
%   C = CUMSUM(B,M) presumes that the pixels not selected by the mask image M are
%   zero.
%
%   C = CUMSUM(B,M,DIM) performs the computation over the dimensions specified in
%   DIM. DIM can be an array with any number of dimensions. M can be [].
%   If DIM==1, then C(I,J) = SUM(B(0:I,J).

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
