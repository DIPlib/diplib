%MEAN   Mean of all pixels in an image.
%   VALUE = MEAN(B) returns the mean intensity of all pixels in image B.
%
%   VALUE = MEAN(B,M) only computes the mean of the pixels within the
%   mask specified by the binary image M, and is equivalent to MEAN(B(M)).
%
%   VALUE = MEAN(B,M,DIM) performs the computation over the dimensions
%   specified in DIM. DIM can be an array with any number of dimensions.
%   M can be [].
%
%   (TODO) If B is a tensor image, MEAN(B) is the image with the mean over all the
%   tensor components.
