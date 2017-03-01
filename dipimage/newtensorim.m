%NEWTENSORIM   Creates a tensor dip_image
%   NEWTENSORIM(N) is a 0D vector image with N elements.
%
%   NEWTENSORIM(N,M) or NEWTENSORIM([N,M]) is a 0D image with a N-by-M tensor.
%
%   NEWTENSORIM(A) is an image identical to the tensor image A, set to 0.
%
%   NEWTENSORIM(A,B,C) is a vector image where the tensor components are the
%   scalar images A, B and C. This mode is used whenever there is more than
%   one input parameter, and any one of them is not scalar.
%   You can also use DIP_IMAGE({A,B,C}).
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
if nargin > 1
   imgs = false;
   for ii=1:nargin
      if isa(varargin{ii},'dip_image') || numel(varargin{ii}) ~= 1
         imgs = true;
         break;
      end
   end
   if imgs
      out = dip_image(varargin);
      return;
   else
      n = [varargin{:}];
   end
elseif nargin == 1
   n = varargin{1};
   if isa(n,'dip_image')
      out = n;
      out(:) = 0;
      return;
   end
else
   error('Argument expected')
end
if ~isnumeric(n) || numel(n) < 1 || numel(n) > 2 || any(fix(n)~=n)
   error('Size vector must be a row vector with at most 2 integer elements')
end
out = dip_image(zeros(prod(n),1,'single'),n);
