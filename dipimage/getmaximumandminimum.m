%GETMAXIMUMANDMINIMUM   Find the minimum and maximum sample value
%
% SYNOPSIS:
%  out = getmaximumandminimum(image_in,mask)
%
%  This is equivalent to
%     out = [min(image_in,mask),max(image_in,mask)]
%  except it's faster.
%
% DEFAULTS:
%  mask = [] (all pixels are examined)
%
% DIPlib:
%  This function calls the DIPlib function dip::GetMaximumAndMinimum.
