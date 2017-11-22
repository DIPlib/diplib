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
%   function: a function handle or function name to execute. The
%      function has as signature:
%                       function(image_in,par1,par2,...)
%   image_in: the input image, with more than one dimension.
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
% NOTE:
%  When using the MATLAB Compiler, make sure to add a %#function
%  pragma to register the function being called by SLICE_OP:
%     %#function bdilation
%     b = slice_op('bdilation',a,5,-1,0);
%
% SEE ALSO:
%  iterate, imarfun, slice_ex, slice_in, im2array, array2im.

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

function out = slice_op(oper,in,varargin)
if ~isa(in,'dip_image')
   in = dip_image(in);
end
sz = imsize(in);
di = numel(sz);
N = sz(di);
isz = sz(1:di-1);
sz(1:di-1) = 1;
indx = repmat({':'},1,di-1);

% Compute first slice and create output
tmp = reshape(in(indx{:},0),isz);
out = feval(oper,tmp,varargin{:});
if  ~isa(out,'dip_image')
   out = dip_image(out);
end
if ndims(out)>di-1
   error('FUNCTION yielded an image with more dimensions than its input');
end
out = repmat(out,sz);

% Compute other slices
for ii = 1:N-1
   tmp = reshape(in(indx{:},ii),isz);
   out(indx{:},ii) = feval(oper,tmp,varargin{:});
end
