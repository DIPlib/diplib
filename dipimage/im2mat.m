%IM2MAT   Converts a dip_image to a matlab array.
%
% SYNOPSIS:
%  A = IM2MAT(B,DATATYPE)
%
%  Coverts B into a matlab array of class DATATYPE. If DATATYPE
%  is left out, doesn't perform any conversion.
%
%  If B has more than one tensor element, the first dimension of
%  the matlab matrix is the tensor dimension.
%
% EXAMPLES:
%  a = readim('flamingo');
%  b = im2mat(a);
%  c = mat2im(b,'rgb');   % c is identical to a.
%
%  a = readim('chromo3d');
%  a = gradientvector(a);
%  b = im2mat(a);
%  c = mat2im(b,1);       % c is identical to a.
%
% NOTE:
%  This function is identical to dip_image/dip_array.
%
% SEE ALSO:
%  mat2im, dip_image, dip_image.dip_array

% (c)2017, Cris Luengo.
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

function out = im2mat(varargin)
if ~isa(varargin{1},'dip_image')
   error('Input image expected');
end
out = dip_array(varargin{:});
