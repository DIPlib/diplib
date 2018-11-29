%DRAWSHAPE   Draws an ellipse (ellipsoid), rectangle (box) or diamond
%
% SYNOPSIS:
%  image_out = drawshape(image_in,sizes,origin,shape,color,sigma,truncation)
%
%  sizes:      array containing the size of the shape along each dimension.
%  origin:     array containing the origin of the shape (center).
%  shape:      a string 'ellipse'/'ellipsoid', 'disk'/'ball', 'circle'/'sphere',
%              'rectangle'/'box', 'box shell', or 'diamond'.
%  color:      color of the shape.
%  sigma:      if larger than 0, draws the shape with a Gaussian profile.
%  truncation: if SIGMA>0, the Gaussian profile is computed up to
%              TRUNCATION*SIGMA from the edge.
%
% DEFAULTS:
%  shape = 'ellipsoid'
%  color = 255
%  sigma = 0
%  truncation = 3
%
% NOTES:
%  If SIGMA==0, a discrete shape is drawn. The pixels making up the shape are
%  set to COLOR. Other pixels in IMAGE_IN are copied to the output unchanged.
%  'ellipse'/'elliposid' and 'diamond' are always discrete shapes.
%
%  If SIGMA>0, a (approximately) bandlimited shape is drawn using Gaussian
%  profiles. The shape is added to existing values in IMAGE_IN to create
%  IMAGE_OUT. 'circle'/'sphere' and 'box shell' are always bandlimited shapes.
%
%  SIZES and ORIGIN are given as floating-point values. ORIGIN does not need
%  to be within the image domain.
%
%  Shape description:
%   - 'ellipse' or 'ellipsoid': this is always a discrete shape, SIGMA is
%     ignored. SIZES indicates the diameter along each of the dimensions. The
%     shape is solid.
%   - 'disk' or 'ball': this is always isotropic, SIZES must be a scalar, and
%     determines the diameter. The shape is solid. If SIGMA==0, produces the
%     same result as 'ellipsoid'.
%   - 'circle' or 'sphere': this always has a Gaussian profile, SIGMA must be
%     non-zero. It is always isotropic, SIZES must be a scalar, and determines
%     the diameter. The shape is hollow (i.e. it is the shell of a 'ball').
%   - 'rectangle' or 'box': this is a solid shape. SIZES determines the width
%     of the box.
%   - 'box shell': this always has a Gaussian profile, SIGMA must be non-zero.
%     SIZES determines the width of the box. The shape is hollow (i.e. it is
%     the shell of a 'box').
%   - 'diamond': this is always a discrete shape, sigma is ignored. SIZES
%     indicates the extent along each of the dimensions. The shape is solid.
%
% EXAMPLE:
%  drawshape(newim,80,[127.7,128.1],'disk',255,1)
%
% DIPlib:
%  This function calls the DIPlib functions dip::DrawEllipsoid, dip::DrawBox,
%  dip::DrawDiamond, dip::DrawBandlimitedBall, and dip::DrawBandlimitedBox.

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

function out = drawshape(varargin)
out = dip_generation('drawshape',varargin{:});
