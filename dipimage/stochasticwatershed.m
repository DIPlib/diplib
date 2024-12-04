%STOCHASTICWATERSHED   Stochastic watershed
%
% SYNOPSIS:
%  pdf = stochasticwatershed(img,seeds,iter,noise,grid)
%
% PARAMETERS:
%  img:   Input image
%  seeds: Number of seeds
%  iter:  Number of iterations
%  noise: Noise strength
%  grid:  How the seeds are distributed. One of:
%    - 'poisson':     According to a Poisson process.
%    - 'rectangular': A randomly rotated and translated square grid.
%    - 'hexagonal':   A randomly rotated and translated hexagonal grid (2D)
%    - 'fcc', 'bcc':  A randomly rotated and translated FCC or BCC grid (3D)
%    - 'exact':       Uses the exact evaluation algorithm.
%
% DEFAULTS:
%  seeds = 100
%  iter = 50
%  noise = 0
%  grid = 'hexagonal' for 2D, 'fcc' for 3D, 'rectangular' otherwise
%
% NOTES:
%  If ITER is 0, or GRID is 'exact', then the exact probabilities are computed,
%  and PDF is in the range [0,1].
%
%  Otherwise, ITER iterations of the watershed are applied with random seeds,
%  and the results are added up. Each pixel value is thus the number of times
%  that pixel was a watershed pixel. Divide this image by ITER to obtain an
%  estimate of the probabilities.
%
%  By setting NOISE to a positive value, a bit of noise is added to the input
%  at every iteration, causing the estimated probabilities to be more
%  relevant in the case that the image contains regions of unequal size.
%
% LITERATURE:
%  - J. Angulo and D. Jeulin, "Stochastic watershed segmentation", Proceedings
%    of the 8th International Symposium on Mathematical Morphology, Instituto
%    Nacional de Pesquisas Espaciais (INPE), São José dos Campos, pp. 265–276,
%    2007.
%  - K.B. Bernander, K. Gustavsson, B. Selig, I.-M. Sintorn, and C.L. Luengo
%    Hendriks, "Improving the stochastic watershed", Pattern Recognition
%    Letters 34:993-1000, 2013.
%  - F. Malmberg and C.L. Luengo Hendriks, "An efficient algorithm for exact
%    evaluation of stochastic watersheds", Pattern Recognition Letters
%    47:80-84, 2014.
%  - B. Selig, F, Malmberg and C.L. Luengo Hendriks, "Fast evaluation of the
%    robust stochastic watershed", Proceedings of ISMM 2015, LNCS 9082:705-716,
%    2015.
%
% DIPlib:
%  This function calls the DIPlib function <a href="https://diplib.org/diplib-docs/segmentation.html#dip-StochasticWatershed-Image-CL-Image-L-Random-L-dip-uint--dip-uint--dfloat--String-CL">dip::StochasticWatershed</a>.

% (c)2019, Cris Luengo.
% Based on original code: (c)2012-2013, Cris Luengo, Karl B. Bernander, Kenneth Gustavsson.
% Based on original code: (c)2013, Filip Malmberg.
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

function out = stochasticwatershed(varargin)
out = dip_morphology('stochasticwatershed',varargin{:});
