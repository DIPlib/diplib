%IM2MAT   Converts a dip_image to a MATLAB array
%
% SYNOPSIS:
%  A = IM2MAT(B,DATATYPE)
%
%  Coverts B into a MATLAB array of class DATATYPE. If DATATYPE is left out,
%  doesn't perform any conversion.
%
%  If B has more than one tensor element, the last dimension of the MATLAB
%  array is the tensor dimension. This causes data to be copied.
%
% EXAMPLES:
%  a = readim('flamingo');
%  b = im2mat(a);
%  c = mat2im(b,'srgb');   % c is identical to a.
%
%  a = readim('chromo3d');
%  a = gradientvector(a);
%  b = im2mat(a);
%  c = mat2im(b,-1);       % c is identical to a.
%
% NOTES:
%  dip_image/dip_array does the same thing, but always puts the tensor
%  dimension as the first dimension of the MATLAB array, not copying data.
%
%  The first two dimensions are always swapped, because of the way that data
%  is stored in MATLAB arrays vs DIPlib images.
%
% SEE ALSO:
%  mat2im, im2cell, dip_image.dip_array

% (c)2017-2021, Cris Luengo.
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

function out = im2mat(in,varargin)
if ~isa(in,'dip_image')
   error('Input image expected');
end
if ~isscalar(in)
   in = tensortospatial(in,ndims(in)+1);
end
out = dip_array(in,varargin{:});
