%CLONE   Creates a new image, identical to in.
%   Conceptually OUT = CLONE(IN) is the same as OUT = IN; OUT(:) = 0;
%   That is, it creates a new image, initialzed to zero, with all the same
%   properties as IN.
%
%   Additional key/value pairs can be used to change specific properties:
%    - 'imsize': changes the sizes of the image
%    - 'tensorsize': changes the sizes of the tensor
%    - 'tensorshape': changes how the dip_image.TensorShape property
%    - 'colorspace': changes the color space
%    - 'datatype': changes the data type, see DIP_IMAGE/DIP_IMAGE for valid
%      DATATYPE strings.
%
%   If 'tensorsize' or 'tensorshape' are given, any previous color space
%   settings will be ignored, including that in IN. Likewise, if
%   'colorspace' is given, any previous tensor size or shape settings
%   will be ignored
%
%   SEE ALSO: newim, newtensorim, newcolorim, dip_image

% (c)2018-2019, Cris Luengo.
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

function out = clone(in,varargin)
if mod(length(varargin),2) ~= 0
   error('All keys must have a value');
end
kk = 1;
imsz = imsize(in);
telem = numtensorel(in);
tsz = [];
tsh = 'column vector';
colsp = in.ColorSpace;
if isempty(colsp)
   tsz = in.TensorSizeInternal;
   tsh = in.TensorShapeInternal;
end
dtype = datatype(in);
while kk < length(varargin)
   key = varargin{kk};
   value = varargin{kk+1};
   kk = kk+2;
   if ~ischar(key)
      error('Keys must be strings')
   end
   switch key
      case 'imsize'
         imsz = value;
         if ~isnumeric(imsz) || ~isvector(imsz)
            error('IMSIZE value must be a numeric vector')
         end
      case 'tensorsize'
         tsz = value;
         if ~isnumeric(tsz) || numel(tsz)<1 || numel(tsz)>2
            error('TENSORSIZE value must be numeric and have 1 or 2 elements')
         end
         colsp = '';
         telem = 0;
      case 'tensorshape'
         tsh = value;
         if ~ischar(tsh)
            error('TENSORSHAPE value must be a string')
         end
         colsp = '';
         telem = 0;
      case 'colorspace'
         colsp = value;
         if ~ischar(colsp)
            error('COLORSPACE value must be a string')
         end
         telem = colorspacemanager(colsp);
         tsz = telem;
         tsh = 'column vector';
      case 'datatype'
         dtype = value;
         if ~ischar(dtype)
            error('DATATYPE value must be a string')
         end
   end
end
update_tsh = '';
if telem == 0
   % Compute telem
   switch tsh
      case {'column vector','row vector'}
         telem = prod(tsz);
      case {'column-major matrix','row-major matrix'}
         telem = prod(tsz);
         update_tsh = tsh;
         tsh = tsz;
      case 'diagonal matrix'
         if numel(tsz)==1
            tsz = [tsz,tsz];
         else
            if tsz(1)~=tsz(2)
               error('A diagonal matrix must be square')
            end
         end
         telem = tsz(1);
      case {'symmetric matrix','upper triangular matrix','lower triangular matrix'}
         if numel(tsz)==1
            tsz = [tsz,tsz];
         else
            if tsz(1)~=tsz(2)
               error('A diagonal matrix must be square')
            end
         end
         telem = tsz(1) * (tsz(1)+1) / 2;
      otherwise
         error('Bad value for TENSORSHAPE')
   end
end
out = dip_image(zeros(telem,1),tsh,dtype);
if ~isempty(update_tsh)
   out.TensorShape = update_tsh;
end
if ~isempty(imsz)
   out = repmat(out,imsz);
end
out.ColorSpace = colsp;
out.PixelSize = in.PixelSize;
