%COUNTINGFRAME   Applies a counting frame to a binary or labelled image
%
% A counting frame is a method to select objects to measure. It avoids bias
% in any per-object measurement, such as object count, size distributions,
% etc. Objects fully outside the counting frame are removed from the image,
% as are those that are partially inside the counting frame but touch the
% exclusion line. The counting frame should be small enough such that no
% objects that remain in the image are cut by the image boundary. This
% function will give a warning if any such objects are present.
%
% See any book on stereology for further information on the counting frame.
%
% SYNOPSIS:
%  image_out = countingframe(image_in,frame_size)
%
% PARAMETERS:
%  image_in:   Binary or labelled (unsigned integer) image. Only 2D images
%              are supported for the time being.
%  image_out:  Image of same size and type as IMAGE_IN.
%  frame_size: Size of the counting frame. The frame will be centered in the
%              image. This should have as many dimensions as the image; if
%              fewer dimensions are given, other dimensions will be filled
%              out by replication. If set to an empty array, an edge size of
%              80% of the image edge size will be used.
%
% DEFAULTS:
%  frame_size = []  ( equivalent to 0.8*imsize(image_in) )
%
% EXAMPLE:
%  a = readim('cermet');
%  b = label(~threshold(a));
%  b = countingframe(b,200);
%  m = measure(b,a,'size');
%  diphist(m.size,20);   % unbiased estimate of size distribution
%  d = size(m,1)/(200^2) % unbiased estimate of particle density (px^-2)

% (c)2014, 2018, Cris Luengo.
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

function lab = countingframe(lab,fsz)

% Make sure we have a proper input image
if ~isa(lab,'dip_image')
   lab = dip_image(lab);
end
isz = imsize(lab);
isbin = islogical(lab);
% We need labels here
if isbin
   lab = label(lab);
end

% Default value for FSZ
if nargin<2 || isempty(fsz)
   fsz = round(isz*0.8);
end
if any(fsz>=isz)
   error('Counting frame does not fit in the image!');
end
topleft = floor((isz-fsz)/2);
botright = topleft + fsz - 1;

% Empty set of objects
LUT = zeros(max(lab),1,class(dip_array(lab)));

% Add objects partially inside counting frame...
q = dip_array(lab(topleft(1):botright(1),topleft(2):botright(2)));
q = unique(q(:));
q(q==0) = [];
LUT(q) = q;

% Remove objects touching exclusion line...
q = newim(lab,'bin');
q(topleft(1),0:botright(2)) = 1;
q(topleft(1):botright(1),botright(2)) = 1;
q(botright(1),botright(2):end) = 1;
q = unique(dip_array(lab(q)));
q(q==0) = [];
LUT(q) = 0;

% Apply LUT
LUT = [0;LUT]; % This should maintain the data type of LUT.
lab = lut(lab,LUT);

% Test
if any(lab([0,end],:)) || any(lab(:,[0,end]))
   warning('Counting frame is not small enough, some objects touching the image border remain.');
end

% Return same type as input
if isbin
   lab = lab>0;
end
