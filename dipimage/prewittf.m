%PREWITTF   Prewitt derivative filter
%
% SYNOPSIS:
%  image_out = prewittf(image_in,direction)
%
% PARAMETERS:
%  direcion: Dimension to take the derivative along.
%            Dimensions are 1-based (i.e. x = 1, y = 2).
%
% DEFAULTS:
%  direction = 1
%
% SEE ALSO:
%  sobelf, derivative, dx, dy, etc.

% (c)2017, Cris Luengo.
% Based on original DIPimage code: (c)1999-2014, Delft University of Technology.

function out = prewittf(in,dim)
nd = ndims(in);
filter = cell(1,nd);
for ii=1:nd
   if ii==dim
      filter{ii} = [1,0,-1]/2;
   else
      filter{ii} = [1,1,1]/3;
   end
end
out = convolve(in,filter);
