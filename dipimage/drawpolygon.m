%DRAWPOLYGON   Draws a polygon in an image
%
% SYNOPSIS:
%  out = drawpolygon(in,coordinates,color,mode);
%
% PARAMETERS:
%  coordinates: matrix containing the coordinates of the corner points
%      of the polygon. It is organised as follows:
%      [x1 y1 z1; x2 y2 z2; etc...]
%  color: gray value(s) with which to draw the lines
%  mode:  'open': do not connect the last point to the first point
%         'closed': do connect the last point to the first point
%         'filled': draw a filled polygon (2D only)
%
% DEFAULTS:
%  color = 255
%  closed = 'open'
%
% DIPlib:
%  This function calls the DIPlib function dip::DrawLines and
%  dip::DrawPolygon2D

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

function out = drawpolygon(varargin)
out = dip_generation('drawpolygon',varargin{:});
