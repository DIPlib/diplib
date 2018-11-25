%SELECT   Selects pixels from one image or another depending on a condition
%
% SYNOPSIS:
%  image_out = select(img1,img2,mask)
%  image_out = select(imgA,imgB,img1,img2,selector)
%
%  IMG1 and IMG2 are the source images. IMAGE_OUT will equal IMG1 for those
%  samples where the condition is true, and will equal IMG2 otherwise.
%
%  MASK is a binary image, where MASK is set, the condition is true.
%
%  SELECTOR is a comparison operator that defines the condition: IMGA <SELECTOR> IMGB.
%  These are the strings available: '==', '!=', '>', '<', '>=', '<='. '~=' is a
%  MATLAB-specific alias for '!='.
%
%  All images are singleton-expanded to a common size.
%
% DIPlib:
%  This function calls the two forms of the DIPlib function dip::Select.

% (c)2017-2018, Cris Luengo.
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

function out = select(varargin)
out = dip_math('select',varargin{:});
