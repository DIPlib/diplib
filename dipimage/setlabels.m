%SETLABELS   Remap or remove labels
%
% SYNOPSIS:
%  [image_out,lut_out] = setlabels(image_in,labels,newval)
%
% PARAMETERS:
%  image_in: Input image
%  labels:   The labels to be changed
%  newval:   If scalar: the affected labels are set to NEWVAL.
%            If array:  must be of the same length as LABELS, each label
%                       in LABELS is set to the corresponding value in NEWVAL.
%            'clear':   the affected labels are set to zero. The remaining
%                       labels are renumbered consecutively.
%
% OUTPUT
%  lut_out:  The look-up table used to remap the label image.
%
% SEE ALSO:
%  relabel, label, dip_measurement.remap

% (c)2008, Michael van Ginkel.
% (c)2018, Cris Luengo.
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

function [lab,lut_out] = setlabels(lab,labels,value)

if ~isa(lab,'dip_image')
   lab = dip_image(lab);
end
dt = class(dip_array(lab));

lut_out = zeros(1,dt):max(lab);
if ischar(value)
   if strcmp(value,'clear')
      lut_out(labels+1) = 0;
      lut_out(lut_out>0) = 1:nnz(lut_out);
   else
      error(['Unknown flag ',value]);
   end
else
   lut_out(labels+1) = value;
end

lab = lut(lab,lut_out);
