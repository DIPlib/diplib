%CLUSTER   Cluster
%
% Generates a labeled image that splits the image into contiguous clusters.
% Two methods are supported: K-means clustering currently uses a random
% initialization, meaning the result can be different on subsequent runs.
% Minimum variance clustering splits the image along one dimension
% iteratively, creating a k-d-tree--like partitioning. The chosen split
% at each iteration is the one that most reduces the sum of variances
% of the partitions.
%
% SYNOPSIS:
%  [image_out,coordinates] = cluster(image_in,nClusters,method)
%
% PARAMETERS:
%  nClusters: number of clusters to generate
%  method:    'kmeans', or 'minvariance'
%
% DEFAULTS:
%  nClusters = 2
%  method = 'minvariance'
%
% NOTE:
%  The optional second output argument lists the coordinates of the cluster
%  centroids.
%  The first output argument is a labeled image representing the clusters.
%
% DIPlib:
%  This function calls the DIPlib functions dip::KMeansClustering and
%  dip::MinimumVariancePartitioning.

% (c)2018, Cris Luengo.
% Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
%
% Licensed under the Apache License, Version 2.0 (the "License");
% you may not use this file except in compliance with the License.
% You may obtain a copy of the License at
%
%    http://www.apache.org/licenses/LICENSE-2.0
%
% Unless required by applicable law or agreed to in writing, software
% distributed under the License is distributed on an "AS IS" BASIS,
% WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
% See the License for the specific language governing permissions and
% limitations under the License.

function varargout = cluster(varargin)
varargout = cell(1,max(nargout,1));
[varargout{:}] = dip_segmentation('cluster',varargin{:});
