%RECONSTRUCTION   Morphological reconstruction by dilation
%
% SYNOPSIS:
%  out = reconstruction(marker,mask,connectivity,flag)
%
% PARAMETERS:
%  marker:       seed image for reconstruction
%  mask:         image to be reconstructed
%  connectivity: defines the metric, that is, the shape of the structuring
%     element.
%     * 1 indicates city-block metric, or a diamond-shaped S.E in 2D.
%     * 2 indicates chessboard metric, or a square structuring element in 2D.
%     For 3D images use 1, 2 or 3.
%  flag:         a string, either "dilation" or "erosion".
%
% DEFAULTS:
%  connectivity = 1
%  flag = 'dilation'
%
% EXAMPLE:
%  a = -readim('cermet')
%  b = erosion(a,20)
%  c = reconstruction(b,a)
%  c = b==c
%  b(c) = a(c)
%  c = reconstruction(b,a)
%
% EXAMPLE:
%  a = readim('cermet')<128
%  b = label(bskeleton(a,0,'looseendsaway'))
%  c = reconstruction(b,a*1000)
%
% DIPlib:
%  This function calls the DIPlib functions dip::MorphologicalReconstruction.

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
