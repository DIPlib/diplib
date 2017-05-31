%RAMP   Creates an image with general coordinates
%   RAMP(SIZE,DIM) returns an image of size SIZE with the value of the
%   DIM dimension's coordinate as the pixel values.
%
%   RAMP(IMG,DIM) is the same as RAMP(SIZE(IMG),DIM).
%
%   RAMP(...,ORIGIN) allows specifying where the origin is:
%      'left'    the pixel to the left of the true center
%      'right'   the pixel to the right of the true center (default)
%      'true'    the true center, between pixels if required
%      'corner'  the pixel on the upper left corner (indexed at (0,0))
%   Note that the first three are identical if the size is odd.
%
%   RAMP(...,'freq') uses frequency domain coordinates, range=(-0.5,0.5)
%      (not in combination with ORIGIN).
%   RAMP(...,'radfreq') uses frequency domain coordinates, range=(-pi,pi)
%      (not in combination with ORIGIN).
%
%   RAMP(...,'math')  Let the Y coordinate increase upwards instead of
%      downwards.
%   RAMP(...,'mleft') Combines 'left' with 'math'. Also available are:
%      'mright', 'mtrue', 'mcorner', 'mfreq' and 'mradfreq'.
%      In the case of 'mcorner' the origin is moved to the bottom of
%      the image.
%   Alternatively, 'math' and an other option can be combined in a cell array:
%   RAMP(...,{'left','math'}).
%
%   Note that the syntax RAMP(X,Y,Z,...) is illegal, in contrast to
%   the functions XX, YY, etc.
%
% DIPlib:
%  This function calls the DIPlib function dip::FillRamp.

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
