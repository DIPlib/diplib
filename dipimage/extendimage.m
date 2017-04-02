%EXTENDIMAGE   Extends the image domain
%
% SYNOPSIS:
%  image_out = extendimage(image_in,border,boundary_condition)
%
%  BORDER is an integer or a vector of integers (one per image dimension)
%  specifying by how many pixels each image edge needs to be expanded. For
%  example, if IMAGE_IN is 2D, and BORDER = [4,6], then the image will be
%  expanded by 4 pixels on the left and right side, and by 6 on the top
%  and bottom, yielding IMSIZE(IMAGE_OUT) == IMSIZE(IMAGE_IN) + [4,6]*2.
%
%  BOUNDARY_CONDITION is a string or a cell array of strings (one per image
%  dimension) specifying how the pixel values outside of the original image
%  domain are to be filled. See HELP BOUNDARY_CONDITION.
%
% DEFAULTS:
%  boundary_condition = 'mirror'
%
% DIPlib:
%  This function calls the DIPlib functions dip::ExtendImage.

% (c)2017, Cris Luengo.
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
