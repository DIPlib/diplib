/*
 * DIPlib 3.0
 * This file contains declarations for segmentation functions
 *
 * (c)2017-2019, Cris Luengo.
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


/// \file
/// \brief Functions for segmentation and binarization.
/// \see segmentation


namespace dip {


/// \defgroup segmentation Segmentation
/// \brief Segmentation and binarization algorithms.
/// \{


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
/// suck in local minima. Repeating the clustering several times and picking the best result
/// (e.g. determined by times each cluster center is found) can be necessary.
///
/// The returned `dip::CoordinateArray` contains the cluster centers.
DIP_EXPORT CoordinateArray KMeansClustering(
      Image const& in,
      Image& out,
      dip::uint nClusters = 2
);
inline Image KMeansClustering(
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
///
/// Note that this creates a spatial partitioning, not a partitioning of image intensities.
///
/// Minimum variance partitioning is much faster than k-means clustering, though its result might not be
/// as good. It is also deterministic.
///
/// `in` must be scalar and real-valued.
///
/// The returned `dip::CoordinateArray` contains the centers of gravity.
DIP_EXPORT CoordinateArray MinimumVariancePartitioning(
      Image const& in,
      Image& out,
      dip::uint nClusters = 2
);
inline Image MinimumVariancePartitioning(
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
/// See <tt>\ref dip::IsodataThreshold(Histogram const&, dip::uint) "dip::IsodataThreshold"</tt>
/// for more information on the algorithm used.
DIP_EXPORT FloatArray IsodataThreshold(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint nThresholds = 1
);
inline Image IsodataThreshold(
      Image const& in,
      Image const& mask = {},
      dip::uint nThresholds = 1 // output is binary if nThresholds==1, labeled otherwise
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
/// See <tt>\ref dip::OtsuThreshold(Histogram const&) "dip::OtsuThreshold"</tt>
/// for more information on the algorithm used.
DIP_EXPORT dfloat OtsuThreshold(
      Image const& in,
      Image const& mask,
      Image& out
);
inline Image OtsuThreshold(
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
/// See <tt>\ref dip::MinimumErrorThreshold(Histogram const&) "dip::MinimumErrorThreshold"</tt>
/// for more information on the algorithm used.
DIP_EXPORT dfloat MinimumErrorThreshold(
      Image const& in,
      Image const& mask,
      Image& out
);
inline Image MinimumErrorThreshold(
      Image const& in,
      Image const& mask = {}
) {
   Image out;
   MinimumErrorThreshold( in, mask, out );
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
/// See <tt>\ref dip::TriangleThreshold(Histogram const&) "dip::TriangleThreshold"</tt>
/// for more information on the algorithm used.
DIP_EXPORT dfloat TriangleThreshold(
      Image const& in,
      Image const& mask,
      Image& out
);
inline Image TriangleThreshold(
      Image const& in,
      Image const& mask = {}
) {
   Image out;
   TriangleThreshold( in, mask, out );
   return out;
}

/// \brief Thresholds the image `in` using the unimodal background-symmetry method, and the histogram of `in`.
///
/// Only those pixels in `mask` are used to determine the histogram on which the threshold estimation algorithm
/// is applied, but the threshold is applied to the whole image. `in` must be scalar and real-valued.
///
/// Returns the threshold value used.
///
/// See <tt>\ref dip::BackgroundThreshold(Histogram const&, dfloat) "dip::BackgroundThreshold"</tt>
/// for more information on the algorithm used.
DIP_EXPORT dfloat BackgroundThreshold(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat distance = 2.0
);
inline Image BackgroundThreshold(
      Image const& in,
      Image const& mask = {},
      dfloat distance = 2.0
) {
   Image out;
   BackgroundThreshold( in, mask, out, distance );
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
inline Image VolumeThreshold(
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
/// If `output` is `"binary"` (the default), `%FixedThreshold` will produce a binary image. Otherwise an
/// image of the same type as the input image is produced, with the pixels set to either
/// `foreground` or `background`. In other words, on a pixel-per-pixel basis the following is applied:
/// `out = ( in >= threshold ) ? foreground : background`.
///
/// `in` must be real-valued, each tensor element is thresholded independently.
///
/// Note that, for the "binary" output case, it might be easier to write:
///
/// ```cpp
///     out = in >= threshold;
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
inline Image FixedThreshold(
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
/// If `output` is `"binary"` (the default), `%RangeThreshold` will produce a binary image. If `foreground == 0.0`,
/// foreground will be set to `false` and background to `true`, otherwise the foreground will be `true` (this is the
/// default).
///
/// If `output` is not `"binary"`, an image of the same type as the input image is produced, with the pixels
/// set to either `foreground` or `background`. In other words, on a pixel-per-pixel basis the following is
/// applied:
/// ```cpp
///     out = ( lowerBound <= in && in <= upperBound ) ? foreground : background
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
inline Image RangeThreshold(
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
inline Image HysteresisThreshold(
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
/// `out` will be a `dip::DT_UINT8`, `dip::DT_UINT16`, `dip::DT_UINT32` or `dip::DT_UINT64` image, depending on the length
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
inline Image MultipleThresholds(
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
/// threshold value. `in` must be scalar and real-valued.
///
/// `method` can be one of:
///  - "isodata": see <tt>\ref dip::IsodataThreshold(Image const&, Image const&, Image&, dip::uint) "dip::IsodataThreshold"</tt>.
///  - "otsu": see <tt>\ref dip::OtsuThreshold(Image const&, Image const&, Image&) "dip::OtsuThreshold"</tt>. This is the default method
///  - "minerror": see <tt>\ref dip::MinimumErrorThreshold(Image const&, Image const&, Image&) "dip::MinimumErrorThreshold"</tt>.
///  - "triangle": see <tt>\ref dip::TriangleThreshold(Image const&, Image const&, Image&) "dip::TriangleThreshold"</tt>.
///  - "background": see <tt>\ref dip::BackgroundThreshold(Image const&, Image const&, Image&, dfloat) "dip::BackgroundThreshold"</tt>.
///  - "volume": see <tt>\ref dip::VolumeThreshold(Image const&, Image const&, Image&, dfloat) "dip::VolumeThreshold"</tt>.
///  - "fixed": see <tt>\ref dip::FixedThreshold(Image const&, Image&, dfloat, dfloat, dfloat, String const&) "dip::FixedThreshold"</tt>. The default parameter value is 128.
///
/// If `parameter` is `dip::infinity`, the default parameter value for the method will be used.
inline dfloat Threshold(
      Image const& in,
      Image& out,
      String const& method = S::OTSU,
      dfloat parameter = infinity
) {
   if( method == "isodata" ) {
      FloatArray values = IsodataThreshold( in, {}, out, 1 );
      return values[ 0 ];
   } else if( method == S::OTSU ) {
      return OtsuThreshold( in, {}, out );
   } else if( method == "minerror" ) {
      return MinimumErrorThreshold( in, {}, out );
   } else if( method == "triangle" ) {
      return TriangleThreshold( in, {}, out );
   } else if( method == "background" ) {
      if( parameter == infinity ) {
         return BackgroundThreshold( in, {}, out );
      } else {
         return BackgroundThreshold( in, {}, out, parameter );
      }
   } else if( method == "volume" ) {
      if( parameter == infinity ) {
         return VolumeThreshold( in, {}, out );
      } else {
         return VolumeThreshold( in, {}, out, parameter );
      }
   } else if( method == "fixed" ) {
      if( parameter == infinity ) {
         parameter = 128.0;
      }
      FixedThreshold( in, out, parameter );
      return parameter;
   } else {
      DIP_THROW_INVALID_FLAG( method );
   }
}
inline Image Threshold(
      Image const& in,
      String const& method = S::OTSU,
      dfloat parameter = infinity
) {
   Image out;
   Threshold( in, out, method, parameter );
   return out;
}


/// \brief Detect edges in the grey-value image by finding salient ridges in the gradient magnitude
///
/// The Canny edge detector finds the ridges in the gradient magnitude of `in`, which correspond to the
/// edges in the image. The gradient magnitude (see `dip::GradientMagnitude`) is computed using
/// Gaussian derivatives, with a sigma of `sigma`. The found ridges are pruned to remove the less
/// salient edges (see `dip::NonMaximumSuppression`). Next, a threshold `t1` is computed such that
/// the `1 - upper` fraction of pixels with the highest gradient magnitude are kept. A second threshold,
/// `t2 = t1 * lower`, is selected that determines the minimal gradient magnitude expected for an edge.
/// All edge pixels equal or larger to `t2`, and are in the same connected region as at least one pixel that
/// is equal or larger to `t1`, are selected as the output of this function (see `dip::HysteresisThreshold`).
/// Finally, a homotopic thinning is applied to reduce the detections to single-pixel--thick lines
/// (see `dip::EuclideanSkeleton`).
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
/// only applied in 2D and 3D, since `dip::EuclideanSkeleton` is not defined for other dimensionalities.
///
/// **Literature**
/// - J. Canny, "A Computational Approach to Edge Detection", IEEE Transactions on Pattern Analysis
///   and Machine Intelligence, 8(6):679-697, 1986.
DIP_EXPORT void Canny(
      Image const& in,
      Image& out,
      FloatArray const& sigmas = { 1 },
      dfloat lower = 0.5,
      dfloat upper = 0.9,
      String const& selection = S::ALL
);
inline Image Canny(
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

/// \}

} // namespace dip

#endif // DIP_SEGMENTATION_H
