%CUMSUM   Cumulative sum of pixels.
%   C = CUMSUM(B) returns an image with the same size as B, where each pixel
%   is the cumulative sum of all pixels with equal or smaller indices in image
%   B. That is, C(I,J) = SUM(B(0:I,0:J).
%
%   C = CUMSUM(B,M) presumes that the pixels not selected by the mask image M are
%   zero.
%
%   C = CUMSUM(B,M,DIM) performs the computation over the dimensions specified in
%   DIM. DIM can be an array with any number of dimensions. M can be [].
%   If DIM==1, then C(I,J) = SUM(B(0:I,J).
