%CONVOLVE   General convolution filter
%
% SYNOPSIS:
%  image_out = convolve(image_in,kernel,boundary_condition)
%
%  KERNEL can be a image (or an array convertible to an image) or a cell
%  array with vectors. These are different ways of describing a convolution
%  kernel.
%
%  If KERNEL is an image, and it is separable, the convolution is computed
%  in the spatial domain using NDIMS(IMAGE_IN) one-dimensional convolutions.
%  If it is not separable, the convolution is computed either through the
%  Fourier Domain or by direct implementation of the convolution sum,
%  depending on the size of the kernel. Currently, a kernel with more than
%  49 elements (i.e. larger than a 7x7 kernel) is computed through the
%  Fourier Domain.
%  For example:
%      a = readim('cermet');
%      k = ones(5,5);
%      convolve(a,k)
%
%  If KERNEL is a cell array with vectors, each vector describes a 1D filter
%  that is applied to a different image dimension. There should be as many
%  vectors as dimensions in IMAGE_IN, or only one to apply it to each dimension.
%  For example:
%      a = readim('cermet');
%      convolve(a,{[1,1,1],[-1,0,1]})
%  In this case, an empty vector or a vector [1] causes that dimension to not
%  be processed.
%
%  In all cases, the output is the same size as the input image.
%
%  BOUNDARY_CONDITION is a string or a cell array of strings (one per image
%  dimension) specifying how the convolution handles pixel values outside
%  of the image domain. See HELP BOUNDARY_CONDITION. It is ignored if the
%  convolution is best computed through the Fourier Domain, where the boundary
%  condition is always 'periodic' by construction.
%
% DEFAULTS:
%  bounary_condition = 'mirror'
%
% NOTE:
%  See the user guide for the available boundary condition strings.
%
% ADVANCED USAGE:
%  KERNEL can also be a struct array with fields 'filter', 'origin' and
%  'flags'. Each array element describes a 1D filter for a different
%  dimension (just like the cell array documented above). There should
%  be as many vectors as dimensions in IMAGE_IN, or only one to apply it
%  to each dimension. The 'filter' field is required, and contains a vector
%  with the filter weights (as in the cell array). The 'origin' field is
%  the index into 'filter' that is the origin of the filter. If it is
%  negative or not given, the origin will be placed at the middle of the
%  filter, or the pixel to the right if the size is even. The 'flags' field
%  contains one of the strings 'general' (or '', or not present), 'even',
%  'odd', 'conj', 'd-even', 'd-odd' or 'd-conj'. These cause the filter weights to be
%  repeated to obtain an even, odd or conjugate symmetry in the filter. The 'd-'
%  prefix creates an even-sized filter by repeating all weights, otherwise
%  an odd-sized filter is created, in which the right-most weight is not
%  repeated.
%
% DIPlib:
%  This function calls the DIPlib functions dip::SeparableConvolution,
%  dip::ConvolveFT or dip::GeneralConvolution, depending on the input.

% (c)2017-2018, Cris Luengo.
% Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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

function out = convolve(varargin)
out = filtering('convolve',varargin{:});
