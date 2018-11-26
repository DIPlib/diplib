%SPLIT   Split an image into subsampled versions
%
%  Reduces size of an image by an integer factor, by subsampling.
%  The subsampling factor evenly divides the image. The output image
%  has an additional dimension, where all possible subsampling shifts
%  are present. That is, the output image has the same number of pixels
%  as the input, but arranged in one more dimension.
%
% SYNOPSIS:
%  out = split(in, stepsize)
%
% PARAMETERS:
%  in:       input image
%  stepsize: integer numbers (array) that divides the image without
%            remainder
%
% SEE ALSO:
%  rebin, subsample
%
% NOTE:
%  REBIN is equal to the sum along the new dimension of the output of SPLIT:
%     a = readim;
%     all(sum(split(a),[],3) == rebin(a))

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

function out = split(in,stepsize)
if nargin<2
   stepsize = 2;
end
if ~isa(in,'dip_image')
   in = dip_image(in);
end
szI = imsize(in);
N = numel(szI);
if numel(stepsize)==N
   stepsize = reshape(stepsize,1,N);
elseif numel(stepsize)==1
   stepsize = repmat(stepsize,1,N);
else
   error('STEPSIZE must be a scalar or have one element per image dimension');
end
if any(rem(stepsize,1))
   error('STEPSIZE must be integer');
end
if any(rem(szI,stepsize))
   error('STEPSIZE must be divider of all image dimensions.');
end
if N>1
   % X and Y dims are swapped in memory
   szI = szI([2,1,3:end]);
   stepsize = stepsize([2,1,3:end]);
end
szO = szI./stepsize;
szX = [stepsize;szO];
szX = szX(:)';
szX = szX([2,1,3:end]); % swap X and Y
out = reshape(in,szX);
dims = [2:2:2*N,1:2:2*N];
dims(1) = 1;
dims(N+1) = 2;
dims = dims([2,1,3:end]); % swap X and Y
out = permute(out,dims);
if N>1
   szO = szO([2,1,3:end]); % swap X and Y
end
out = reshape(out,[szO,prod(stepsize)]);
