%CONVOLVE   General convolution filter
%
% SYNOPSIS:
%  image_out = convolve(image_in,kernel,boundary_condition,method)
%
%  KERNEL can be a image (or an array convertible to an image) or a cell
%  array with vectors. These are different ways of describing a convolution
%  kernel.
%
%  If KERNEL is an image, one of three algorithms is used to compute the
%  convolution. By default, the most efficient algorithm is chosen depending on
%  the size of IMAGE_IN, on the size of KERNEL, and on whether KERNEL is
%  separable or not. The computation time is estimated using simple models
%  fitted to data obtained for one specific computer, these estimates might
%  not be correct for other machines, but should still allow to select a
%  reasonably efficient method. It is always best to measure computation time
%  on the target machine and choose an algorithm explicitly.
%
%  The computation methods are:
%   - Separate the kernel into 1D kernels, and apply them in sequence; this
%     strongly reduces the computational cost, but is only applicable when the
%     kernel is separable.
%   - Direct computation of the convolution sum, only efficient for small
%     kernels.
%   - By multiplication in the Fourier domain. In this case, the image is first
%     padded using the selected boundary condition, and the kernel is padded
%     with zeros to the same size. The FFT is applied to both, the results
%     multiplied together, and the resul is inverse transformed. If both inputs
%     are real-valued, the output will be real-valued as well.
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
%  of the image domain. See HELP BOUNDARY_CONDITION.
%
%  METHOD is one of 'best', 'separable', 'direct' or 'fourier', and selects
%  the method to use to compute the convolution in the case of KERNEL being an
%  image. 'best' is the default, and causes the behavior described above for
%  the image KERNEL. The other three select a method. If 'separable', but KERNEL
%  is not separable, an error is thrown.
%
%  The METHOD parameter is ignored if KERNEL is not an image, but represents an
%  already separated kernel.
%
% DEFAULTS:
%  bounary_condition = 'mirror'
%  method = 'best'
%
% ADVANCED USAGE:
%  KERNEL can also be a struct array with fields 'filter', 'origin' and 'flags'.
%  Each array element describes a 1D filter for a different dimension (just
%  like the cell array documented above). There should be as many vectors as
%  dimensions in IMAGE_IN, or only one to apply it to each dimension. The
%  'filter' field is required, and contains a vector with the filter weights
%  (as in the cell array). The 'origin' field is the index into 'filter' that is
%  the origin of the filter. If it is negative or not given, the origin will be
%  placed at the middle of the filter, or the pixel to the right if the size is
%  even. The 'flags' field contains one of the strings 'general' (or '', or not
%  present), 'even', 'odd', 'conj', 'd-even', 'd-odd' or 'd-conj'. These cause
%  the filter weights to be repeated to obtain an even, odd or conjugate
%  symmetry in the filter. The 'd-' prefix creates an even-sized filter by
%  repeating all weights, otherwise an odd-sized filter is created, in which
%  the right-most weight is not repeated.
%
% DIPlib:
%  This function calls the DIPlib functions <a href="https://diplib.org/diplib-docs/linear.html#dip-Convolution-Image-CL-Image-CL-Image-L-String-CL-StringArray-CL">dip::Convolution</a> or
%  <a href="https://diplib.org/diplib-docs/linear.html#dip-SeparableConvolution-Image-CL-Image-L-OneDimensionalFilterArray-CL-StringArray-CL-BooleanArray-">dip::SeparableConvolution</a>, depending on the input.

% (c)2017-2022, Cris Luengo.
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
out = dip_filtering('convolve',varargin{:});
