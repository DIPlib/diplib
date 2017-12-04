%MSR2OBJ   Label each object in the image with its measurement
%
% SYNOPSIS:
%  image_out = msr2obj(image_in,msr,feature,value)
%
% PARAMETERS:
%  image_in:      labelled image containing the objects.
%  msr:           measurement object derived from IMAGE_IN.
%  feature:       name of one of the features in MSR.
%  value:         the component of the feature, if it has more than one.
%
% DEFAULTS:
%  feature = fieldnames(msr){1} (i.e. the first feature)
%  value = 1;

% (c)2017, Cris Luengo.
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

function image_out = msr2obj(image_in,msr,msrID,msrDim)
type = class(dip_array(image_in));
if ~strncmp(type,'uint',4)
   error('Input image must be labelled.')
end
if nargin<4
   msrDim = 1;
end
if nargin<3
   msrID = fieldnames(msr);
   msrID = msrID{1};
end
newlabs = msr.(msrID)(:,msrDim);
table = zeros(max(image_in)+1,1,'single');
table(msr.id+1) = newlabs;
image_out = lut(image_in,table);
