/*
 * (c)2017-2022, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#ifndef DIP_SEGMENTATION_H
#define DIP_SEGMENTATION_H

#include "diplib.h"
#include "diplib/random.h"


/// \file
/// \brief Functions for segmentation and binarization.
/// See \ref segmentation.


namespace dip {


/// \group segmentation Segmentation
/// \brief Segmentation and binarization algorithms.
/// \addtogroup


/// \brief Applies k-means clustering to an image, yielding `nClusters` labeled regions.
///
/// `in` is a scalar, real-valued image. `nClusters` cluster centers are found, centered on regions
/// of high intensity. `out` is a labeled image with `nClusters` regions tiling the image. Each
/// region is identified by a different label. Boundaries between regions are the Voronoi tessellation
/// given the identified cluster centers.
///
/// Note that this creates a spatial partitioning, not a partitioning of image intensities.
///
/// K-means clustering is an iterative process with a random initialization. It is likely to get
/// stuck in local minima. Repeating the clustering several times and picking the best result
/// (e.g. determined by times each cluster center is found) can be necessary.
///
/// The returned \ref dip::CoordinateArray contains the cluster centers.
/// Element `i` in this array corresponds to label `i+1`.
DIP_EXPORT CoordinateArray KMeansClustering(
      Image const& in,
      Image& out,
      Random& random,
      dip::uint nClusters = 2
);
DIP_NODISCARD inline Image KMeansClustering(
      Image const& in,
      Random& random,
      dip::uint nClusters = 2
) {
   Image out;
   KMeansClustering( in, out, random, nClusters );
   return out;
}
/// \brief Like above, using a default-initialized \ref dip::Random object.
inline CoordinateArray KMeansClustering(
      Image const& in,
      Image& out,
      dip::uint nClusters = 2
) {
   Random random;
   return KMeansClustering( in, out, random, nClusters );
}
DIP_NODISCARD inline Image KMeansClustering(
      Image const& in,
      dip::uint nClusters = 2
) {
   Image out;
   KMeansClustering( in, out, nClusters );
   return out;
}

/// \brief Spatially partitions an image into `nClusters` partitions iteratively, minimizing the variance
/// of the partitions.
///
/// Minimum variance partitioning builds a k-d tree, where, for each node, the orthogonal projection
/// with the largest variance is split using the same logic as Otsu thresholding applies to a histogram.
/// Note that this creates a spatial partitioning, not a partitioning of image intensities. `out` is
/// a labeled image with `nClusters` regions tiling the image. Each region is identified by a different
/// label.
///
/// Minimum variance partitioning is much faster than k-means clustering, though its result might not be
/// as good. It is also deterministic.
///
/// `in` must be scalar and real-valued.
///
/// The returned \ref dip::CoordinateArray contains the centers of gravity for each cluster.
/// Element `i` in this array corresponds to label `i+1`.
DIP_EXPORT CoordinateArray MinimumVariancePartitioning(
      Image const& in,
      Image& out,
      dip::uint nClusters = 2
);
DIP_NODISCARD inline Image MinimumVariancePartitioning(
      Image const& in,
      dip::uint nClusters = 2
) {
   Image out;
   MinimumVariancePartitioning( in, out, nClusters );
   return out;
}


/// \brief Thresholds the image `in` using `nThresholds` thresholds, determined using the Isodata algorithm
/// (k-means clustering), and the histogram of `in`.
///
/// Only those pixels in `mask` are used to determine the histogram on which the Isodata algorithm is applied,
/// but the threshold is applied to the whole image. `in` must be scalar and real-valued.
///
/// If `nThresholds` is 1, then `out` is a binary image. With more thresholds, the output image is labeled.
///
/// The output array contains the thresholds used.
///
/// See \ref dip::IsodataThreshold(Histogram const&, dip::uint) for more information on the algorithm used.
DIP_EXPORT FloatArray IsodataThreshold(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint nThresholds = 1
);
DIP_NODISCARD inline Image IsodataThreshold(
      Image const& in,
      Image const& mask = {},
      dip::uint nThresholds = 1
) {
   Image out;
   IsodataThreshold( in, mask, out, nThresholds );
   return out;
}

/// \brief Thresholds the image `in` using the maximal inter-class variance method by Otsu, and the histogram of `in`.
///
/// Only those pixels in `mask` are used to determine the histogram on which the threshold estimation algorithm
/// is applied, but the threshold is applied to the whole image. `in` must be scalar and real-valued.
///
/// Returns the threshold value used.
///
/// See \ref dip::OtsuThreshold(Histogram const&) for more information on the algorithm used.
DIP_EXPORT dfloat OtsuThreshold(
      Image const& in,
      Image const& mask,
      Image& out
);
DIP_NODISCARD inline Image OtsuThreshold(
      Image const& in,
      Image const& mask = {}
) {
   Image out;
   OtsuThreshold( in, mask, out );
   return out;
}

/// \brief Thresholds the image `in` using the minimal error method by Kittler and Illingworth, and the histogram of `in`.
///
/// Only those pixels in `mask` are used to determine the histogram on which the threshold estimation algorithm
/// is applied, but the threshold is applied to the whole image. `in` must be scalar and real-valued.
///
/// Returns the threshold value used.
///
/// See \ref dip::MinimumErrorThreshold(Histogram const&) for more information on the algorithm used.
DIP_EXPORT dfloat MinimumErrorThreshold(
      Image const& in,
      Image const& mask,
      Image& out
);
DIP_NODISCARD inline Image MinimumErrorThreshold(
      Image const& in,
      Image const& mask = {}
) {
   Image out;
   MinimumErrorThreshold( in, mask, out );
   return out;
}

/// \brief Thresholds the image `in` using `nThresholds` thresholds, determined by fitting a Gaussian Mixture
/// Model to the histogram of `in`.
///
/// Only those pixels in `mask` are used to determine the histogram on which the Gaussian Mixture Model
/// algorithm is applied, but the threshold is applied to the whole image. `in` must be scalar and real-valued.
///
/// If `nThresholds` is 1, then `out` is a binary image. With more thresholds, the output image is labeled.
///
/// The output array contains the thresholds used.
///
/// See \ref dip::GaussianMixtureModelThreshold(Histogram const&, dip::uint) for more information on the algorithm used.
DIP_EXPORT FloatArray GaussianMixtureModelThreshold(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint nThresholds = 1
);
DIP_NODISCARD inline Image GaussianMixtureModelThreshold(
      Image const& in,
      Image const& mask = {},
      dip::uint nThresholds = 1
) {
   Image out;
   GaussianMixtureModelThreshold( in, mask, out, nThresholds );
   return out;
}

/// \brief Thresholds the image `in` using the chord method (a.k.a. skewed bi-modality, maximum distance to triangle),
/// and the histogram of `in`.
///
/// Only those pixels in `mask` are used to determine the histogram on which the threshold estimation algorithm
/// is applied, but the threshold is applied to the whole image. `in` must be scalar and real-valued.
///
/// Returns the threshold value used.
///
/// See \ref dip::TriangleThreshold(Histogram const&, dfloat) for more information on the algorithm used and the `sigma`
/// parameter.
DIP_EXPORT dfloat TriangleThreshold(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat sigma = 4.0
);
DIP_NODISCARD inline Image TriangleThreshold(
      Image const& in,
      Image const& mask = {},
      dfloat sigma = 4.0
) {
   Image out;
   TriangleThreshold( in, mask, out, sigma );
   return out;
}

/// \brief Thresholds the image `in` using the unimodal background-symmetry method, and the histogram of `in`.
///
/// Only those pixels in `mask` are used to determine the histogram on which the threshold estimation algorithm
/// is applied, but the threshold is applied to the whole image. `in` must be scalar and real-valued.
///
/// Returns the threshold value used.
///
/// See \ref dip::BackgroundThreshold(Histogram const&, dfloat, dfloat) for more information on the algorithm used
/// and the `sigma` parameter.
DIP_EXPORT dfloat BackgroundThreshold(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat distance = 2.0,
      dfloat sigma = 4.0
);
DIP_NODISCARD inline Image BackgroundThreshold(
      Image const& in,
      Image const& mask = {},
      dfloat distance = 2.0,
      dfloat sigma = 4.0
) {
   Image out;
   BackgroundThreshold( in, mask, out, distance, sigma );
   return out;
}

/// \brief Thresholds an image such that a fraction `volumeFraction` of pixels is foreground.
///
/// Only pixels within `mask` are used to determine the threshold value, but the threshold is applied to the
/// whole image. `in` must be scalar and real-valued.
///
/// The return value is the threshold applied.
DIP_EXPORT dfloat VolumeThreshold(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat volumeFraction = 0.5
);
DIP_NODISCARD inline Image VolumeThreshold(
      Image const& in,
      Image const& mask = {},
      dfloat volumeFraction = 0.5
) {
   Image out;
   VolumeThreshold( in, mask, out, volumeFraction );
   return out;
}

/// \brief Thresholds an image at the `threshold` value.
///
/// If `output` is `"binary"` (the default), `FixedThreshold` will produce a binary image. Otherwise an
/// image of the same type as the input image is produced, with the pixels set to either
/// `foreground` or `background`. In other words, on a pixel-per-pixel basis the following is applied:
/// `out = ( in >= threshold ) ? foreground : background`.
///
/// `in` must be real-valued, each tensor element is thresholded independently.
///
/// Note that, for the "binary" output case, it might be easier to write:
///
/// ```cpp
/// out = in >= threshold;
/// ```
///
/// \see dip::NotGreater, dip::NotLesser, dip::Select
DIP_EXPORT void FixedThreshold(
      Image const& in,
      Image& out,
      dfloat threshold,
      dfloat foreground = 1.0,
      dfloat background = 0.0,
      String const& output = S::BINARY
);
DIP_NODISCARD inline Image FixedThreshold(
      Image const& in,
      dfloat threshold,
      dfloat foreground = 1.0,
      dfloat background = 0.0,
      String const& output = S::BINARY
) {
   Image out;
   FixedThreshold( in, out, threshold, foreground, background, output );
   return out;
}

/// \brief Thresholds an image at two values, equivalent to `lowerBound <= in && in <= upperBound`.
///
/// If `output` is `"binary"` (the default), `RangeThreshold` will produce a binary image. If `foreground == 0.0`,
/// foreground will be set to `false` and background to `true`, otherwise the foreground will be `true` (this is the
/// default).
///
/// If `output` is not `"binary"`, an image of the same type as the input image is produced, with the pixels
/// set to either `foreground` or `background`. In other words, on a pixel-per-pixel basis the following is
/// applied:
///
/// ```cpp
/// out = ( lowerBound <= in && in <= upperBound ) ? foreground : background
/// ```
///
/// `in` must be real-valued, each tensor element is thresholded independently.
///
/// \see dip::InRange, dip::OutOfRange
DIP_EXPORT void RangeThreshold(
      Image const& in,
      Image& out,
      dfloat lowerBound,
      dfloat upperBound,
      String const& output = S::BINARY,
      dfloat foreground = 1.0,
      dfloat background = 0.0
);
DIP_NODISCARD inline Image RangeThreshold(
      Image const& in,
      dfloat lowerBound,
      dfloat upperBound,
      String const& output = S::BINARY,
      dfloat foreground = 1.0,
      dfloat background = 0.0
) {
   Image out;
   RangeThreshold( in, out, lowerBound, upperBound, output, foreground, background );
   return out;
}

/// \brief Hysteresis threshold.
///
/// From the binary image `in >= lowThreshold` only those connected regions are selected for which at least one
/// location also has `in >= highThreshold`.
///
/// The output image will be a binary image with foreground pixels == 1 and background pixels == 0.
///
/// `in` must be scalar and real-valued.
DIP_EXPORT void HysteresisThreshold(
      Image const& in,
      Image& out,
      dfloat lowThreshold,
      dfloat highThreshold
);
DIP_NODISCARD inline Image HysteresisThreshold(
      Image const& in,
      dfloat lowThreshold,
      dfloat highThreshold
) {
   Image out;
   HysteresisThreshold( in, out, lowThreshold, highThreshold );
   return out;
}

/// \brief Thresholds an image at multiple values, yielding a labeled image.
///
/// `out` will be a \ref dip::DT_UINT8, \ref dip::DT_UINT16, \ref dip::DT_UINT32 or \ref dip::DT_UINT64 image, depending on the length
/// of `thresholds`. All pixels below `thresholds[ 0 ]` with be assigned the label 0, all pixels greater or
/// equal to `thresholds[ 0 ]` and smaller than `thresholds[ 1 ]` will be assigned label 1, etc. Results might
/// not be as expected if thresholds are not sorted.
///
/// `in` must be scalar and real-valued.
DIP_EXPORT void MultipleThresholds(
      Image const& in,
      Image& out,
      FloatArray const& thresholds
);
DIP_NODISCARD inline Image MultipleThresholds(
      Image const& in,
      FloatArray const& thresholds
) {
   Image out;
   MultipleThresholds( in, out, thresholds );
   return out;
}

/// \brief Automated threshold using `method`.
///
/// This function computes an optimal threshold value for `in` using `method`, and applies it. Returns the found
/// threshold value. `in` must be scalar and real-valued. `mask` can optionally select the pixels used to determine
/// the threshold value. The threshold is applied to the image as a whole, you can combine it with the mask afterwards:
/// ```cpp
/// dip::Image bin = dip::Threshold( image, mask, "otsu" );
/// bin &= mask;
/// ```
///
/// `method` can be one of:
///
/// - `"isodata"`: see \ref dip::IsodataThreshold(Image const&, Image const&, Image&, dip::uint).
/// - `"otsu"`: see \ref dip::OtsuThreshold(Image const&, Image const&, Image&). This is the default method.
/// - `"minerror"`: see \ref dip::MinimumErrorThreshold(Image const&, Image const&, Image&)".
/// - `"gmm"`: see \ref dip::GaussianMixtureModelThreshold(Image const&, Image const&, Image&, dip::uint).
/// - `"triangle"`: see \ref dip::TriangleThreshold(Image const&, Image const&, Image&, dfloat).
/// - `"background"`: see \ref dip::BackgroundThreshold(Image const&, Image const&, Image&, dfloat, dfloat).
/// - `"volume"`: see \ref dip::VolumeThreshold(Image const&, Image const&, Image&, dfloat).
/// - `"fixed"`: see \ref dip::FixedThreshold(Image const&, Image&, dfloat, dfloat, dfloat, String const&).
///   The default parameter value is 128.
///
/// If `parameter` is \ref dip::infinity, the default parameter value for the method will be used.
inline dfloat Threshold(
      Image const& in,
      Image const& mask,
      Image& out,
      String const& method = S::OTSU,
      dfloat parameter = infinity
) {
   if( method == "isodata" ) {
      FloatArray values = IsodataThreshold( in, mask, out, 1 );
      return values[ 0 ];
   }
   if( method == S::OTSU ) {
      return OtsuThreshold( in, mask, out );
   }
   if( method == "minerror" ) {
      return MinimumErrorThreshold( in, mask, out );
   }
   if( method == "gmm" ) {
      FloatArray values = GaussianMixtureModelThreshold( in, mask, out, 1 );
      return values[ 0 ];
   }
   if( method == "triangle" ) {
      return ( parameter == infinity ) ? TriangleThreshold( in, mask, out )
                                       : TriangleThreshold( in, mask, out, parameter );
   }
   if( method == "background" ) {
      return ( parameter == infinity ) ? BackgroundThreshold( in, mask, out )
                                       : BackgroundThreshold( in, mask, out, parameter );
   }
   if( method == "volume" ) {
      return ( parameter == infinity ) ? VolumeThreshold( in, mask, out )
                                       : VolumeThreshold( in, mask, out, parameter );
   }
   if( method == "fixed" ) {
      if( parameter == infinity ) {
         parameter = 128.0;
      }
      FixedThreshold( in, out, parameter );
      return parameter;
   }
   DIP_THROW_INVALID_FLAG( method );
}
DIP_NODISCARD inline Image Threshold(
      Image const& in,
      Image const& mask,
      String const& method = S::OTSU,
      dfloat parameter = infinity
) {
   Image out;
   Threshold( in, mask, out, method, parameter );
   return out;
}
DIP_NODISCARD inline Image Threshold(
      Image const& in,
      String const& method = S::OTSU,
      dfloat parameter = infinity
) {
   return Threshold( in, {}, method, parameter );
}

/// \brief Defines the parameters for the `PerObjectEllipsoidFit` function.
struct PerObjectEllipsoidFitParameters {
   dip::uint minSize = 25;          ///< Area in pixels of the smallest object detected
   dip::uint maxArea = 25000;       ///< Area in pixels of the largest object detected
   dfloat minEllipsoidFit = 0.88;   ///< Smallest allowed ratio of object size vs fitted ellipse size
   dfloat minAspectRatio = 1.0;     ///< Smallest allowed aspect ratio of ellipse (largest radius divided by smallest radius); 1.0 is a circle/sphere
   dfloat maxAspectRatio = 10.0;    ///< Largest allowed aspect ratio
   dfloat minThreshold = 0.0;       ///< Smallest allowed threshold
   dfloat maxThreshold = 255.0;     ///< Largest allowed threshold
};

/// \brief Finds a per-object threshold such that found objects are maximally ellipsoidal.
///
/// This function thresholds the image such that all objects found are approximately ellipsoidal, within the bounds expressed
/// by `parameters`. Each object is thresholded at a different level, chosen to maximize its fit to an ellipsoid. The measure
/// maximized is the ratio of the object's size (area or volume) to the size of the fitted ellipsoid. Ellipsoids are fitted by
/// determining the ellipsoid with the same second order central moments as the object at the given threshold level.
///
/// `in` must be scalar, real-valued, and be 2D (TODO: port the 3D version of this function also).
/// `out` will be binary and of the same sizes as `in`.
///
/// !!! literature
///     - P. Ranefall, S.K. Sadanandan, C. Wahlby, "Fast Adaptive Local Thresholding Based on Ellipse Fit",
///       International Symposium on Biomedical Imaging (ISBI'16), Prague, Czech Republic, 2016.
DIP_EXPORT void PerObjectEllipsoidFit(
      Image const& in,
      Image& out,
      PerObjectEllipsoidFitParameters const& parameters

);
DIP_NODISCARD inline Image PerObjectEllipsoidFit(
      Image const& in,
      PerObjectEllipsoidFitParameters const& parameters

) {
   Image out;
   PerObjectEllipsoidFit( in, out, parameters );
   return out;
}


/// \brief Detect edges in the grey-value image by finding salient ridges in the gradient magnitude
///
/// The Canny edge detector finds the ridges in the gradient magnitude of `in`, which correspond to the
/// edges in the image. The gradient magnitude (see \ref dip::GradientMagnitude) is computed using
/// Gaussian derivatives, with a sigma of `sigma`. The found ridges are pruned to remove the less
/// salient edges (see \ref dip::NonMaximumSuppression). Next, a threshold `t1` is computed such that
/// the `1 - upper` fraction of pixels with the highest gradient magnitude are kept. A second threshold,
/// `t2 = t1 * lower`, is selected that determines the minimal gradient magnitude expected for an edge.
/// All edge pixels equal or larger to `t2`, and are in the same connected region as at least one pixel that
/// is equal or larger to `t1`, are selected as the output of this function (see \ref dip::HysteresisThreshold).
/// Finally, a homotopic thinning is applied to reduce the detections to single-pixel--thick lines
/// (see \ref dip::EuclideanSkeleton).
///
/// The `1 - upper` fraction is computed over all pixels in the image by default. If the image has relatively
/// few edges, this can lead to `t1` being equal to 0. If this happens, the hysteresis threshold would select
/// all pixels in the image, and the homotopic thinning will lead to a line across the image that is
/// unrelated to any edges. Instead, `t1` will be set to a value slightly larger than 0.
///
/// For more control over the thresholds, the `selection` parameter can be set to `"nonzero"`, in which
/// case the fraction `1 - upper` refers to non-zero pixels only; or to `"absolute"`, in which case
/// `upper` and `lower` represent absolute threshold values, and `t1` will be set to `upper` and `t2`
/// will be set to `lower`.
///
/// `in` must be scalar, real-valued, and have at least one dimension.
///
/// The Canny edge detector was originally described, and typically implemented, for 2D images only.
/// Here we provide an obvious extension to arbitrary dimensions. The final homotopic thinning is
/// only applied in 2D and 3D, since \ref dip::EuclideanSkeleton is not defined for other dimensionalities.
///
/// !!! literature
///     - J. Canny, "A Computational Approach to Edge Detection", IEEE Transactions on Pattern Analysis
///       and Machine Intelligence, 8(6):679-697, 1986.
DIP_EXPORT void Canny(
      Image const& in,
      Image& out,
      FloatArray const& sigmas = { 1 },
      dfloat lower = 0.5,
      dfloat upper = 0.9,
      String const& selection = S::ALL
);
DIP_NODISCARD inline Image Canny(
      Image const& in,
      FloatArray const& sigmas = { 1 },
      dfloat lower = 0.5,
      dfloat upper = 0.9,
      String const& selection = S::ALL
) {
   Image out;
   Canny( in, out, sigmas, lower, upper, selection );
   return out;
}

/// \brief Generates superpixels (oversegmentation)
///
/// `density` indicates how many superpixels, on average, should be created. It is given in superpixels per pixel.
/// That is, `1/density` is the average size of the superpixels.
///
/// `compactness` controls the shape of the superpixels. Reducing this value leads to superpixels that more
/// precisely follow image contours, but also are more varied in size and shape. Increasing this value leads
/// to more isotropic superpixels and less variation in size.
///
/// `method` controls the method used to generate superpixels. Currently only `"CW"` is supported. This is the
/// compact watershed superpixel segmentation (Neubert and Protzel, 2014).
///
/// `flags` can contain the following flags:
///
/// - `"rectangular"` (default) or `"hexagonal"`: controls the basic shape of the superpixels (the shape they tend
///   towards as `compactness` increases). For 3D images, `"hexagonal"` implies an FCC grid (see \ref dip::FillRandomGrid).
///   For images with more than 3 dimensions, `"rectangular"` will always be used.
///
/// - `"no gaps"`  indicates that the superpixels must cover the whole image. By default a 1-pixel gap is left in
///   between superpixels.
///
/// `in` must be real-valued. If not scalar, the norm of the gradient magnitude for each tensor element is used
/// to determine where edges are located. In the case of a color image, no color space conversion is performed,
/// the image is used as-is. It is recommended to pass an image in an appropriate color space for superpixel
/// segmentation, such as CIE Lab.
///
/// \see dip::FillRandomGrid, dip::CompactWatershed
///
/// !!! literature
///     - P. Neubert and P. Protzel, "Compact Watershed and Preemptive SLIC: On improving trade-offs of superpixel segmentation algorithms",
///       22^nd^ International Conference on Pattern Recognition, Stockholm, 2014, pp. 996-1001.
DIP_EXPORT void Superpixels(
      Image const& in,
      Image& out,
      Random& random,
      dfloat density = 0.005,
      dfloat compactness = 1.0,
      String const& method = S::CW,
      StringSet const& flags = {}
);
DIP_NODISCARD inline Image Superpixels(
      Image const& in,
      Random& random,
      dfloat density = 0.005,
      dfloat compactness = 1.0,
      String const& method = S::CW,
      StringSet const& flags = {}
) {
   Image out;
   Superpixels( in, out, random, density, compactness, method, flags );
   return out;
}
/// \brief Like above, using a default-initialized \ref dip::Random object.
inline void Superpixels(
      Image const& in,
      Image& out,
      dfloat density = 0.005,
      dfloat compactness = 1.0,
      String const& method = S::CW,
      StringSet const& flags = {}
) {
   Random random;
   Superpixels( in, out, random, density, compactness, method, flags );
}
DIP_NODISCARD inline Image Superpixels(
      Image const& in,
      dfloat density = 0.005,
      dfloat compactness = 1.0,
      String const& method = S::CW,
      StringSet const& flags = {}
) {
   Image out;
   Superpixels( in, out, density, compactness, method, flags );
   return out;
}

/// \brief Graph-cut segmentation
///
/// Applies the graph-cut segmentation algorithm to the image `in` as described by Boykov and Jolly (2001).
/// Pixels in `markers` with the value 1 are determined by the called to be object pixels; pixels
/// with the value 2 are background pixels. All other pixels will be assigned to either foreground
/// or background by the algorithm.
///
/// A \ref DirectedGraph is constructed in which each pixel is a vertex. Neighboring pixels
/// (4-connected neighborhood in 2D, 6-connected in 3D) are connected with an edge in either direction,
/// both with a weight $w$ given by
///
/// $$ w = \exp( \frac{-(v_1 - v_2)^2}{2 \sigma^2} ) \; ,$$
///
/// where $\sigma$ is given by `sigma`, and $v_1$ and $v_2$ are the two pixel's intensities.
///
/// Additionally, two terminal nodes are added to the graph (the source and the sink nodes). These
/// are joined by edges to all pixels. The weights of these edges are determined by `markers`. Pixels where
/// `markers == 1` (the foreground marker) are connected to the source node by an edge with an infinite weight.
/// Likewise, pixels where `markers == 2` (the background marker) are connected to the sink node by an edge
/// with an infinite weight.
/// The weight of edges to all other pixels is determined by intensity statistics of the pixels known
/// to be foreground and background, and the distances to those pixels, as described next.
///
/// The function computes a histogram $H_1$ of all pixels marked as foreground, a histogram $H_2$ of all pixels
/// marked as background, a distance $D_1(p)$ from each pixel $p$ to the nearest foreground pixel, and a distance
/// $D_2(p)$ from each pixel $p$ to the nearest background pixel.
///
/// The histograms $H_1$ and $H_2$ are smoothed according to the Freedman–Diaconis rule for bin width,
/// to approximate a kernel density estimate of the pixel intensities of foreground and background.
/// We now compute $R_1 = -\ln \frac{H_1}{\sum H_1}$ and likewise for $R_2$. We denote $R_1(p)$ the value of
/// this function at the bin corresponding to the intensity of pixel $p$.
///
/// The weights of the edges from the source node to all pixels $p$ (that are not in a marker) is now given by
///
/// $$ w = \lambda R_2(p) + \gamma D_2(p) \; .$$
///
/// (Indeed, the source weights are determined using the intensity statistics and distances for the background marker).
/// $\lambda$ is given by `lambda` and $\gamma$ is given by `gamma`.
/// The weights of the edges for the sink node are computed identically using $R_1(p)$ and $D_1(p)$.
///
/// `lambda` controls the relative importance of intensity information with respect to the edges in the image.
///
/// `gamma` controls the relative importance of distances to the markers.
/// It is 0 by default, as Boykov and Jolly did not mention distances in their original paper.
/// Adding in a distance is an attempt to avoid the bias towards placing the segmentation boundary tightly around
/// the foreground or background marker. The value of `gamma` obviously must depend on the size of the image and
/// the distances between foreground and background markers, and should always be very small to avoid these
/// distances trumping everything. Note that the distances computed are influenced by the pixel sizes of `in`.
///
/// We can make some simplifications to the graph in a way that the minimal cut is not affected. This simplification
/// allows us to have, for each pixel, either an edge to only the source or to only the sink, but never both. This
/// saves a significant amount of memory and computation time.
///
/// Finally, this function calls \ref GraphCut(DirectedGraph&, DirectedGraph::VertexIndex, DirectedGraph::VertexIndex)
/// to compute the globally optimal segmentation of the graph. All pixels connected to the source node will
/// become object pixels in the output binary image.
///
/// `in` must be scalar and real-valued. `markers` must have the same sizes and be of an unsigned integer type.
///
/// !!! literature
///     - Y.Y. Boykov and M.P. Jolly, "Interactive graph cuts for optimal boundary and region segmentation of objects in N-D images",
///       Proceedings Eighth IEEE International Conference on Computer Vision (ICCV 2001) 1:105-112, 2001.
///
/// !!! attention
///     This is a slow algorithm that uses a lot of memory. It is not suited for very large images.
///     It is usually advantageous to work with superpixels if a graph-cut segmentation is needed.
DIP_EXPORT void GraphCut(
   Image const& in,
   Image const& markers,
   Image& out,
   dfloat sigma = 30.0,
   dfloat lambda = 1.0,
   dfloat gamma = 0.0
);
DIP_NODISCARD inline Image GraphCut(
   Image const& in,
   Image const& markers,
   dfloat sigma = 30.0,
   dfloat lambda = 1.0,
   dfloat gamma = 0.0
) {
   Image out;
   GraphCut( in, markers, out, sigma, lambda, gamma );
   return out;
}

/// \endgroup

} // namespace dip

#endif // DIP_SEGMENTATION_H
