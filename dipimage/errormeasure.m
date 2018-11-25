%ERRORMEASURE   Compares two images
%
% SYNOPSIS:
%  error = errormeasure(image_in,image_reference,mask,method)
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
%
%    - 'Dice':         Dice coefficient
%    - 'Jaccard':      Jaccard index
%    - 'Specificity':  Specificity (True Negative Rate)
%    - 'Sensitivity':  Sensitivity or recall (True Positive Rate)
%    - 'Accuracy':     Fraction of identical values
%    - 'Precision':    Precision (Positive Predictive Value)
%    - 'Hausdorff':    Hausdorff distance (binary images only)
%
% DEFAULTS:
%  method = 'MSE'
%
% NOTES:
%  The methods in the first block ('MSE' through 'MutualInformation') work on
%  any real-valued images, as well as complex-valued images unless otherwise
%  stated; most work also on multi-valued (tensor) images.
%
%  The methods in the second block compare segmentations. The input images are
%  either binary images, or real-valued images in the range [0,1]. If the images
%  contain values outside of that range, the output will be nonsensical. Images
%  must be scalar.
%
% DIPlib:
%  This function calls the DIPlib functions dip::MeanError, dip::MeanSquareError,
%  dip::RootMeanSquareError, dip::MeanAbsoluteError, dip::IDivergence,
%  dip::InProduct, dip::LnNormError, dip::PSNR, dip::SSIM, dip::MutualInformation,
%  dip::DiceCoefficient, dip::JaccardIndex, dip::Specificity, dip::Sensitivity,
%  dip::Accuracy, dip::Precision, dip::HausdorffDistance.
%  See the documentation of these functions for more information.
%
% LITERATURE:
%  - I. Csiszar, "Why Least Squares and Maximum Entropy? An axiomatic approach to
%    inference for linear inverse problems", The Annals of Statistics 19:2032-2066,
%    1991.
%  - Z. Wang, A.C. Bovik, H.R. Sheikh and E.P. Simoncelli, "Image quality assessment:
%    from error visibility to structural similarity", IEEE Transactions on Image
%    Processing 13(4):600-612, 2004.

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

function out = errormeasure(varargin)
out = dip_math('errormeasure',varargin{:});
