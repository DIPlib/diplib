% DIPimage toolbox for quantitative image analysis
% Version 3.0.beta   04-May-2020
% (c)2016-2019, Cris Luengo and contributors
% (c)1999-2014, Delft University of Technology
%
% GUI:
%   dipimage                - Starts the DIPimage GUI
%   dipshow                 - Shows an image in an interactive display window
%   viewslice               - 
%   view5d                  - Start the Java 5D image viewer by Rainer Heintzmann
%
% Configuration:
%   dipgetpref              - Gets a DIPimage preference
%   dipsetpref              - Sets a DIPimage preference
%   dipinit                 - Initialize the working environment
%   dipfig                  - Links a variable name with a figure window
%   dipmenus                - Describes the menu system to the DIPimage GUI
%
% File I/O:
%   readim                  - Read image from file
%   readroiim               - Read ROI of an image from file
%   readtimeseries          - Reads a series/stack of TIFF images as a 3D image
%   readrawim               - Read image from RAW format file
%   writeim                 - Write an image to file
%   readics                 - Read an ICS file into an image
%   readtiff                - Read a TIFF file into an image
%   writeics                - Write an image as an ICS file
%   writetiff               - Write an image as a TIFF file
%
% Display:
%   dipshow                 - Shows an image in an interactive display window
%   viewslice               - 
%   view5d                  - Start the Java 5D image viewer by Rainer Heintzmann
%   overlay                 - Overlay a grey-value or color image with a binary or label image
%   overlay_confidence      - Overlay a grey-value image with a grey-value image
%   overlay_vector          - Overlay an image with a vector image
%   displaylabelnumbers     - Overlay the label numbers on a figure window
%   dipgetimage             - Retrieves an image from a display
%   dipprofile              - Interactive extraction of 1D function from image
%   dipstackinspect         - Interactive inspection of the third dimension
%   dipcrop                 - Interactive image cropping
%   dipgetcoords            - Interactive coordinate extraction
%   dipdrawpolygon          - Interactive polygon drawing tool
%   diproi                  - Interactive ROI selection
%   diptruesize             - Sets the size of a display to its natural size
%   dipclf                  - Clears figure windows created by DIPSHOW
%   dipmapping              - Changes the mapping of an image in a figure window
%   diptest                 - Interactive pixel testing
%   dipzoom                 - Interactive image zooming
%   dippan                  - Interactive panning of an image
%   diplooking              - Interactive looking glas over the image
%   dipstep                 - Stepping through slices of a 3D image
%   diplink                 - Linking of display windows
%   dipanimate              - Animates a 3D image in a display window
%   dipisosurface           - Plot isosurfaces of 3D grey value images
%
% Generation:
%   newim                   - Creates a scalar image initialized to zero
%   newtensorim             - Creates a tensor dip_image initialized to zero
%   newcolorim              - Creates a color image initialized to zero
%   joinchannels            - Joins scalar images as channels in a color image
%   deltaim                 - Generate a discrete delta function
%   coordinates             - Creates an image with general coordinates
%   ramp                    - Creates an image with one cartesian coordinate
%   ramp1                   - Creates a 1D image with one cartesian coordinate
%   xx                      - Creates an image with the x-axis cartesian coordinate
%   xx1                     - Creates an image with the x-axis cartesian coordinate
%   yy                      - Creates an image with the y-axis cartesian coordinate
%   yy1                     - Creates an image with the y-axis cartesian coordinate
%   zz                      - Creates an image with the z-axis cartesian coordinate
%   zz1                     - Creates an image with the z-axis cartesian coordinate
%   rr                      - Creates an image with the radius component of polar or spherical coordinates
%   phiphi                  - Creates an image with the phi component of polar or spherical coordinates
%   thetatheta              - Creates an image with the theta component of spherical coordinates
%   window                  - Multiplies the image with a windowing function
%   psf                     - Creates an image with an incoherent PSF or OTF
%   testobject              - Creates bandlimited test objects
%   rngseed                 - Seeds DIPimage's random number generator
%   noise                   - Add noise to an image
%   randomseeds             - Create an image with randomly placed points
%   coord2image             - Generate binary image with ones at coordinates
%   setborder               - Sets the pixels at the border of the image to a constant value
%   drawline                - Draws a line in an image
%   drawpolygon             - Draws a polygon in an image
%   drawshape               - Draws an ellipse (ellipsoid), rectangle (box) or diamond
%   gaussianblob            - Adds Gauss shaped spots to an image
%
% Manipulation:
%   mirror                  - Mirror an image
%   shift                   - Shift an image using interpolation
%   correctshift            - Corrects the shift in a time series
%   wrap                    - Wraps (circular shifts) an image
%   localshift              - Shifts/warps an image by a shift vector per pixel/grid point
%   rotation                - Rotate an image within an orthogonal plane
%   skew                    - Geometric transformation
%   affine_trans            - Rotate, translate and scale a 2D or 3D image
%   warp_subpixel           - Warps image using a coordinate map.
%   resample                - Shift and a scale an image using interpolation
%   subsample               - Subsample an image
%   rebin                   - Rebinning of an image
%   splitim                 - Split an image into subsampled versions
%   get_subpixel            - Retrieves subpixel values in an image by interpolation
%   extend                  - Extends/pads an image with values
%   extendregion            - Extends a region in the image
%   cut                     - Cuts/crops an image symmetrically around the center
%   bbox                    - Bounding box of an n-D binary image
%   arrangeslices           - Converts an n-D image into a 2D image by arringing the slices
%   tile                    - Tiles tensor components
%   detile                  - Splits an image in subimages
%   umbra                   - Umbra of an image
%
% Point:
%   clip                    - Grey-value clipping (or clamping)
%   erfclip                 - Grey-value error function clipping
%   gaussianedgeclip        - Clips/maps grey-values to produce a Gaussian edge
%   gaussianlineclip        - Clips/maps grey-values to produce a Gaussian line
%   stretch                 - Grey-value stretching
%   hist_equalize           - Histogram equalization
%   quantize                - Quantize intensity values or colors
%   lut                     - Look-up Table (with interpolation)
%
% Image Arithmetic:
%   select                  - Selects pixels from one image or another depending on a condition
%   abssqr                  - Square magnitude of complex values
%
% Tensor Arithmetic:
%   eig_largest             - Computes the largest eigenvector and value
%   curlvector              - Curl of a vector field
%   divergencevector        - Divergence of a vector field
%
% Filters:
%   convolve                - General convolution filter
%   normconv                - Normalized convolution with a Gaussian kernel
%   gaussf                  - Gaussian filter
%   unif                    - Uniform filter, mean filter, convolution with uniform weights
%   maxf                    - Maximum filter
%   minf                    - Minimum filter
%   medif                   - Median filter
%   percf                   - Percentile filter
%   varif                   - Variance filter
%   gabor                   - Gabor filter
%   gabor_click             - Interactive Gabor filter
%   loggabor                - Log-Gabor filter bank
%   integral_image          - Compute the integral image.
%
% Differential Filters:
%   derivative              - Derivative filters
%   normconv                - Normalized convolution with a Gaussian kernel
%   dx                      - First derivative in the X-direction
%   dy                      - First derivative in the Y-direction
%   dz                      - First derivative in the Z-direction
%   gradientvector          - Gradient vector
%   gradmag                 - Gradient magnitude
%   dxx                     - Second derivative in the X-direction
%   dyy                     - Second derivative in the Y-direction
%   dzz                     - Second derivative in the Z-direction
%   dxy                     - Second derivative in the XY-direction
%   dxz                     - Second derivative in the XZ-direction
%   dyz                     - Second derivative in the YZ-direction
%   dgg                     - Second derivative in the gradient direction
%   dcc                     - Sum of second derivatives in directions perpendicular to gradient
%   hessian                 - Hessian matrix of an image
%   dethessian              - Det(Hessian) operator
%   laplace                 - Laplace operator
%   laplace_plus_dgg        - Laplace + Dgg
%   laplace_min_dgg         - Laplace - Dgg
%   prewittf                - Prewitt derivative filter
%   sobelf                  - Sobel derivative filter
%
% Adaptive Filters:
%   bilateralf              - Bilateral filter with different implementations
%   kuwahara                - Kuwahara filter for edge-preserving smoothing
%   selectionf              - Selection filter
%   dgg                     - Second derivative in the gradient direction
%   dcc                     - Sum of second derivatives in directions perpendicular to gradient
%   tframehessian           - Second derivatives driven by the structure tensor
%   gaussf_adap             - Adaptive Gaussian filtering for 2D and 3D images
%   gaussf_adap_banana      - Adaptive Gaussian filtering in banana like neighborhood in 2D
%
% Binary Filters:
%   bdilation               - Binary dilation
%   berosion                - Binary erosion
%   bopening                - Binary opening
%   bclosing                - Binary closing
%   hitmiss                 - Hit-and-miss transform -- morphological template matching
%   bskeleton               - Binary skeleton
%   bpropagation            - Binary propagation
%   brmedgeobjs             - Remove edge objects from binary image
%   smallobjectsremove      - Removes small objects from binary or labeled image
%   fillholes               - Fill holes in a binary image
%   hull                    - Generates convex hull of a 2D or 3D binary image
%   countneighbors          - Counts the number of set neighbors for each pixel
%   bmajority               - Binary majority voting
%   getsinglepixel          - Get single-pixels from skeleton
%   getendpixel             - Get end-pixels from skeleton
%   getlinkpixel            - Get link-pixels from skeleton
%   getbranchpixel          - Get branch-pixels from skeleton
%
% Morphology:
%   dilation                - Grey-value dilation
%   erosion                 - Grey-value erosion
%   closing                 - Grey-value closing
%   opening                 - Grey-value opening
%   tophat                  - Top-hat
%   lee                     - Lee operator
%   rankmin_closing         - Rank-min closing
%   rankmax_opening         - Rank-max opening
%   areaopening             - Area opening
%   areaclosing             - Area closing
%   pathopening             - Path opening
%   pathclosing             - Path closing
%   hmaxima                 - H-maxima transform
%   hminima                 - H-minima transform
%   reconstruction          - Morphological reconstruction by dilation or erosion
%   asf                     - Alternating sequential filters, morphological smoothing
%
% Diffusion:
%   gaussf                  - Gaussian filter
%   pmd                     - Perona-Malik anisotropic diffusion
%   pmd_gaussian            - Perona-Malik diffusion with Gaussian derivatives
%   aniso                   - Robust anisotropic diffusion using Tukey error norm
%   ced                     - Coherence enhancing (anisotropic) diffusion
%
% Restoration:
%   wiener                  - Wiener deconvolution filter
%
% Segmentation:
%   threshold               - Thresholding
%   hist2image              - Backmaps a 2D histogram ROI to the images
%   minima                  - Detect local minima
%   maxima                  - Detect local maxima
%   watershed               - Watershed
%   waterseed               - 
%   compactwaterseed        - Compact watershed initialized with a seed image
%   stochasticwatershed     - Stochastic watershed
%   superpixels             - Generates superpixels (oversegmentation)
%   cluster                 - Spatial clustering
%   canny                   - Canny edge detector
%   nonmaximumsuppression   - Non-maximum suppression
%   snakeminimize           - Minimizes the energy function of a snake
%   gvf                     - Computes an external force using Gradient Vector Flow
%   vfc                     - Computes an external force using Vector Field Convolution
%   snakedraw               - Draws a snake over an image
%   snake2im                - Creates a binary image based on a snake
%   im2snake                - Creates a snake based on a binary image
%   label                   - Label objects in a binary image
%   relabel                 - Renumber labels in a labeled image
%   setlabels               - Remap or remove labels
%   displaylabelnumbers     - Overlay the label numbers on a figure window
%   traceobjects            - Traces the objects in an image
%   countingframe           - Applies a counting frame to a binary or labelled image
%
% Transforms:
%   ft                      - Fourier Transform (forward)
%   ift                     - Fourier transform (inverse)
%   riesz                   - Riesz transform
%   dt                      - Euclidean distance transform
%   vdt                     - Vector components of Euclidean distance transform
%   gdt                     - Grey-weighted distance transform
%   radoncircle             - Compute Radon transform to find circles/spheres
%   watershed               - Watershed
%   waterseed               - 
%
% Analysis:
%   measure                 - Do measurements on objects in an image
%   msr2obj                 - Label each object in the image with its measurement
%   scalespace              - Gaussian scale-space
%   scale2rgb               - Convert scale-space to RGB image
%   structuretensor         - Computes the structure tensor
%   monogenicsignal         - Computes the monogenic signal, phase congruency and phase symmetry
%   orientationspace        - Orientation space
%   orientation             - Local structure orientation
%   curvature               - Local structure curvature
%   granulometry            - Obtains a particle size distribution.
%   chordlength             - Computes the chord lengths of the phases in a labeled image
%   paircorrelation         - Computes the pair correlation of the phases in a labeled image
%   distancedistribution    - Compute a histogram of distances within each label
%   semivariogram           - Computes the semivariogram of the field
%   autocorrelation         - Computes the auto-correlation of an image
%   crosscorrelation        - Computes the cross-correlation between two images
%   findshift               - Finds shift between two images
%   fmmatch                 - Matches two images using the Fourier Mellin transform
%   cornerdetector          - Corner detector
%   linedetector            - Line detector
%   opticflow               - Computes the optic flow of a 2D/3D time series
%   findlocalshift          - Estimates locally the shift
%   findlocmax              - Finds iteratively the local maximum in a LSIZE cube
%   minima                  - Detect local minima
%   maxima                  - Detect local maxima
%   findminima              - Find local minima, with sub-pixel precision
%   findmaxima              - Find local maxima, with sub-pixel precision
%   subpixlocation          - Find sub-pixel location of extrema
%
% Statistics:
%   diphist                 - Displays a histogram
%   diphist2d               - Generates a 2D histogram
%   mdhistogram             - Compute a multi-dimensional histogram
%   mdhistogrammap          - Reverse map a multi-dimensional histogram
%   perobjecthist           - Compute a histogram for each object in a labeled image
%   getmaximumandminimum    - Find the minimum and maximum sample value
%   getsamplestatistics     - Compute the first four central moments of the image's sample values
%   bbox                    - Bounding box of an n-D binary image
%   radialsum               - Computes the sum as a function of the R-coordinate
%   radialmean              - Computes the average as a function of the R-coordinate (angular mean)
%   radialmin               - Computes the minimum as a function of the R-coordinate
%   radialmax               - Computes the maximum as a function of the R-coordinate
%   errormeasure            - Compares two images
%   noisestd                - Estimate noise standard deviation
%   entropy                 - Computes the entropy (in bits) of an image
%   cal_readnoise           - Calculates the read noise/gain of a CCD
%
% Other available functions are:
%   array2im                - Convert a tensor image to an image stack
%   boundary_condition      - Information on the BOUNDARY_CONDITION parameter (not a function)
%   cell2im                 - Converts a cell array of images to a dip_image
%   dipmex                  - Compile a MEX-file that uses DIPlib
%   im2array                - Convert an image stack to a vector image
%   im2cell                 - Converts a dip_image to a cell array of images
%   im2mat                  - Converts a dip_image to a matlab array
%   mat2im                  - Converts a matlab array to a dip_image
%
% Type
%   methods dip_image
% to get a list of functions overloaded for dip_images.
%
% More information is available in the DIPimage User Manual, type
%   web(dipgetpref('UserManualLocation'),'-browser')

% (c)2017-2019, Cris Luengo.
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
