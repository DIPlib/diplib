%SUBSAMPLE   Subsample an image
%
% SYNOPSIS:
%  image_out = subsample(image_in,subsample_factor)
%
% PARAMETERS:
%  subsample_factor = integer array containing the subsampling
%
% DEFAULT:
%  subsample_factor = 2
%
% SEE ALSO:
%  resample, rebin

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

function in = subsample(in,step)
if nargin<2 || isempty(step)
   step = 2;
elseif ~isnumeric(step) || any(mod(step,1)) || any(step<1)
   error('SUBSAMPLE_FACTOR must be a positive integer array');
end
if ~isa(in,'dip_image')
   in = dip_image(in);
end
nd = ndims(in);
if numel(step)==1
   step = repmat(step,1,nd);
elseif numel(step)~=nd
   error('SUBSAMPLE_FACTOR has a wrong length');
end
sz = imsize(in)-1;
subs = cell(1,nd);
for ii=1:nd
   subs{ii} = 0:step(ii):sz(ii);
end
subs = substruct('()',subs);
in = subsref(in,subs);
