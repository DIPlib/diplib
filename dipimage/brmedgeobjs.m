%BRMEDGEOBJS   Remove edge objects from binary image
%
% SYNOPSIS:
%  image_out = brmedgeobjs(image_in,connectivity)
%
% PARAMETERS:
%  connectivity: defines the neighborhood:
%     * 1 indicates 4-connected neighbors in 2D or 6-connected in 3D.
%     * 2 indicates 8-connected neighbors in 2D
%     * 3 indicates 28-connected neighbors in 3D
%
% DEFAULTS:
%  connectivity = 1
