%MEDIAN   Get the median of an image.
%   VALUE = MEDIAN(B) gets the value of the median of all pixels in
%   the image B.
%
%   VALUE = MEDIAN(B,M), with M a binary image, is the same as MEDIAN(B(M)).
%
%   VALUE = MEDIAN(B,M,DIM) computes the median over the dimensions specified
%   in DIM. For example, if B is a 3D image, MEDIAN(B,[],3) returns an image
%   with 2 dimensions, containing the median over the pixel values along
%   the third dimension (z). DIM can be an array with any number of
%   dimensions. M can be [].
%
%   (TODO) [VALUE,POSITION] = MEDIAN(B,...) returns the position of the found values
%   as well. With this syntax, DIM can specify just one dimension.

function varargout = median(varargin)
varargout = cell(1,max(nargout,1));
[varargout{:}] = percentile(varargin{1},50,varargin{2:end});
