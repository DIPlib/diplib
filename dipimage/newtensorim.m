%NEWTENSORIM   Creates a tensor dip_image
%   NEWTENSORIM(N) is a 0D vector image with N elements.
%
%   NEWTENSORIM([N,M]) is a 0D image with a N-by-M tensor.
%
%   NEWTENSORIM([N,M],[P,Q,...]) is a image of size P-by-Q-by-... with a
%   N-by-M tensor.
%
%   NEWTENSORIM(...,TYPE) sets the data type of the new image to TYPE.
%   TYPE can be any of the type parameters allowed by DIP_IMAGE. The
%   default data type is 'single'.
%
%   NEWTENSORIM(A) is an image identical to the tensor image A, set to 0.
%
%   NEWTENSORIM(A,B,C) is a vector image where the tensor components are
%   the scalar images A, B and C. This mode is used whenever there is
%   more than one input parameter, and any one of them is not a scalar
%   or vector array. You can also use DIP_IMAGE({A,B,C}).
%
%  SEE ALSO: newim, newcolorim, dip_image

% (c)2017, Cris Luengo.
% (c)1999-2014, Delft University of Technology.
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

function out = newtensorim(varargin)
tsize = [];
imsize = [];
dtype = 'sfloat';
if nargin == 0
   error('Argument expected')
end
imgs = false;
for ii=1:nargin
   if isa(varargin{ii},'dip_image') || ~isvector(varargin{ii})
      imgs = true;
      break;
   end
end
if imgs
   if nargin==1
      out = varargin{1};
      out(:) = 0;
   else
      out = dip_image(varargin');
   end
   return;
end
tsize = varargin{1};
if ~isnumeric(tsize) || numel(tsize) < 1 || numel(tsize) > 2 || any(fix(tsize)~=tsize)
   error('Size vector must be a row vector with at most 2 integer elements')
end
N = 2;
while nargin >= N
   if isempty(imsize) && isnumeric(varargin{N})
      imsize = varargin{N};
      if (~isempty(imsize) && ~isvector(imsize)) || any(fix(imsize)~=imsize)
         error('Size vector must be a row vector with at most 2 integer elements')
      end
      N = N+1;
   else
      dtype = varargin{N};
      if ~ischar(dtype) || ~isvector(dtype)
         error('Data type must be a string')
      end
      if nargin > N
         error('Too many input arguments')
      end
      break;
   end
end
out = dip_image(zeros(prod(tsize),1),tsize,dtype);
if ~isempty(imsize)
   out = repmat(out,imsize);
end
