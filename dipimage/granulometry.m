%GRANULOMETRY   Obtains a particle size distribution.
%
% The granulometry is a volume-weighted, grey-value--weighted, cumulative
% distribution of object sizes. It can be used to obtain a size distribution
% of the image without attempting to segment and separate individual objects.
% It is computed by a series of openings or closings at different scales. The
% result at each scale is integrated (summed). The obtained series of values
% is scaled such that the value for scale 0 is 0, and for scale infinity is 1.
%
% SYNOPSIS:
%  distr = granulometry(in,mask,scales,type,polarity,options)
%
% OLD-STYLE SYNOPSIS (for backwards compatibility, uses TYPE='isotropic'):
%  distr = granulometry(in,scales,minimumFilterSize,maximumFilterSize,...
%                       minimumZoom,maximumZoom,options,polarity)
%
% PARAMETERS:
%  in:      input grey-value image
%  mask:    mask image, limits where particles are analyzed
%  scales:  scales at which to sample the size distribution
%  type:    type of opening or closing to apply, can be one of the following
%     strings:
%     - 'isotropic': uses openings or closings with a disk structuring
%       element, leading to a width-based size distribution.
%     - 'length':    uses path openings or closings, leading to a length-based
%       size distribution.
%  polarity: 'closing' or 'opening'. 'dark' and 'light' are supported for
%     backwards compatibility. 'closing' analyzes dark objects on a light
%     background, 'opening' analyzes light objects on a dark background.
%  options: cell array containing zero or more of the following strings:
%     - For 'isotropic' granulometries:
%         - 'reconstruction': uses openings or closings by reconstruction
%           instead of structural openings or closings. This leads to objects
%           not being broken up in the same way. Objects need to be clearly
%           separated spatially for this to work.
%         - 'shifted'`: uses sub-pixel shifted isotropic strcuturing elements.
%           This allows a finer sampling of the scale axis (see Luengo et al.,
%           2007). Ignored for images with more than 3 dimensions.
%         - 'interpolate': interpolates by a factor up to 8x for smaller
%           scales, attempting to avoid SE diameters smaller than 8. This
%           improves precision of the result for small scales (see Luengo et
%           al., 2007).
%         - 'subsample': subsamples for larger scales, such that the largest
%           SE diameter is 64. This speeds up computation, at the expense of
%           precision.
%     - For 'length' granulometries:
%         - 'non-constrained': by default, we use constrained path openings
%           or closings, which improves the precision of the measurement, but
%           is a little bit more expensive (see Luengo, 2010). This option
%           causes the use of normal path openings or closings.
%         - 'robust': applies path openings or closings in such a way that
%           they are less sensitive to noise.
%     - For backwards compatibility, 'usecenter' is equivalent to `shifted',
%       and 'usereconstruction' is equivalent to 'reconstruction'. 'verbose'
%       and 'usegrey' options are ignored.
%
%  minimumFilterSize, maximumFilterSize: ignored, these are always 8 and 64.
%  minimumZoom: allowable level of subsampling. If not set to 1, the
%     'subsample' option will be set.
%  maximumZoom: allowable level of interpolation. If not set to 1, the
%     'interpolate' option will be set.
%
% DEFAULTS:
%  scales = sqrt(2).^([1:12])
%  type = 'isotropic'
%  polarity = 'opening' (but 'closing' in the old-style syntax)
%  options = {}
%
% RETURNS:
%  distr: An array containing the points in a cumulative volume-weighted
%     distribution. The first column is SCALES, the second column is the
%     values.
%
% DIPlib:
%  This function calls the DIPlib function dip::Granulometry.

% (c)2018, Cris Luengo.
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

function out = granulometry(varargin)
out = dip_analysis('granulometry',varargin{:});
