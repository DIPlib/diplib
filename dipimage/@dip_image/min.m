%MIN   Get the first minimum in an image.
%   [VALUE,POSITION] = MIN(B) gets the value and postion of the first
%   minimum in image B.
%
%   (TODO: The POSITION output value is not yet implemented)
%
%   [VALUE,POSITION] = MIN(B,M) gets the value and postion of the first
%   minimum in image B masked by M. M may be [] for no mask.
%
%   VALUE = MIN(B,M,DIM) performs the computation over the dimensions
%   specified in DIM. DIM can be an array with any number of
%   dimensions. M may be [] for no mask.
%
%   [VALUE,POSITION] = MIN(B,M,DIM) gets the value and position of
%   the first minimum along dimension DIM. DIM is a single dimension.
%
%   (TODO) VALUE = MIN(B,C) is the pixel-by-pixel minimum operator. It returns
%   an image with each pixel the largest taken from B or C. C must not
%   be a binary image, or it will be taken as a mask image (see syntax
%   above).
%
%   (TODO) If B is a tensor image, MIN(B) is the image with the minimum over all
%   the tensor components.
