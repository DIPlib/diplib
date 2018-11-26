%AFFINE_TRANS   Rotate, translate and scale a 2D or 3D image
%
% SYNOPSIS:
%  [image_out,R] = affine_trans(image_in,zoom,translation,angle,method)
%  [image_out] = affine_trans(image_in,R,method)
%
% PARAMETERS:
%  zoom:        array containing a zoom
%  translation: array containing a transloation
%  angle:       rotation angle [in radian]
%  R:           linear transformation matrix
%  method:      one of: 'linear', '3-cubic', 'nearest'
%
% DEFAULTS:
%  method = 'linear'
%
% NOTE:
%  In the first form, all three ZOOM, TRANSLATION and ANGLE must be given.
%  This form only applies to 2D images. The affine transform matrix R is
%  formed as follows:
%
%      R = [ zoom(1)*cos(angle), -zoom(1)*sin(angle), translation(1);
%            zoom(2)*sin(angle),  zoom(2)*cos(angle), translation(2);
%            0                 ,  0                 , 1              ]
%
%  That is, the image is first rotated, then zoomed, and then translated.
%
%  In the second form, R has either 4 values (if the image is 2D) or 9
%  (if the image is 3D), representing the linear transformation matrix.
%  Optionally, 2 or 3 values can be added representing a translation.
%  This results in an affine transform matrix (as above) but without the
%  bottom row.
%
%  The rotation is performed around the central pixel of the image. This
%  is the pixel to the right of the true center if the image is even-sized.
%
% SEE ALSO:
%  rotation, resample, fmmatch
%
% DIPlib:
%  This function calls the DIPlib function dip::AffineTransform.

% (c)2018, Cris Luengo.
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

function varargout = affine_trans(varargin)
varargout = cell(1,max(nargout,1));
[varargout{:}] = dip_geometry('affine_trans',varargin{:});
