%DELTAIM   Generate a discrete delta function
%
% SYNOPSIS:
%  image_out = deltaim(image_size,data_type)
%
% Generates a discrete delta function. It is centred such that it
% is at what is considered the origin by the Fourier transform. The
% argument IMAGE_SIZE has many possiblities (an image to copy size
% from, a size array, etc). Read the help on NEWIM for the possibilities.
%
% DATA_TYPE sets the data type of the new image. The default is 'sfloat'.

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

function out = deltaim(varargin)
out = newim(varargin{:});
ind = num2cell(floor(imsize(out)/2));
out(ind{:}) = 1;
