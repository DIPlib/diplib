%JOINCHANNELS   Joins scalar images as channels in a color image
%   JOINCHANNELS(COL,A,B,C,...) creates a color image with components
%   A, B, C, etc. The color space is set to COL. The images A, B, C,
%   etc. must be scalar images of the same size.
%
%   If any of A, B, C is binary, it is converted to a grey-value image
%   with values 0 and 255.
%
%   JOINCHANNELS(COL,A) creates a color image with ndims(A)-1 dimensions.
%   The last dimension of A is assumed to be the color channels.
%
%   See also dip_image/colorspace

% (c)2017, Cris Luengo.
% (c)1999-2014, Delft University of Technology.
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

function out = joinchannels(col,varargin)

if nargin < 2
   error('I need at least a color space name and an image to work on')
end
try
   chans = dip_image.numberchannels(col);
catch
   error('Color space unknown')
end
N = nargin-1;
if N == 1
   out = dip_image(varargin{1});
   if ~isscalar(out)
      if iscolor(out)
         error('The input image is already a color image')
      end
      k = numtensorel(out);
      if k > chans
         error(['Too many channels, ',num2str(chans),' channels expected.']);
      elseif k < chans
         out = tensortospatial(out);
         sz = imsize(out);
         sz(2) = chans-k; % tensor dimension becomes second dimension!
         out = cat(2,out,newim(sz,datatype(out)));
         out = spatialtotensor(out);
      end
      out = colorspace(out,col);
   else
      if islogical(out)
         out = dip_image(out,'uint8');
      end
      dim = ndims(out);
      k = imsize(out,dim);
      if k > chans
         error(['Too many channels, ',num2str(chans),' channels expected.']);
      elseif k < chans
         sz = imsize(out);
         sz(dim) = chans-k;
         out = cat(dim,out,newim(sz,datatype(out)));
      end
      out = spatialtotensor(out,dim);
      out = colorspace(out,col);
   end
else
   % Convert binary images to uint8 with values 0 and 255.
   if N > chans
      error(['Too many channels, ',num2str(chans),' channels expected.']);
   end
   for ii=1:N
      tmp = varargin{ii};
      if isa(tmp,'dip_image') && ~isscalar(tmp)
         error('I need scalar images as channels')
      end
      if islogical(tmp)
         if isa(tmp,'dip_image')
            tmp = dip_array(tmp);
         end
         new = zeros(size(tmp),'uint8');
         new(tmp) = 255;
         varargin{ii} = new;
      end
   end
   if N < chans
      varargin{N+1:chans} = newim(imsize(varargin{1}),datatype(varargin{1}));
   end
   out = dip_image(varargin);
   out = colorspace(out,col);
end
