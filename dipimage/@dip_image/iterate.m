%ITERATE   Apply a function to all tensor components of an image
%
%  This function is useful for operations that intrinsically only
%  work with scalar images, such as DILATION in the example below.
%  It applies the function to each of the tensor elements (channels).
%  This process works even if the function produces an output that
%  is of a different size or dimensionality than the input, as long
%  as the output is a scalar image.
%
% SYNOPSIS:
%  image_out = iterate(function,par1,par2,...)
%
% PARAMETERS:
%  function: a function handle or function name to execute.
%  parX:     parameters to FUNCTION.
%
% EXAMPLE:
%  a = readim('flamingo');
%  b = iterate(@dilation,a,5);
%
% NOTE:
%  Each non-scalar image passed as parameter (PAR1, PAR2, etc) must have
%  the same tensor sizes. IMAGE_OUT will also have the same tensor sizes.
%  Each tensor element IMAGE_OUT{I,J} will be computed by:
%     FUNCTION(PAR1{I,J},PAR2{I,J},...)
%  Parameters that are not non-scalar images are passed identically to
%  each call (this includes arrays as well as scalar images).
%
%  Note that a color image is a tensor image. Color space information is
%  kept if possible. There is no support yet for applying a filter in
%  selected channels only.
%
% SEE ALSO:
%  dip_image/slice_op, dip_image/tensorfun, im2array, array2im.

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

function out = iterate(fun,varargin)

if ~any(strcmp(class(fun),{'char','function_handle'}))
   error('First parameter should be a string or function handle')
end

n = length(varargin);
istensor = false(n,1);
tsz = [1,1];
tsh = 'column vector';
telem = 1;
elementwise = true;
anytensor = false;
colsp = '';
for kk=1:n
   if isa(varargin{kk},'dip_image') && numtensorel(varargin{kk})>1
      istensor(kk) = true;
      if ~anytensor
         anytensor = true;
         tsz = tensorsize(varargin{kk});
         tsh = varargin{kk}.TensorShape;
         telem = numtensorel(varargin{kk});
         colsp = colorspace(varargin{kk});
      else
         if ~isequal(tensorsize(varargin{kk}),tsz)
            error('All non-scalar images should have the same tensor size');
         end
         if telem ~= numtensorel(varargin{kk})
            elementwise = false;
         end
         if isempty(colsp)
            colsp = colorspace(varargin{kk}); % Using the colorspace of the first color image
         end
      end
   end
end

% Compute first slice
args = varargin;
for kk=1:n
   if istensor(kk)
      tmp = varargin{kk};
      indx = repmat({':'},1,ndims(tmp));
      tmp.Data = tmp.Data(:,1,indx{:});
      args{kk} = tmp;
   end
end
tmp = feval(fun,args{:});
if ~isa(tmp,'dip_image')
   tmp = dip_image(tmp);
end
if ~isscalar(tmp)
   error('FUNCTION yielded a non-scalar image');
end

% Create output
if elementwise
   if isempty(colsp)
      out = clone(tmp,'tensorsize',tsz,'tensorshape',tsh);
   else
      out = clone(tmp,'colorspace',colsp);
      if numtensorel(out) ~= telem
         error('An input image has a color space that doesn''t match it''s tensor size')
      end
   end
else
   out = clone(tmp,'tensorsize',tsz,'tensorshape','column-major matrix');
end
indx = repmat({':'},1,ndims(out));
out.Data(:,1,indx{:}) = tmp.Data;

% Compute other slices
if elementwise
   for ii=2:telem
      for kk=1:n
         if istensor(kk)
            tmp = varargin{kk};
            indx = repmat({':'},1,ndims(tmp));
            tmp.Data = tmp.Data(:,ii,indx{:});
            args{kk} = tmp;
         end
      end
      tmp = feval(fun,args{:});
      if ~isa(tmp,'dip_image')
         tmp = dip_image(tmp);
      end
      indx = repmat({':'},1,ndims(out));
      out.Data(:,ii,indx{:}) = tmp.Data;
   end
else
   tlookup = cell(1,n);
   for kk=1:n
      if istensor(kk)
         tlookup{kk} = dip_tensor_indices(varargin{kk}) + 1;
      end
   end
   for jj=1:tsz(2)
      for ii=1:tsz(1)
         if jj*ii>1 % skip first element
            I = (jj-1)*tsz(1) + ii;
            for kk=1:n
               if istensor(kk)
                  tmp = varargin{kk};
                  indx = repmat({':'},1,ndims(tmp));
                  J = tlookup{kk}(I);
                  if J<1
                     s = size(tmp.Data);
                     s(2) = 1;
                     tmp.Data = zeros(s);
                  else
                     tmp.Data = tmp.Data(:,J,indx{:});
                  end
                  args{kk} = tmp;
               end
            end
            tmp = feval(fun,args{:});
            if ~isa(tmp,'dip_image')
               tmp = dip_image(tmp);
            end
            indx = repmat({':'},1,ndims(out));
            out.Data(:,I,indx{:}) = tmp.Data;   % table lookup would yield I
         end
      end
   end
end
