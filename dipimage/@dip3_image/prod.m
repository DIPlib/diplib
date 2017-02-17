%PROD Product of elements.
%   VALUE = PROD(B) gets the value of the product of all pixels in an image.
%
%   VALUE = PROD(B,M) only computes the product of the pixels within the
%   mask specified by the binary image M, and is equivalent to PROD(B(M)).
%
%   VALUE = PROD(B,M,DIM) computes the product over the dimensions specified
%   in DIM. For example, if B is a 3D image, PROD(B,[],3) returns an image
%   with 2 dimensions, containing the product over the pixel values along
%   the third dimension (z). DIM can be an array with any number of
%   dimensions. M can be [].
%
%   (TODO) If B is a tensor image, PROD(B) is the image with the product over all
%   the tensor components.
