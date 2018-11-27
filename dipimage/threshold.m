%THRESHOLD   Thresholding
%
% SYNOPSIS:
%  [out,th_value] = threshold(in,type,parameter)
%
% PARAMETERS:
%  type: string, one of:
%     'isodata':    Thresholding using the Isodata algorithm
%                   by Ridler and Calvard (1978).
%     'otsu':       Thresholding using maximal inter-class variance method
%                   by Otsu (1979).
%     'minerror':   Thresholding using minimal error method
%                   by Kittler and Illingworth (1986).
%     'triangle':   Thresholding using chord method
%                   (a.k.a. skewed bi-modality, maximum distance to triangle)
%                   by Zack, Rogers and Latt (1977).
%     'background': Thresholding using unimodal background-symmetry method.
%     'fixed':      Thresholding at a fixed value.
%     'double':     Thresholding between two fixed values.
%     'volume':     Thresholding to obtain a given volume fraction.
%     'hysteresis': From the binary image (in>low) only those regions are
%                   selected for which at least one pixel is (in>high).
%
%  parameter ('isodata'):    The number of thresholds to compute. If not
%                            equal to 1, the output image is a labeled image
%                            rather than binary. Inf means 1.
%            ('background'): Distance to the peak where we cut off, in
%                            terms of the half-width at half the maximum.
%                            Inf selects the default value, which is 2.
%            ('fixed'):      Threshold value. Inf means halfway between
%                            minimum and maximum value. Use a vector with
%                            multiple values to produce a labeled image
%                            with multiple classes.
%            ('volume'):     Parameter = the volume fraction. Inf means 0.5.
%            ('double'):     Two threshold values. Inf means
%                            min+[1/3,2/3]*(max-min).
%            ('hysteresis'): Two values: [low,high]. Inf means
%                            min+[1/3,2/3]*(max-min).
%
% DEFAULT:
%  type:      'isodata'
%  parameter: Inf (means: use default for method)
%
% DIPlib:
%  This function calls the DIPlib functions dip::Threshold,
%  fip::IsodataThreshold, dip::RangeThreshold and dip::HysteresisThreshold.

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

function varargout = threshold(varargin)
varargout = cell(1,max(nargout,1));
[varargout{:}] = dip_segmentation('threshold',varargin{:});
