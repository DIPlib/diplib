%ROTATION3D   Rotate a 3D image
%
% There are 2 different methods avaiable to do the rotation:
% by DIRECT mapping or by 9 SHEARS. Both have different advantages.
%
% SYNOPSIS:
%  image_out = rotation3d(image_in,method,rotation_params,origin,interpolation,bgval)
%
% PARAMETERS:
%  method: Computation method: 'direct' or '9 shears'
%  rotation_params:
%      DIRECT: 1x3 array containing the Euler angles [alpha beta gamma]
%              3x3 array containing the rotation matrix
%              4x4 array containing the transformation matrix in homogenous
%                  coordinates (or 3x4, assuming the bottom row is [0,0,0,1].
%      9 SHEARS: 1x3 array containing the Euler angles [alpha beta gamma]
%  origin: Coordinates for the origin (3-element vector). Only used in DIRECT
%      mode. For 9 SHEARS the origin is always the center of the image.
%  interpolation: One of: 'linear', '3-cubic', 'nearest'.
%      In 9 SHEARS mode, other methods are available too, see ROTATION.
%  bgval: One of 'zero', 'min', 'max'. Ignored in DIRECT mode (always 'zero').
%
% DEFAULTS:
%  origin = []  (uses the center of the image)
%  interpolation = '3-cubic'
%  bgval = 'zero'
%
% NOTES:
%  All angles are in radian.
%
%  Rotation in 3D by 3 Euler angles:
%     R = R_{3''}(gamma) R_{2'}(beta) R_3(alpha)
%  Note: There is another definition of the Euler angle. The second rotation
%  is then about the intermediate 1-axis. This convention is traditionally
%  used in the mechanics of the rigid body. The angles are called psi, theta,
%  phi and their relation with the other angles is:
%     phi = gamma-pi/2 mod 2pi,
%     theta = beta,
%     psi = alpha+pi/2 mod 2pi.
%
%  The output image has the same data type as the input image.
%
%  The DIRECT method preserves the input image's dimensions, portions of the
%  input image will end up outside the output image. The 9 SHEARS method
%  expands the image such that all input pixels fit in the output.
%
%  The homogeneous coordinate array (in the DIRECT method) uses the center of
%  the image as coordinate system origin. ORIGIN is ignored in this case.
%
% LITERATURE:
%  D. Hearn and M.P. Baker, Computer Graphics, Prentice Hall
%  J. Foley and A. van Dam, Computer Graphics, Addison-Wesly
%  F. Scheck, Mechanics, Springer
%
% SEE ALSO:
%  rotation, skew, affine_trans, resample
%
% DIPlib:
%  This function calls the DIPlib functions <a href="https://diplib.org/diplib-docs/geometry.html#dip-Rotation3D-Image-CL-Image-L-dfloat--dip-uint--String-CL-String-CL">dip::Rotation3D</a>,
%  <a href="https://diplib.org/diplib-docs/geometry.html#dip-AffineTransform-Image-CL-Image-L-FloatArray-CL-String-CL">dip::AffineTransform</a> and <a href="https://diplib.org/diplib-docs/geometry.html#dip-RotationMatrix3D-Image-L-dfloat--dfloat--dfloat-">dip::RotationMatrix3D</a>.

% (c)2020, Cris Luengo.
% Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
% Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
% Originally written by Bernd Rieger.
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

function out = rotation3d(in,method,rotpara,origin,interpolation,bgval)
in = dip_image(in);
if ndims(in) ~= 3
   error('Input image must be 3D')
end
if nargin<5
   interpolation = '';
end
switch method
   case 'direct'
      if nargin < 4 || isempty(origin)
          origin = [0,0,0];
      end
      % Rotation
      if numel(rotpara) == 3
         % 3 Euler angles
         R = dip_geometry('rotationmatrix',rotpara);
      elseif isequal(size(rotpara),[3,3]) || isequal(size(rotpara),[3,4]) || isequal(size(rotpara),[4,4])
         % Rotation matrix or homogeneous coordinates
         R = rotpara;
      else
         error('Rotation parameters invalid.');
      end
      R(4,1:4) = [0,0,0,1];
      % Translate
      T1 = eye(4,4);
      T1(1:3,4) = -origin;
      % Translate back
      T2 = eye(4,4);
      T2(1:3,4) = origin;
      % Combine steps to overall Matrix
      R = T2 * R * T1;
      out = dip_geometry('affine_trans',in,R(1:3,:),interpolation);
   case '9 shears'
      if ~isnumeric(rotpara) || numel(rotpara)~=3
         error('rotation_params must be a 3-vector')
      end
      if nargin<6
         bgval = 'zero';
      end
      switch bgval % Note that other boundary conditions are allowed here!
         case 'zero'
            bgval = 'add zeros';
         case 'min'
            bgval = 'add min';
         case 'max'
            bgval = 'add max';
      end
      out = dip_geometry('rotation3d',in,rotpara(1),rotpara(2),rotpara(3),interpolation,bgval);
   otherwise
      error('Unkown rotation method.');
end
