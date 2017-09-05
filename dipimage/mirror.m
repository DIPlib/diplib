%MIRROR   Mirror an image
%
% Mirrors an image, i.e the pixels in those dimensions of image
%  specified.
%
% SYNOPSIS:
%  image_out = mirror(image_in, mirror_type, mirror_parameters)
%  image_out = mirror(image_in, mirror_parameters)
%
% PARAMETERS:
%  mirror_type: String, one of:
%     - 'x-axis', 'y-axis', 'z-axis': mirror the single axis.
%     - 'point': mirror all axes.
%     - 'user': mirror the axes as given by MIRROR_PARAMETERS.
%  mirror_parameters: Specifies the axes to mirror. Either a
%     logical vector with one element per image dimension, or
%     integer vector.
%
% DEFAULT:
%  mirror_type = 'point'
%
% NOTES:
%  If MIRROR_TYPE is left out, and MIRROR_PARAMETERS is given, 'user'
%  is presumed (since that is the only mode where parameters make sense).
%
%  MIRROR_PARAMETERS can be either an array with integer values
%  indicating which dimensions to mirror (e.g. [1,2]), or a logical
%  array with one element for each image dimension, where the element
%  is TRUE for those dimensions that need to be mirrored. That is,
%  if MIRROR_PARAMETERS is logical, FIND(MIRROR_PARAMETERS) returns
%  the integer array version.

% (c)2017, Cris Luengo.
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

function out = mirror(in,mir,para)
if ~isa(in,'dip_image')
   in = dip_image(in);
end
di = ndims(in);
if ischar(mir)
   mir_para = repmat(0,1,di);
   switch mir
   case 'x-axis'
      if di>=1
         mir_para = 1;
      end
   case 'y-axis'
      if di>=2
         mir_para = 2;
      end
   case 'z-axis'
      if di>=3
         mir_para = 3;
      end
   case 'point'
      mir_para = 1:di;
   case 'user'
      mir_para = para;
   otherwise
      error('MIRROR_TYPE parameter not recognized');
   end
else
   mir_para = mir;
end
if islogical(mir_para) || ( numel(mir_para) == di && all(mir_para==0|mir_para==1) )
   mir_para = find(mir_para);
end
if any(mod(mir_para,1))
   error('MIRROR_PARAMETERS must be an integer array')
end
if any(mir_para<1) || any(mir_para>di)
   error('MIRROR_PARAMETERS out of range')
end
mir_para = unique(mir_para);
mir_para = mir_para(:)';
s = substruct('()',cell(di,1));
s(1).subs(:) = {':'};
for p = mir_para
   s(1).subs{p} = [imsize(in,p)-1:-1:0];
end
out = subsref(in,s);
