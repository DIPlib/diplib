%RADONCIRCLE   Compute Radon transform to find circles/spheres
%
% SYNOPSIS:
%  [rt,p] = radoncircle(in,radii,sigma,threshold,mode,options)
%
% PARAMETERS:
%  radii:     Range of radii to search, in the form [START,STOP,STEP]. For
%             example, [30,45,2] means from 30 to 45 pixels radius, both
%             included, and in steps of 2 pixels. STEP defaults to 1.
%  sigma:     Regularization parameter. Should be at least equal to STEP
%             in RADII.
%  threshold: Height of peak above local valley for detected maxima in
%             parameter space.
%  mode:      How to compute the parameters space. One of:
%     - 'full': RT is the full parameter space.
%     - 'projection': RT is two images (in the form of two tensor elements),
%       containing the max and argmax along the r-axis.
%     - 'subpixel projection': Idem, but the argmax is computed with sub-pixel
%       precision.
%  options:   Cell array containing one or more of:
%     - 'normalize': Normalizes to prevents a bias towards larger circles.
%     - 'correct': If normalized, adds a correction to reduce bias in the
%       radius estimate.
%     - 'hollow': Adds a negative ring just inside the positive ring of the
%       template. This forces the algorithm to look for rings, not disks.
%     - 'filled': Fills the positive ring with negative values. This forces
%       the algorithm to look for rings without anything in them.
%     - 'no maxima detection': P is an empty array.
%     - 'no parameter space': RT is an empty array. If MODE is 'full', then
%       the parameter space is computed in chunks to save memory.
%
% OUTPUTS:
%  rt:  Radon transform space, depending on MODE parameter.
%  p:   Parameters of the N found circles as an NxM matrix, with each row
%       containing [X,Y,Z,...,R].
%
% DEFAULTS:
%  radii = [10,30]
%  sigma = 1
%  threshold = 1
%  mode = 'full'
%  options = {'normalize','correct'}
%
% NOTES:
%  The parameter space RT has the same size as IN. In the case of MODE=='full'
%  it has one additional dimension, with a size given by RADII. In the other
%  modes it doesn't have the additional dimension, but it has two tensor
%  elements (channels).
%
%  It is important for RADII to exceed the expected radii of circles in the
%  image, because if a local maximum is on the edge of the image, it cannot
%  be detected. For example, if circles are in the range [24,32], specify
%  at least [23,33] for RADII.
%
% EXAMPLE:
%  img = gaussianlineclip(rr-30.3) + gaussianlineclip(rr-50.8);
%  img = noise(img + shift(img,[10,-30]),'gaussian',0.1)
%  [rt,p] =  radoncircle(img,[24,56],1,0.4,'full',{'normalize','correct','hollow'})
%
% LITERATURE:
% - C.L. Luengo Hendriks, M. van Ginkel, P.W. Verbeek and L.J. van Vliet,
%   "The generalized Radon transform: sampling, accuracy and memory
%   considerations", Pattern Recognition 38(12):2494–2505, 2005.
% - C.L. Luengo Hendriks, M. van Ginkel and L.J. van Vliet,
%   "Underestimation of the radius in the Radon transform for circles and
%   spheres", Technical Report PH-2003-02, Pattern Recognition Group, Delft
%   University of Technology, The Netherlands, 2003.

% (c)2018-2019, Cris Luengo.
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

function varargout = radoncircle(varargin)
varargout = cell(1,max(nargout,1));
[varargout{:}] = dip_segmentation('radoncircle',varargin{:});
