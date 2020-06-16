%SCALESPACE   Gaussian scale-space
%
% SYNOPSIS:
%  [sp,bp,pp] = scalespace(image_in,nscales,base)
%
% DEFAULTS:
%  nscales = 7
%  base = sqrt(2)    Images are smoothed with base^ii, ii=0:nscales-2
%                    (the first scale has no smoothing).
%
% OUTPUT PARAMETERS:
%  sp: Scale space
%  bp: Difference between scales
%  pp: Variance between scales
%
% EXAMPLES:
%  [x,y,z] = scalespace(readim) %2D image
%  [x1,y1] = scalespace(readim('chromo3d')) %3D image
%
% LITERATURE:
%  J.J. Koenderink, The Structure of Images, Biological Cybernetics, 50:363-370, 1984.
%  T. Lindeberg, Scale-Space for Discrete Signals, IEEE Transactions PAMI, 12(3):234-254, 1990.

% (c)2020, Delft University of Technology, by Yan Guo
% Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
% Originally written by Peter Bakker.
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

function [varargout] = scalespace(in,scales,base)

if nargin < 2
    scales = 7;
end
if nargin < 3
    base = sqrt(2);
end
if ~isa(in,'dip_image')
    in = dip_image(in);
end
if ~isscalar(in)
    error('Only implemented for scalar images');
end

di = ndims(in)+1;

sp = cell(scales,1);
sp{1} = in;
for ii=2:scales
    sp{ii} = gaussf(in,base.^(ii-2));
end

varargout{1} = cat(di,sp{:});

if nargout>=2

    bp = cell(scales-1,1);
    for ii=1:scales-1
        bp{ii} = sp{ii+1}-sp{ii};
    end

    clear sp % no longer used, clear to make space for other outputs
    varargout{2} = cat(di,bp{:});

    if nargout>=3

        pp = cell(scales-1,1);
        for ii=1:scales-1
            pp{ii} = gaussf(bp{ii}^2,base.^(ii));
        end

        clear bp % no longer used, clear to make space for other outputs
        varargout{3} = cat(di,pp{:});

    end

end
