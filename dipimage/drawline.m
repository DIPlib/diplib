%DRAWLINE   Draws a line in an image
%
% SYNOPSIS:
%  image_out = drawline(image_in,start,end,color,sigma,truncation,blend)
%
%  start:      array containing the start position(s) of the line(s)
%  end:        array containing the end position(s) of the line(s)
%  color:      color of the line(s), either one or one per line
%  sigma:      if larger than 0, draws a line with a Gaussian profile.
%  truncation: if sigma>0, the Gaussian profile is computed up to
%              trunction*sigma from the line.
%  blend:      a string 'assign' or 'add' defining the blend mode. If
%              sigma>0, the blend mode is always 'add'.
%
% DEFAULTS:
%  color = 255
%  sigma = 0
%  truncation = 3
%  blend = 'assign' (sigma=0) or 'add' (sigma>0)
%
% EXAMPLE:
%  drawline(newim,[100 100; 0 200],[200 200; 200 0],255)
%
% DIPlib:
%  This function calls the DIPlib function dip::DrawLine and
%  dip::DrawBandlimitedLine

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

function out = drawline(varargin)
out = dip_generation('drawline',varargin{:});
