%MAXIMA   Detect local maxima
%
% MAXIMA returns a binary image with pixels set that belong to local maxima
% of the input.
%
% SYNOPSIS:
%  image_out = maxima(image_in,connectivity,flag)
%
% PARAMETERS:
%  connectivity: defines the metric, that is, the shape of the structuring
%     element.
%     * 1 indicates city-block metric, or a diamond-shaped S.E in 2D.
%     * 2 indicates chessboard metric, or a square structuring element in 2D.
%     For 3D images use 1, 2 or 3.
%  flag: set to 'labels' to return a labeled output image.
%
% DEFAULTS:
%  connectivity = 1
%  flag = 'binary'
%
% NOTE:
%     a = maxima(in,2,'labels')
%  is equivalent to
%     a = label(maxima(in,2),2)
%  except that labels might be assigned in a different order.
%
% NOTE:
%  See the user guide for the definition of connectivity in DIPimage.
%
% SEE ALSO:
%  minima
%
% DIPlib:
%  This function calls the DIPlib function dip::Maxima.

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

function out = maxima(varargin)
out = dip_morphology('maxima',varargin{:});
