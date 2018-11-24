%AREAOPENING   Area opening or closing
%
% SYNOPSIS:
%  image_out = areaopening(image_in,filterSize,connectivity,polarity)
%
% PARAMETERS:
%  filterSize:   the size (area) of the structuring element.
%  connectivity: defines the metric, that is, the shape of the structuring
%     element.
%     * 1 indicates city-block metric, or a diamond-shaped S.E in 2D.
%     * 2 indicates chessboard metric, or a square structuring element in 2D.
%     For 3D images use 1, 2 or 3.
%  polarity:     'opening' or 'closing'.
%
% DEFAULTS:
%  filterSize = 50
%  connectivity = 1
%  polarity = 'opening'
%
%  The area opening is an opening over all possible structuring elements of a
%  specific area (number of pixels) given by FILTERSIZE. The structuring element
%  is always a connected set, under CONNECTIVITY.
%
%  The result is an image where the local maxima with an area smaller than the
%  FILTERSIZE have been removed.
%
%  If POLARITY is 'closing', the closing is computed instead.
%
% DIPlib:
%  This function calls the DIPlib function dip::AreaOpening.

% (c)2017-2018, Cris Luengo.
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

function out = areaopening(varargin)
out = dip_morphology('areaopening',varargin{:});
