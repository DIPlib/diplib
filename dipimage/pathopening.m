%PATHOPENING   Path opening
%
% SYNOPSIS:
%  image_out = pathopening(image_in,filterParams,mode)
%
% PARAMETERS:
%  filterParams: Either a scalar value or an array with as many elements as
%                dimensions in IMAGE_IN. See below.
%  mode:         Cell array with one of 'unconstrained' or 'constrained',
%                and/or 'robust'. See below.
%
% DEFAULTS:
%  filterParams = 7
%  mode = {'unconstrained'}
%
%  The path opening is an opening over all possible paths of a specific length
%  and general direction. A path direction represents a 90 degree cone within
%  which paths are generated. The paths are formed by single pixel steps in one
%  of three directions (in 2D): the main direction, or 45 degrees to the left
%  or right. That is, if the main direction is [1,0] (to the right), then
%  [1,-1] and [1,1] (diagonal up or down) are also possible steps. This leads
%  to a number of different paths that is exponential in its lengths. However,
%  the opening over all these paths can be computed in O(n log n ) time, with
%  n the path length.
%
%  The direction description above can be generalized to any number of
%  dimensions by realizing that the main direction can be specified by any of
%  the neighbors of a central pixel, and then the other allowed steps are the
%  neighbor pixels that are also neighbor to the pixel that represents the main
%  direction. In 3D, this leads to 6 or 8 alternate steps.
%
%  There are 4 possible path directions in 2D, and 13 in 3D. Both length and
%  direction are specified through the FILTERPARAM argument, see below.
%
%  When MODE constains 'constrained', the path construction described above is
%  modified such that, after every alternate step, a step in the main direction
%  must be taken. This constraint avoids a zig-zag line that causes the path
%  opening to yield much shorter lines for the diagonal directions if the lines
%  in the image are thicker than one pixel. See the paper by Luengo referenced
%  below. It also reduces the cone size from 90 degrees to 45 degrees, making
%  the algorithm more directionally-selective. The constrained mode increases
%  computation time a little, but is highly recommended when using the path
%  opening in a granulometry.
%
%  When MODE constains 'robust', single-pixel gaps in lines are bridged.
%
%  Definition of FILTERSIZE: LENGTH = MAX(ABS(FILTERSIZE)) is the number of
%  pixels in the line. The path direction is determined by translating
%  FILTERSIZE to an array with -1, 0 and 1 values using
%     DIRECTION = ROUND(FILTERSIZE/LENGTH)
%  For example, if FILTERSIZE = [7,0], then LENGTH is 7, and DIRECTION is [1,0]
%  (to the right), with [1,1] and [1,-1] as alternate directions.
%
%  If FILTERSIZE is scalar, then the path opening is applied under all possible
%  directions, using the FILTERSIZE as length. The supremum over the openings
%  is the final result.
%
% DIPlib:
%  This function calls the DIPlib functions dip::PathOpening and
%  dip::DirectedPathOpening.

% (c)2017-2018, Cris Luengo.
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

function out = pathopening(varargin)
out = dip_morphology('pathopening',varargin{:});
