%SLICE_OP   Apply a function to all slices along the last image dimension
%
%  This function is useful for operations that intrinsically do
%  not allow a no-op along one dimension, such as BDILATION in the
%  example below. It applies the function to each of the (N-1)-dimensional
%  slices of the N-dimensional image. This process works even if the
%  function produces an output that is of a different size or
%  dimensionality than the input, as long as there are not more dimensions
%  produced.
%
% SYNOPSIS:
%  image_out = slice_op(function,image_in,par1,par2,...)
%
% PARAMETERS:
%  function: a function handle or function name to execute. The
%     function has as signature: function(image_in,par1,par2,...)
%  image_in: the input image, with more than one dimension.
%
% EXAMPLE:
%  a = threshold(readim('chromo3d'));
%  b = slice_op(@bdilation,a,5,-1);
%
% NOTE:
%  The FUNCTION must take the input image as its first argument. If any
%  other arguments are images, they are not sliced, but passed directly to
%  the function.
%
% SEE ALSO:
%  dip_image/iterate, dip_image/tensorfun, dip_image/slice_ex,
%  dip_image/slice_in, im2array, array2im.

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

function out = slice_op(fun,in,varargin)

if ~any(strcmp(class(fun),{'char','function_handle'}))
   error('First parameter should be a string or function handle')
end
if ~isa(in,'dip_image')
   in = dip_image(in);
end

sz = imsize(in);
di = numel(sz);
N = sz(di);
if N==1
   out = feval(fun,in,varargin{:});
   return;
   % We're now guaranteed that in.TrailingSingletons==0
end
isz = sz(1:di-1);
sz(1:di-1) = 1;
indx = repmat({':'},1,di+1);

% Compute first slice and create output
tmp = in;
tmp.Data = tmp.Data(indx{:},1); % Dimensionality is adjusted automatically, since we don't have trailing singletons.
out = feval(fun,tmp,varargin{:});
if ~isa(out,'dip_image')
   out = dip_image(out);
end
if ndims(out)>di-1
   error('FUNCTION yielded an image with more dimensions than its input');
end
out = repmat(out,sz);

% Compute other slices
for ii = 2:N
   tmp = in;
   tmp.Data = tmp.Data(indx{:},ii);
   tmp = feval(fun,tmp,varargin{:});
   if ~isa(tmp,'dip_image')
      tmp = dip_image(tmp);
   end
   out.Data(indx{:},ii) = tmp.Data;
end
