%RELABEL   Renumber labels in a labeled image
%  image_out will be like image_in, but all label IDs will be
%  consecutive.
%
% SYNOPSIS:
%  [image_out,lut_out] = relabel(image_in)
%
% OUTPUTS
%  lut_out:  The look-up table used to remap the label image.
%
% SEE ALSO:
%  setlabels, label, dip_measurement.remap

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

function [lab,lut_out] = relabel(lab)

if ~isa(lab,'dip_image')
   lab = dip_image(lab);
end
dt = class(dip_array(lab));

% Find used labels
L = unique(dip_array(lab));
if any(mod(L,1)) | any(L<0)
   error('Input image is not a labeled image (all pixels should be positive integers)')
end

% Remove the background label 0
if L(1)==0
   L(1)=[];
end
if L(end)~=numel(L)
   lut_out = zeros(1,max(L),dt);
   lut_out(L+1) = 1:numel(L);
   lab = lut(lab,lut_out);
end
