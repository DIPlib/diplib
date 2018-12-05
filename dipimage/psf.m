%PSF   Creates an image with an incoherent PSF or OTF
%
% SYNOPSIS:
%  image_out = psf(sizes,method,oversampling,amplitude,defocus)
%
% PARAMETERS:
%  sizes:   The sizes of the output image.
%  method:  What to create, and how:
%    - 'PSF':      Create a point spread function
%    - 'OTF':      Create an optical transfer function using the formulation
%                  by Stokseth.
%    - 'Stokseth': Same as 'OTF'
%    - 'Hopkins':  Create an optical transfer function using the formulation
%                  by Hopkins.
%  oversampling: The oversampling rate, values larger than 1 yield
%                oversampled functions.
%  amplitude:    The amplitude.
%  defocus:      Defined as the maximum defocus path length error divided by
%                the wave length. Not used for the PSF.
%
% DEFAULTS:
%  sizes = [], which translates to [256,256] for OTF, and
%                                  [19,19]*oversampling for PSF
%  method = 'PSF'
%  oversampling = 1
%  amplitude = 1
%  defocus = 0
%
% NOTE:
%  If DEFOCUS is nonzero, 'Stokseth' or 'Hopkins' methods will be used.
%
% DIPlib:
%  This function calls the DIPlib functions dip::IncoherentPSF and
%  dip::IncoherentOTF.

% (c)2018, Cris Luengo.
% Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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

function out = psf(varargin)
out = dip_microscopy('psf',varargin{:});
