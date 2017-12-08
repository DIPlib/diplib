% DIPimage toolbox for quantitative image analysis
% Version 3.0.alpha   07-Dec-2017
% (c)2016-2017, Cris Luengo and contributors
% (c)1999-2014, Delft University of Technology
%
% GUI:
%   dipimage               - Starts the interactive user interface DIPimage
%   dipshow                - Shows an image in an interactive display window
%
% Configuration:
%   dipgetpref             - Gets a DIPimage preference
%   dipsetpref             - Sets a DIPimage preference
%   dipinit                - Initialize the working environment
%   dipfig                 - Links a variable name with a figure window
%   dipmenus               - Describes the menu system to DIPIMAGE
%
% File I/O:
%   readim                 - Read image from file
%   readtimeseries         - Reads a series/stack of TIFF images as a 3D image
%   writeim                - Write an image to file
%
% Display:
%   overlay                - Overlay a grey-value or color image with a binary or label image
%   dipgetcoords           - Interactive coordinate extraction
%   dipdrawpolygon         - Interactive polygon drawing tool
%   diproi                 - Interactive ROI selection
%   diptruesize            - Sets the size of a display to its natural size
%   dipclf                 - Clears figure windows created by DIPSHOW
%
% Generation:
%   newim                  - Creates a scalar image initialized to zero
%   newcolorim             - Creates a color image initialized to zero
%   newtensorim            - Creates a tensor dip_image initialized to zero
%   deltaim                - Generate a discrete delta function
%   ramp                   - Creates an image with one cartesian coordinate
%   xx                     - Creates an image with the x-axis cartesian coordinate
%   yy                     - Creates an image with the y-axis cartesian coordinate
%   zz                     - Creates an image with the z-axis cartesian coordinate
%   rr                     - Creates an image with the radius component of polar or spherical coordinates
%   phiphi                 - Creates an image with the phi component of polar or spherical coordinates
%   thetatheta             - Creates an image with the theta component of spherical coordinates
%   setborder              - Sets the pixels at the border of the image to a constant value
%   noise                  - Add noise to an image
%   drawline               - Draws a line in an image
%   drawpolygon            - Draws a polygon in an image
%   drawshape              - Draws an ellipse (ellipsoid), rectangle (box) or diamond
%   gaussianblob           - Adds Gauss shaped spots to an image
%
% Manipulation:
%   mirror                 - Mirror an image
%   shift                  - Shift an image using interpolation
%   rotation               - Rotate an image within an orthogonal plane
%   skew                   - Geometric transformation
%   resample               - Shift and a scale an image using interpolation
%   subsample              - Subsample an image
%   rebin                  - Rebinning of an image
%   split                  - Split an image into subsampled versions
%   extend                 - Extends/pads an image with values
%   cut                    - Cuts/crops an image symmetrically around the center
%
% Point:
%   clip                   - Grey-value clipping (or clamping)
%   erfclip                - Grey-value error function clipping
%   gaussianedgeclip       - Clips/maps grey-values to produce a Gaussian edge
%   gaussianlineclip       - Clips/maps grey-values to produce a Gaussian line
%   stretch                - Grey-value stretching
%   lut                    - Look-up Table (with interpolation)
%
% Filters:
%   convolve               - General convolution filter
%   gaussf                 - Gaussian filter
%   unif                   - Uniform filter, mean filter, convolution with uniform weights
%   maxf                   - Maximum filter
%   minf                   - Minimum filter
%   medif                  - Median filter
%   percf                  - Percentile filter
%   varif                  - Variance filter
%
% Differential Filters:
%   derivative             - Derivative filters
%   dx                     - First derivative in the X-direction
%   dy                     - First derivative in the Y-direction
%   dz                     - First derivative in the Z-direction
%   gradientvector         - Gradient vector
%   gradmag                - Gradient magnitude
%   dxx                    - Second derivative in the X-direction
%   dyy                    - Second derivative in the Y-direction
%   dzz                    - Second derivative in the Z-direction
%   dxy                    - Second derivative in the XY-direction
%   dxz                    - Second derivative in the XZ-direction
%   dyz                    - Second derivative in the YZ-direction
%   hessian                - Hessian matrix of an image
%   dethessian             - Det(Hessian) operator
%   laplace                - Laplace operator
%   prewittf               - Prewitt derivative filter
%   sobelf                 - Sobel derivative filter
%
% Adaptive Filters:
%   kuwahara               - Kuwahara filter for edge-preserving smoothing
%   selectionf             - Selection filter
%
% Binary Filters:
%   bdilation              - Binary dilation
%   berosion               - Binary erosion
%   bopening               - Binary opening
%   bclosing               - Binary closing
%   hitmiss                - Hit-miss operator -- morphological template matching
%   bskeleton              - Binary skeleton
%   bpropagation           - Binary propagation
%   brmedgeobjs            - Remove edge objects from binary image
%   smallobjectsremove     - Removes small objects from binary or labeled image
%   fillholes              - Fill holes in a binary image
%   hull                   - Generates convex hull of a 2D or 3D binary image
%   countneighbors         - Counts the number of set neighbors for each pixel
%   bmajority              - Binary majority voting
%   getsinglepixel         - Get single-pixels from skeleton
%   getendpixel            - Get end-pixels from skeleton
%   getlinkpixel           - Get link-pixels from skeleton
%   getbranchpixel         - Get branch-pixels from skeleton
%
% Morphology:
%   dilation               - Grey-value dilation
%   erosion                - Grey-value erosion
%   closing                - Grey-value closing
%   opening                - Grey-value opening
%   tophat                 - Top-hat
%   rankmin_closing        - Rank-min closing
%   rankmax_opening        - Rank-max opening
%   areaopening            - Area opening or closing
%   pathopening            - Path opening and closing
%   hmaxima                - H-maxima transform
%   hminima                - H-minima transform
%   reconstruction         - Morphological reconstruction by dilation or erosion
%
% Diffusion:
%
% Restoration:
%
% Segmentation:
%   threshold              - Thresholding
%   minima                 - Detect local minima
%   maxima                 - Detect local maxima
%   watershed              - Watershed
%   waterseed              - Watershed initialized with a seed image
%   label                  - Label objects in a binary image
%   relabel                - Renumber labels in a labeled image
%
% Transforms:
%   ft                     - Fourier Transform (forward)
%   ift                    - Fourier transform (inverse)
%   dt                     - Euclidean distance transform
%   vdt                    - Vector components of Euclidean distance transform
%   gdt                    - Grey-weighted distance transform
%
% Analysis:
%   measure                - Do measurements on objects in an image
%   msr2obj                - Label each object in the image with its measurement
%   structuretensor        - Computes the Structure Tensor
%   findshift              - Finds shift between two images
%   findminima             - Find local minima, with sub-pixel precision
%   findmaxima             - Find local maxima, with sub-pixel precision
%   subpixlocation         - Find sub-pixel location of extrema
%
% Statistics:
%   diphist                - Displays a histogram
%   diphist2d              - Generates a 2D histogram
%   getmaximumandminimum   - Find the minimum and maximum sample value
%   getsamplestatistics    - Compute the first four central moments of the image's sample values
%   autocorrelation        - Computes the auto-correlation of an image
%   crosscorrelation       - Computes the cross-correlation between two images
%   errormeasure           - Compares two images
%   noisestd               - Estimate noise standard deviation
%   entropy                - Computes the entropy (in bits) of an image
%
% Interactive image display:
%   dipshow                - Shows an image in an interactive display window
%   diptruesize            - Sets the size of a display to its natural size
%   dipclf                 - Clears figure windows created by DIPSHOW
%   dipanimate             - Animates a 3D image in a display window
%   dipisosurface          - Plot isosurfaces of 3D grey value images
%   diplink                - Linking of displays for 3D images
%   diplooking             - Interactive looking glas over the image
%   dipmapping             - Changes the mapping of an image in a figure window
%   dipmaxaspect           - Undocumented till usefulness proved
%   dippan                 - Interactive panning of an image
%   dipstep                - Stepping through slices of a 3D image
%   diptest                - Interactive pixel testing
%   dipzoom                - Interactive image zooming
%
% Other available functions are:
%   arrangeslices          - Converts an n-D image into a 2D image by arringing the slices
%   array2im               - Convert a tensor image to an image stack
%   bbox                   - Bounding box of an n-D binary image
%   boundary_condition     - Information on the BOUNDARY_CONDITION parameter (not a function)
%   coord2image            - Generate binary image with ones at coordinates
%   coordinates            - Creates an image with general coordinates
%   detile                 - Splits an image in subimages
%   displaylabelnumbers    - Overlay the label numbers on a figure window
%   eig_largest            - Computes the largest eigenvector and value
%   extendregion           - Extends a region in the image
%   im2array               - Convert an image stack to a vector image
%   im2mat                 - Converts a dip_image to a matlab array
%   joinchannels           - Joins scalar images as channels in a color image
%   lee                    - Lee operator
%   mat2im                 - Converts a matlab array to a dip_image
%   mdhistogram            - Compute a multi-dimensional histogram
%   ramp1                  - Creates a 1D image with one cartesian coordinate
%   readics                - Read an ICS file into an image
%   readtiff               - Read a TIFF file into an image
%   select                 - Selects pixels from one image or another depending on a condition
%   tile                   - Displays a 2D tensor image in one 2D image
%   writeics               - Write an image as an ICS file
%   writetiff              - Write an image as a TIFF file
%   xx1                    - Creates an image with the x-axis cartesian coordinate
%   yy1                    - Creates an image with the y-axis cartesian coordinate
%   zz1                    - Creates an image with the z-axis cartesian coordinate
%
% Type
%   methods dip_image
% to get a list of functions overloaded for dip_images.
%
% More information is available in the DIPimage User Manual, type
%   web(dipgetpref('UserManualLocation'),'-browser')

% (c)2017, Cris Luengo.
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
