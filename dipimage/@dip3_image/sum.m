%SUM   Sum of all pixels in an image.
%   VALUE = SUM(B) gets the value of the sum of all pixels in an image.
%
%   VALUE = SUM(B,M) only computes the sum of the pixels within the
%   mask specified by the binary image M, and is equivalent to SUM(B(M)).
%
%   VALUE = SUM(B,M,DIM) computes the sum over the dimensions specified
%   in DIM. For example, if B is a 3D image, SUM(B,[],3) returns an image
%   with 2 dimensions, containing the sum over the pixel values along
%   the third dimension (z). DIM can be an array with any number of
%   dimensions. M can be [].
%
%   (TODO) If B is a tensor image, SUM(B) is the image with the sum over all the
%   tensor components.
