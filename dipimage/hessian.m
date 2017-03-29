%HESSIAN   Hessian matrix of an image
%
% SYNOPSIS:
%  image_out = gradientvector(image_in,sigma,method,boundary_condition,truncation)
%
%  IMAGE_IN is a real-valued, scalar image with N dimensions.
%  IMAGE_OUT is a N-by-N tensor image, where each image component
%  is a second-order Gaussian derivative. That is, each
%  pixel contains the Hessian matrix of the image at that point.
%
%  See DERIVATIVE for a description of the parameters.
%
% DIPlib:
%  This function calls the DIPlib function dip::Hessian.

% (c)2017, Cris Luengo.
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
