%THRESHOLD   Thresholding
%
% SYNOPSIS:
%  [out,value] = threshold(in,type,parameter)
%  [out,value] = threshold(in,mask,type,parameter)
%
% PARAMETERS:
%  mask:      limits the region used to determine the threshold. The
%             threshold itself is applied to the image as a whole.
%  type:      'isodata', 'otsu', 'minerror', 'triangle', 'background',
%             'fixed', 'double', 'volume', or 'hysteresis'.
%  parameter: parameter to the algorithm, see below. Inf selects the
%             default value.
%
%  value: The second output argument is the selected threshold value.
%
% DEFAULT:
%  mask:      []
%  type:      'isodata'
%  parameter: Inf
%
% THRESHOLD METHODS:
%  - isodata: Thresholding using the Isodata algorithm by Ridler and Calvard
%    (1978). PARAMETER is the number of thresholds to compute. If not equal to
%    1, the output image is a labeled image rather than binary. Default is 1.
%
%  - otsu: Thresholding using maximal inter-class variance method by Otsu
%    (1979). PARAMETER is ignored.
%
%  - minerror: Thresholding using minimal error method by Kittler and
%    Illingworth (1986). PARAMETER is ignored.
%
%  - gmm: Thresholding using a Gaussian Mixture Model. PARAMETER is the number
%    of thresholds to compute. Fits PARAMETER+1 Gaussians to the histogram,
%    then finds thresholds to optimally separate those Gaussians. PARAMETER
%    defaults to 1.
%
%  - triangle: Thresholding using chord method (a.k.a. skewed bi-modality,
%    maximum distance to triangle) by Zack, Rogers and Latt (1977). PARAMETER
%    is the sigma of the smoothing applied (sigma measured in histogram bins).
%    Default is 4.
%
%  - background: Thresholding using unimodal background-symmetry method.
%    PARAMETER is the distance to the peak where we cut off, in terms of the
%    half-width at half the maximum. Default is 2.
%
%  - fixed: Thresholding at a fixed value. PARAMETER is the threshold value.
%    Default is halfway between minimum and maximum value. Use a vector with
%    multiple values to produce a labeled image with multiple classes.
%    Ignores MASK parameter.
%
%  - double: Thresholding between two fixed values, given by the two values
%    in PARAMETER. Default is min+[1/3,2/3]*(max-min).
%    Ignores MASK parameter if PARAMETER is given.
%
%  - volume: Thresholding to obtain a set volume fraction, specified through
%    PARAMETER.
%
%  - hysteresis: From the binary image (in>low) only those regions are
%    selected for which at least one pixel is (in>high). PARAMETER contains
%    two values: [low,high]. Default is min+[1/3,2/3]*(max-min).
%    Ignores MASK parameter if PARAMETER is given.
%
% DIPlib:
%  This function calls the DIPlib functions <a href="https://diplib.org/diplib-docs/segmentation.html#dip-Threshold-Image-CL-Image-CL-Image-L-String-CL-dfloat-">dip::Threshold</a>,
%  <a href="https://diplib.org/diplib-docs/segmentation.html#dip-IsodataThreshold-Image-CL-Image-CL-Image-L-dip-uint-">dip::IsodataThreshold</a>, <a href="https://diplib.org/diplib-docs/segmentation.html#dip-GaussianMixtureModelThreshold-Image-CL-Image-CL-Image-L-dip-uint-">dip::GaussianMixtureModelThreshold</a>,
%  <a href="https://diplib.org/diplib-docs/segmentation.html#dip-RangeThreshold-Image-CL-Image-L-dfloat--dfloat--String-CL-dfloat--dfloat-">dip::RangeThreshold</a> and <a href="https://diplib.org/diplib-docs/segmentation.html#dip-HysteresisThreshold-Image-CL-Image-L-dfloat--dfloat-">dip::HysteresisThreshold</a>.

% (c)2017-2020, Cris Luengo.
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

function varargout = threshold(varargin)
varargout = cell(1,max(nargout,1));
[varargout{:}] = dip_segmentation('threshold',varargin{:});
