%IM2SNAKE   Creates a snake based on a binary image
%
% SYNOPSIS:
%  snake = im2snake(image)
%
% PARAMETERS:
%  image: 2D binary or labelled image. If it is a labelled image, only
%         object with id=1 is considered.
%
% NOTE:
%  Only the first (top-left) contiguous group of pixels is used to create
%  the snake. If you want to control which group is used, label the image
%  and select an id:
%
%     snake = im2snake(label(img)==3)
%
% SEE ALSO:
%  snake2im, snakeminimize, snakedraw

% (c)2009, 2018, Cris Luengo.
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

function snake = im2snake(img)

if ~isa(img,'dip_image')
   img = dip_image(img);
end
if ~isscalar(img) || issigned(img) || isfloat(img) || ndims(img)~=2
   error('EDGE_IMAGE must be a 2D scalar image, of binary or unsigned integer type')
end

snake = traceobjects(+img,1,2,'polygon');
snake = snake{1};
