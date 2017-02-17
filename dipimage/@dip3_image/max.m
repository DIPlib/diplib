%MAX   Get the first maximum in an image.
%   [VALUE,POSITION] = MAX(B) gets the value and postion of the first
%   maximum in image B.
%
%   (TODO: The POSITION output value is not yet implemented)
%
%   [VALUE,POSITION] = MAX(B,M) gets the value and postion of the first
%   maximum in image B masked by M. M may be [] for no mask.
%
%   VALUE = MAX(B,M,DIM) performs the computation over the dimensions
%   specified in DIM. DIM can be an array with any number of
%   dimensions. M may be [] for no mask.
%
%   [VALUE,POSITION] = MAX(B,M,DIM) gets the value and position of
%   the first maximum along dimension DIM. DIM is a single dimension.
%
%   (TODO) VALUE = MAX(B,C) is the pixel-by-pixel maximum operator. It returns
%   an image with each pixel the largest taken from B or C. C must not
%   be a binary image, or it will be taken as a mask image (see syntax
%   above).
%
%   (TODO) f B is a tensor image, MAX(B) is the image with the maximum over all
%   the tensor components.
