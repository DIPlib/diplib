%PERCENTILE   Get the percentile of an image.
%   VALUE = PERCENTILE(B,P) gets the value of the P percentile of all
%   pixels in the image B. P must be between 0 and 100.
%
%   Note that:
%   PERCENTILE(B,0) is the same as MIN(B)
%   PERCENTILE(B,50) is the same as MEDIAN(B)
%   PERCENTILE(B,100) is the same as MAX(B)
%
%   VALUE = PERCENTILE(B,P,M), with M a binary image, is the same as
%   PERCENTILE(B(M),P).
%
%   VALUE = PERCENTILE(B,P,M,DIM) computes the P percentile over the
%   dimensions specified in DIM. For example, if B is a 3D image,
%   PERCENTILE(B,10,[],3) returns an image with 2 dimensions, containing
%   the 10th percentile over the pixel values along the third dimension (z).
%   DIM can be an array with any number of dimensions. M can be [].
%
%   (TODO) [VALUE,POSITION] = PERCENTILE(B,P,...) returns the position of the found
%   values as well. With this syntax, DIM can specify just one dimension.
