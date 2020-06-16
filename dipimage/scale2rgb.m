%SCALE2RGB   Convert scale-space to RGB image
%
% SYNOPSIS:
%  image_out = scale2rgb(image_in,red,green,blue,relative)
%
% PARAMETERS:
%  image_in:         3D image with scale along the third axis.
%  red, green, blue: arrays with indices into the third dimension in image_in.
%  relative:         set to true for scaling of intensity values per pixel.

% (c)2020, Delft University of Technology, by Yan Guo
% Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
% Originally written by Cris Luengo.
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

function image_out = scale2rgb(image_in,red,green,blue,relative)

if nargin < 2
    red = 3;
end
if nargin < 3
    green = 2;
end
if nargin < 4
    blue = 1;
end
if nargin < 5
    relative = 0;
end
if ~isa(image_in,'dip_image')
    in = dip_image(image_in);
end
if ~isscalar(image_in)
    error('Only implemented for scalar images');
end

if ndims(image_in) ~= 3
    error('Input image must have three dimensions.')
end
M = size(image_in,3)-1;
red = red(:);
green = green(:);
blue = blue(:);
if isempty(red) | isempty(green) | isempty(blue)
    error('Indices must be provided.');
end
tmp = [red;green;blue];
if any(tmp>M) | any(tmp<0)
    error('Index exceeds image dimensions.')
end
red = squeeze(sum(image_in(:,:,red),[],3));
green = squeeze(sum(image_in(:,:,green),[],3));
blue = squeeze(sum(image_in(:,:,blue),[],3));
val = min([min(red),min(green),min(blue)]);
if val~=0
    red = red-val;
    green = green-val;
    blue = blue-val;
end
if relative
    val = (red+green+blue)*(1/255);
    val(val==0) = 1;
    val = 1/val;
    red = dip_image(red*val,'uint8');
    green = dip_image(green*val,'uint8');
    blue = dip_image(blue*val,'uint8');
else
    val = 255/percentile([red,green,blue],95);
    red = dip_image(red*val,'uint8');
    green = dip_image(green*val,'uint8');
    blue = dip_image(blue*val,'uint8');
end
image_out = joinchannels('RGB',red,green,blue);
image_out = dip_image(image_out,'uint8');

end
