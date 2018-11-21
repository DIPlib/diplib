%TENSORFUN   Applies a functions across tensor elements
%
% SYNOPSIS:
%  out = tensorfun(function,image_in)
%
% PARAMETERS:
%  function: one of the strings indicated below
%  image_in: the image to apply the operation on
%
% MODE 1:
%  FUNCTION is one of:
%     'imsum'     -- sum of all elements
%     'improd'    -- product of all elements
%     'imor'      -- true if any elements is non-zero
%     'imand'     -- true if all elements are non-zero
%     'immax'     -- maximum over all elements
%     'immin'     -- minimum over all elements
%     'immean'    -- mean over all elements
%     'immedian'  -- median over all elements
%     'imeq'      -- true if elements are all equal
%     'imlargest' -- index of first element with largest pixel value
%     'imsmallest'-- index of first element with smallest pixel value
%
%  OUT is a scalar image that results in applying the given projection
%  across the tensor elements. That is, each output pixel is the result
%  of the given operation on all input tensor elements.
%
%  For example,
%     A = TENSORFUN('imsum',IMG);
%  is the same as
%     A = IMG{1} + IMG{2} + IMG{3} + ...;
%
%  NOTE! The first 8 options can all be accomplished by using the 'tensor'
%  option of the equivalent method. For example 'imsum' calls
%  SUM(IMG,'tensor'). Only the last three modes are still useful.
%
% MODE 2:
%  NOTE! This exists mostly for backwards-compatibility, and should no
%  longer be useful in new code.
%
%  FUNCTION is one of:
%     'isempty'   -- true for empty image
%     'islogical' -- true for binary image
%     'isreal'    -- true for non-complex image
%     'ndims'     -- number of dimensions of image
%     'prodofsize'-- number of pixels in image
%     'max'       -- maximum pixel value in image
%     'mean'      -- mean pixel value in image
%     'median'    -- median pixel value in image
%     'min'       -- minimum pixel value in image
%     'std'       -- standard deviation of pixels in image
%     'sum'       -- sum of pixels in image
%   OUT is a MATLAB array of the same size as the tensor of IMAGE_IN,
%   each element containing the scalar result of FUNCTION on each
%   tensor element.
%
%   Note that the first 5 functions retun an array with all the same
%   values, and the latter 6 return the same as applying the method
%   of the same name to IMAGE_IN:
%      TENSORFUN('max',IMG) == MAX(IMG)
%
% SEE ALSO:
%  dip_image/iterate, dip_image/slice_op

% (c)2017-2018, Cris Luengo.
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

function out = tensorfun(fun,in)

if ~ischar(fun)
   error('First parameter should be a string')
end

if ~isa(in,'dip_image')
   in = dip_image(in);
end

if strncmpi(fun,'im',2)
   % MODE 1
   N = numtensorel(in);
   if N < 2
      error('Input image is scalar')
   end
   switch fun
      case 'imsum'
         out = dip_projection('sum',in,'tensor');
      case 'improd'
         out = dip_projection('prod',in,'tensor');
      case 'imor'
         out = dip_projection('any',in,'tensor');
      case 'imand'
         out = dip_projection('all',in,'tensor');
      case 'immax'
         out = dip_projection('max',in,'tensor');
      case 'immin'
         out = dip_projection('min',in,'tensor');
      case 'immean'
         out = dip_projection('mean',in,'tensor');
      case 'immedian'
         out = dip_projection('median',in,'tensor');
      case 'imeq'
         indx = repmat({':'},1,ndims(in));
         base = in;
         base.Data = base.Data(:,1,indx{:});
         other = in;
         other.Data = other.Data(:,2,indx{:});
         out = base == other;
         for ii=3:N
            other = in;
            other.Data = other.Data(:,2,indx{:});
            out = out & (base == other);
         end
         out.ColorSpace = '';
      case 'imlargest'
         [~,out] = max(tensortospatial(in),[],2);
         out = reshape(out,imsize(in));
      case 'imsmallest'
         [~,out] = min(tensortospatial(in),[],2);
         out = reshape(out,imsize(in));
      otherwise
         error('Unknown FUNCTION')
   end
else
   % MODE 2
   out = zeros(tensorsize(in));
   switch fun
      case 'isempty'
         out(:) = isempty(in);
      case 'islogical'
         out(:) = islogical(in);
      case 'isreal'
         out(:) = isreal(in);
      case 'ndims'
         out(:) = ndims(in);
      case 'prodofsize'
         out(:) = numpixels(in);
      case 'max'
         out = max(in); % TODO: The projection function don't return an array with the same shape as the input
      case 'mean'
         out = mean(in);
      case 'median'
         out = median(in);
      case 'min'
         out = min(in);
      case 'std'
         out = std(in);
      case 'sum'
         out = sum(in);
      otherwise
         error('Unknown FUNCTION')
   end
end
