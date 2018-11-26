%FMMATCH   Matches two images using the Fourier-Mellin transform
%  Finds the scaling, rotation and translation between two 2D images
%
% SYNOPSIS:
%  [image_out,matrix] = fmmatch(image_in1,image_in2,method)
%
% PARAMETERS:
%  method:  Interpolation method, one of: 'linear', '3-cubic', or 'nearest'.
%
% OUPUTS:
%  image_out:  image_in2 transformed to match image_in1.
%  matrix:     Affine transformation matrix (2x3, the bottom row is [0,0,1])
%              that can be used to transform image_in2 to yield image_out.
%
% EXAMPLE:
%  a = readim('trui');
%  phi = pi/8;
%  m = [cos(phi),sin(phi),30;-sin(phi),cos(phi),-20];
%  b = affine_trans(a,m);
%  [c,n] = fmmatch(a,b);
%  dipshow(a-c,'based')  % -> shows c matches a (except where stuff ended up
%                        %    outside the image boundaries
%  disp(inv([m;0,0,1]))  % -> the top two rows here match n
%  disp(n)
%  d = affine_trans(b,n) % -> d should be approximately identical to c
%
% SEE ALSO:
%  affine_trans, findshift
%
% DIPlib:
%  This function calls the DIPlib function dip::FourierMellinMatch2D.

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

function varargout = fmmatch(varargin)
varargout = cell(1,max(nargout,1));
[varargout{:}] = dip_geometry('fmmatch',varargin{:});
