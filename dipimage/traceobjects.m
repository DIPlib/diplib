%TRACEOBJECTS   Traces the objects in an image
%
% SYNOPSIS:
%  traces = traceobjects(image_in,labels,connectivity,output)
%
% PARAMETERS:
%  image_in:     Binary or labeled image, only 2D supported
%  labels:       The labels to trace. [] traces all objects
%  connectivity: 1 for 4-connected objects, 2 for 8-connected objects
%  output:       String, one of 'chain code', 'polygon', 'convex hull'
%
% DEFAULTS:
%  labels = []
%  connectivity = 2
%  output = 'polygon'
%
%  If IMAGE_IN is binary, labels the image and traces all objects. LABELS
%  is not expected.
%
%  TRACES is a cell array with a vector for each object. The vector is
%  either a chain code or a polygon. If 'convex hull' was given, then the
%  output polygon represents the convex hull of the object.
%
%  Chain codes are represented by a uint8 vector, each element has the
%  following representation: the bottom 3 bits represent the chain code.
%  and bit 3 is set if the ojbect touches the image edge at that pixel.
%  In a 4-connected chain code, codes are in the range 0-3. In an
%  8-connected chain code, they are in the range 0-7. A chain code of 0 is
%  a step to the right, and codes are numbered counter-clockwise (a code of
%  1 is up-right (8-connected) or up (4-connected), etc.)
%
%  Holes in objects are not represented, and if an object is not connected
%  (i.e. two different connected components have the same label) only the
%  first one encountered is traced.
%
% EXAMPLE:
%  a = readim('cermet');
%  b = a<120;
%  pol = traceobjects(b,[],2,'polygon');
%  ch = traceobjects(b,[],2,'convex hull');
%  a/2 + 128
%  hold on
%  n=23;
%  plot(pol{n}(:,1),pol{n}(:,2))
%  plot(ch{n}(:,1),ch{n}(:,2))
%
% DIPlib:
%  This function calls the DIPlib function dip::GetImageChainCodes.

% (c), Cris Luengo.
% Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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

function out = traceobjects(varargin)
out = dip_segmentation('traceobjects',varargin{:});
