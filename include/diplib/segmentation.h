/*
 * DIPlib 3.0
 * This file contains declarations for segmentation functions
 *
 * (c)2017, Cris Luengo.
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
///
/// See also watershed functions in the \ref morphology group: `dip::Watershed`, `dip::SeededWatershed`.
/// \{


/// \brief Applies k-means clustering to an image, yielding `nClusters` labeled regions.
///
/// `in` is a scalar, real-valued image. `nClusters` cluster centers are found, centered on regions
/// of high intensity. `out` is a labeled image with `nClusters` regions tiling the image. Each
/// region is identified by a different label. Boundaries between regions are the Voronoi tessellation
/// given the identified cluster centers.
DIP_EXPORT void KMeansClustering( // TODO: return the cluster centers?
      Image const& in,
      Image& out,
      dip::uint nClusters = 2
);
// TODO: port dip_KMeansClustering (dip_analysis.h)

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
/// See `dip::IsodataThreshold(Histogram const&,dip::uint)` for more information on the algorithm used.
DIP_EXPORT FloatArray IsodataThreshold(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint nThresholds = 1
);
inline Image IsodataThreshold(
      Image const& in,
      Image const& mask,
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
/// See `dip::OtsuThreshold(Histogram const&)` for more information on the algorithm used.
DIP_EXPORT dfloat OtsuThreshold(
      Image const& in,
      Image const& mask,
      Image& out
);
inline Image OtsuThreshold(
      Image const& in,
      Image const& mask
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
/// See `dip::MinimumErrorThreshold(Histogram const&)` for more information on the algorithm used.
DIP_EXPORT dfloat MinimumErrorThreshold(
      Image const& in,
      Image const& mask,
      Image& out
);
inline Image MinimumErrorThreshold(
      Image const& in,
      Image const& mask
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
/// See `dip::TriangleThreshold(Histogram const&)` for more information on the algorithm used.
DIP_EXPORT dfloat TriangleThreshold(
      Image const& in,
      Image const& mask,
      Image& out
);
inline Image TriangleThreshold(
      Image const& in,
      Image const& mask
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
/// See `dip::BackgroundThreshold(Histogram const&,dfloat)` for more information on the algorithm used.
DIP_EXPORT dfloat BackgroundThreshold(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat distance = 2.0
);
inline Image BackgroundThreshold(
      Image const& in,
      Image const& mask,
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
      Image const& mask,
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
/// `in` must be scalar and real-valued.
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
      String const& output = "binary"
);
inline Image FixedThreshold(
      Image const& in,
      dfloat threshold,
      dfloat foreground = 1.0,
      dfloat background = 0.0,
      String const& output = "binary"
) {
   Image out;
   FixedThreshold( in, out, threshold, foreground, background, output );
   return out;
}

/// \brief Thresholds an image at two values, equivalent to `lowerBound <= in && in <= upperBound`.
///
/// If `output` is `"binary"` (the default), `%RangeThreshold` will produce a binary image. Otherwise an
/// image of the same type as the input image is produced, with the pixels set to either
/// `foreground` or `background`. In other words, on a pixel-per-pixel basis the following is applied:
/// `out = ( lowerBound <= in && in <= upperBound ) ? foreground : background`.
///
/// `in` must be scalar and real-valued.
///
/// \see dip::InRange, dip::OutOfRange
DIP_EXPORT void RangeThreshold(
      Image const& in,
      Image& out,
      dfloat lowerBound,
      dfloat upperBound,
      dfloat foreground = 1.0,
      dfloat background = 0.0,
      String const& output = "binary"
);
inline Image RangeThreshold(
      Image const& in,
      dfloat lowerBound,
      dfloat upperBound,
      dfloat foreground = 1.0,
      dfloat background = 0.0,
      String const& output = "binary"
) {
   Image out;
   RangeThreshold( in, out, lowerBound, upperBound, foreground, background, output );
   return out;
}

/// \brief Hysteresis threshold.
///
/// From the binary image `in >= lowThreshold` only those regions are selected for which at least one
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
/// `out` will be a `dip::DT_UINT8`, `dip::DT_UINT16` or `dip::DT_UINT32` image, depending on the length
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
///  - "isodata": the Isodata algorithm by Ridler and Calvard (1978), i.e. 2-means clustering.
///  - "otsu": the maximal inter-class variance method by Otsu (1979). This is the default method.
///  - "minerror": the minimal error method by Kittler and Illingworth (1986).
///  - "triangle": the chord method (a.k.a. skewed bi-modality, maximum distance to triangle) by Zack, Rogers and Latt (1977).
///  - "background": using the unimodal background-symmetry method. `parameter` is the distance to the peak
///    where we cut off, in terms of the half-width at half the maximum.
///  - "volume": such that the output has a given volume fraction. `parameter` is the required volume fraction.
///  - "fixed": using the given threshold value. `parameter` is the threshold value.
inline dfloat Threshold(
      Image const& in,
      Image& out,
      String const& method = "otsu",
      dfloat parameter = infinity
) {
   if( method == "isodata" ) {
      FloatArray values = IsodataThreshold( in, {}, out, 1 );
      return values[ 0 ];
   } else if( method == "otsu" ) {
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
      String const& method = "otsu",
      dfloat parameter = infinity
) {
   Image out;
   Threshold( in, out, method, parameter );
   return out;
}


// TODO: Add 2D snakes from diptree/dipimage/mfiles/snakeminimize.m
// TODO: Add level-set segmentation and graph-cut segmentation


/// \}

} // namespace dip

#endif // DIP_SEGMENTATION_H
