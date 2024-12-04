%ERRORMEASURE   Compares two images
%
% SYNOPSIS:
%  error = errormeasure(image_in,image_reference,mask,method)
%  error = errormeasure(image_in,image_reference,method)
%
% PARAMETERS:
%  image_reference: Noiseless image to compare against
%  mask:            Optionally selects which pixels to use in comparison
%  method:          Error measure. One of:
%    - 'MSE':          Mean square error
%    - 'RMSE':         Root mean square error
%    - 'ME':           Mean error (real-valued images only)
%    - 'MAE':          Mean absolute error
%    - 'IDivergence':  I-divergence measure from Csiszar (1991) (real-valued
%                      images only)
%    - 'InProduct':    Cross-correlation for zero translation (sum of product)
%                      (real-valued images only)
%    - 'LnNormError':  2nd order norm difference
%    - 'PSNR':         Peak signal-to-noise ratio, in dB.
%    - 'SSIM':         Structural similarity index from Wang et al. (2004)
%                      (real-valued images only)
%    - 'MutualInformation': Mutual information in bits (real-valued scalar
%                           images only)
%    - 'Pearson':      Pearson correlation coefficient (real-valued images only)
%    - 'Spearman':     Spearman rank correlation coefficient (real-valued images
%                      only)
%
%    - 'Dice':         Dice coefficient
%    - 'Jaccard':      Jaccard index
%    - 'Specificity':  Specificity (True Negative Rate)
%    - 'Sensitivity':  Sensitivity or recall (True Positive Rate)
%    - 'Accuracy':     Fraction of identical values
%    - 'Precision':    Precision (Positive Predictive Value)
%    - 'Hausdorff':    Hausdorff distance (binary images only)
%    - 'ModifiedHausdorff': Modified Hausdorff distance (binary images only)
%    - 'SMD':          Sum of minimal distances (binary images only)
%    - 'CWSMD':        Complement weighted sum of minimal distances from
%                      Ćurić et al. (2014) (binary images only)
%
% DEFAULTS:
%  method = 'MSE'
%
% NOTES:
%  The methods in the first block ('MSE' through 'Spearman') work on
%  any real-valued images, as well as complex-valued images unless otherwise
%  stated; most work also on multi-valued (tensor) images.
%
%  The methods in the second block compare segmentations. The input images are
%  either binary images, or real-valued images in the range [0,1]. If the images
%  contain values outside of that range, the output will be nonsensical. Images
%  must be scalar. MASK is ignored.
%
% DIPlib:
%  This function calls the DIPlib functions <a href="https://diplib.org/diplib-docs/math_error.html#dip-MeanError-Image-CL-Image-CL-Image-CL">dip::MeanError</a>, <a href="https://diplib.org/diplib-docs/math_error.html#dip-MeanSquareError-Image-CL-Image-CL-Image-CL">dip::MeanSquareError</a>,
%  <a href="https://diplib.org/diplib-docs/math_error.html#dip-RootMeanSquareError-Image-CL-Image-CL-Image-CL">dip::RootMeanSquareError</a>, <a href="https://diplib.org/diplib-docs/math_error.html#dip-MeanAbsoluteError-Image-CL-Image-CL-Image-CL">dip::MeanAbsoluteError</a>, <a href="https://diplib.org/diplib-docs/math_error.html#dip-IDivergence-Image-CL-Image-CL-Image-CL">dip::IDivergence</a>,
%  <a href="https://diplib.org/diplib-docs/math_error.html#dip-InProduct-Image-CL-Image-CL-Image-CL">dip::InProduct</a>, <a href="https://diplib.org/diplib-docs/math_error.html#dip-LnNormError-Image-CL-Image-CL-Image-CL-dfloat-">dip::LnNormError</a>, <a href="https://diplib.org/diplib-docs/math_error.html#dip-PSNR-Image-CL-Image-CL-Image-CL-dfloat-">dip::PSNR</a>, <a href="https://diplib.org/diplib-docs/math_error.html#dip-SSIM-Image-CL-Image-CL-Image-CL-dfloat--dfloat--dfloat-">dip::SSIM</a>, <a href="https://diplib.org/diplib-docs/math_error.html#dip-MutualInformation-Image-CL-Image-CL-Image-CL-dip-uint-">dip::MutualInformation</a>,
%  <a href="https://diplib.org/diplib-docs/math_statistics.html#dip-PearsonCorrelation-Image-CL-Image-CL-Image-CL">dip::PearsonCorrelation</a>, <a href="https://diplib.org/diplib-docs/math_statistics.html#dip-SpearmanRankCorrelation-Image-CL-Image-CL-Image-CL">dip::SpearmanRankCorrelation</a>,
%  <a href="https://diplib.org/diplib-docs/math_error.html#dip-DiceCoefficient-Image-CL-Image-CL">dip::DiceCoefficient</a>, <a href="https://diplib.org/diplib-docs/math_error.html#dip-JaccardIndex-Image-CL-Image-CL">dip::JaccardIndex</a>, <a href="https://diplib.org/diplib-docs/math_error.html#dip-Specificity-Image-CL-Image-CL">dip::Specificity</a>, <a href="https://diplib.org/diplib-docs/math_error.html#dip-Sensitivity-Image-CL-Image-CL">dip::Sensitivity</a>,
%  <a href="https://diplib.org/diplib-docs/math_error.html#dip-Accuracy-Image-CL-Image-CL">dip::Accuracy</a>, <a href="https://diplib.org/diplib-docs/math_error.html#dip-Precision-Image-CL-Image-CL">dip::Precision</a>, <a href="https://diplib.org/diplib-docs/math_error.html#dip-HausdorffDistance-Image-CL-Image-CL">dip::HausdorffDistance</a>,
%  <a href="https://diplib.org/diplib-docs/math_error.html#dip-ModifiedHausdorffDistance-Image-CL-Image-CL">dip::ModifiedHausdorffDistance</a>, <a href="https://diplib.org/diplib-docs/math_error.html#dip-SumOfMinimalDistances-Image-CL-Image-CL">dip::SumOfMinimalDistances</a>,
%  <a href="https://diplib.org/diplib-docs/math_error.html#dip-ComplementWeightedSumOfMinimalDistances-Image-CL-Image-CL">dip::ComplementWeightedSumOfMinimalDistances</a>.
%  See the documentation of these functions for more information.
%
% LITERATURE:
%  - I. Csiszar, "Why Least Squares and Maximum Entropy? An axiomatic approach to
%    inference for linear inverse problems", The Annals of Statistics 19:2032-2066,
%    1991.
%  - Z. Wang, A.C. Bovik, H.R. Sheikh and E.P. Simoncelli, "Image quality assessment:
%    from error visibility to structural similarity", IEEE Transactions on Image
%    Processing 13(4):600-612, 2004.
%  - V. Ćurić, J. Lindblad, N. Sladoje, H. Sarve, and G. Borgefors, "A new set
%    distance and its application to shape registration", Pattern Analysis and
%    Applications 17:141-152, 2014.

% (c)2017-2019, Cris Luengo.
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

function out = errormeasure(varargin)
out = dip_math('errormeasure',varargin{:});
