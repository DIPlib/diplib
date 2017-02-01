%EXTENDIMAGE   Extends the image domain
%
% SYNOPSIS:
%  image_out = extendimage(image_in,border,boundary_condition)
%
%  BORDER is an integer or a vector of integers (one per image dimension)
%  specifying by how many pixels each image edge needs to be expanded. For
%  example, if IMAGE_IN is 2D, and BORDER = [4,6], then the image will be
%  expanded by 4 pixels on the left and right side, and by 6 on the top
%  and bottom, yielding IMSIZE(IMAGE_OUT) == IMSIZE(IMAGE_IN) + [4,6]*2.
%
%  BOUNDARY_CONDITION is a string or a cell array of strings (one per image
%  dimension) specifying how the pixel values outside of the original image
%  domain are to be filled.
%
% DEFAULTS:
%  boundary_condition = {} (equivalent to 'mirror')
%
% NOTE:
%  See the user guide for the available boundary condition strings.
%
% DIPlib:
%  This function calls the DIPlib functions dip::ExtendImage.
