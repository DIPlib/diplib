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


% (C) Copyright 1999-2010               Pattern Recognition Group
%     All rights reserved               Faculty of Applied Physics
%                                       Delft University of Technology
%                                       Lorentzweg 1
%                                       2628 CJ Delft
%                                       The Netherlands
%
% Peter Bakker, Oct 2000.
% Jan 2004:          Make nD, extended help and make faster. (BR)
% March 2007:        Changed default base to sqrt(2).  (BR)
% 27 September 2010: Rewrote for simplicity and speed. (CL)

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