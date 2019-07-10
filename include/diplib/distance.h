/*
 * DIPlib 3.0
 * This file contains declarations for distance transforms
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

#ifndef DIP_DISTANCE_H
#define DIP_DISTANCE_H

#include "diplib.h"
#include "diplib/neighborlist.h"


/// \file
/// \brief Functions that compute distance transforms.
/// \see distance


namespace dip {


/// \defgroup distance Distance transforms
/// \brief Various distance transforms.
/// \{

/// \brief Euclidean distance transform
///
/// This function computes the Euclidean distance transform with one of several different methods. The distance
/// transform `out` indicates the distances from the objects (binary 1's) to the nearest background (binary 0's) of
/// `in`. `out` is of type `dip::DT_SFLOAT`, and is zero everywhere where `in` is zero.
///
/// Computed distances use the pixel sizes (ignoring any units). To compute distances in pixels, reset the pixel
/// size (dip::Image::ResetPixelSize). Note that, when pixels sizes are correctly set, this function handles
/// anisotropic sampling densities correctly.
///
/// The `border` parameter specifies whether the edge of the image should be treated as objects (`"object"`) or
/// as background (`"background"`).
///
/// The `method` parameter specifies the method to use to compute the distances. There are three general algorithms
/// implemented:
///
/// \m_class{m-spaced-list}
///
///  1. A separable algorithm based on parabolic erosions as first described by van den Boomgaard (1992) and later by
///     Meijster et al. (2002), but for some reason frequently referred to as "Felzenszwalb and Huttenlocher", who published
///     the same method without attribution 10 years later.
///     This is a very fast algorithm of linear time complexity, it is parallelized, and produces
///     exact Euclidean distances in any number of dimensions. `method` must be `"separable"`, which is the default,
///     or `"square"`, in which case squared distances are returned.
///
///  2. A vector distance transform, which propagates vectors to the nearest background pixel instead of propagating
///     distances as the chamfer method does. This leads to a fairly fast algorithm that can yield exact results.
///     This algorithm was described by Mullikin, based on previous work by Danielsson and Ye.
///
///     `method` must be one of `"fast"`, `"ties"` or `"true"`. The difference is in how vectors of equal length are
///     propagated. The `"fast"` method chooses one of them, which can lead to errors of around 0.2 pixels. The
///     `"ties"` method stores all vectors of equal length, which reduces the average errors but can still lead to
///     significant errors. The `"true"` method stores also vectors close in length, which leads to exact distances
///     in all cases. These three methods are listed in increasing computational complexity.
///
///     This method works with 2D and 3D images only. The `"fast"` method can have a time advantage over the
///     `"separable"` method in situations where parallelism is not an option. When exact distances are needed,
///     the separable algorithm is always fastest.
///
///     Individual vector components of the Euclidean distance transform can be obtained with
///     `dip::VectorDistanceTransform`.
///
///  3. A brute force algorithm that scales quadratically with the number of pixels. The results are always exact.
///     Use only with small images to determine a ground-truth result. For 2D and 3D inputs only.
///     `method` must be `"brute force"`.
///
/// \bug The `"true"` transform type is prone to produce an internal buffer overflow when applied to larger, almost
/// spherical objects. It this case, use a different method.
///
/// \attention The option `border` = `"background"` is not supported for the `"brute force"` method.
///
/// \literature
/// <li>R. van den Boomgaard, "Mathematical Morphology--Extensions towards Computer Vision",
///     PhD Thesis, University of Amsterdam, 1992.
/// <li>A. Meijster, J.B.T.M. Roerdink and W.H. Hesselink, "A General Algorithm for Computing Distance Transforms in Linear Time",
///     Mathematical Morphology and its Applications to %Image and Signal Processing, Springer, 2002.
/// <li>P.F. Felzenszwalb and D.P. Huttenlocher, "Distance Transforms of Sampled Functions", Theory of Computing 8:415–428, 2012.
/// <li>P.E. Danielsson, "Euclidean distance mapping", Computer Graphics and %Image Processing 14:227-248, 1980.
/// <li>Q.Z. Ye, "The signed Euclidean distance transform and its applications",
///     in: 9<sup>th</sup> International Conference on Pattern Recognition, 495-499, 1988.
/// <li>J.C. Mullikin, "The vector distance transform in two and three dimensions",
///     CVGIP: Graphical Models and %Image Processing 54(6):526-535, 1992.
/// \endliterature
DIP_EXPORT void EuclideanDistanceTransform(
      Image const& in,
      Image& out,
      String const& border = S::BACKGROUND,
      String const& method = S::SEPARABLE
);
inline Image EuclideanDistanceTransform(
      Image const& in,
      String const& border = S::BACKGROUND,
      String const& method = S::SEPARABLE
) {
   Image out;
   EuclideanDistanceTransform( in, out, border, method );
   return out;
}

/// \brief Euclidean vector distance transform
///
/// This function produces the vector components of the Euclidean distance transform, in the form of a vector image.
/// The norm of `out` is identical to the result of `dip::EuclideanDistanceTransform`.
///
/// See `dip::EuclideanDistanceTransform` for detailed information about the parameters. Valid `method` strings are
/// `"fast"`, `"ties"`, `"true"` and `"brute force"`. That is, `"separable"` or `"square"` are not allowed.
///
/// `in` should not have any dimension larger than 1e7 pixels, otherwise the vector components will underflow.
DIP_EXPORT void VectorDistanceTransform(
      Image const& in,
      Image& out,
      String const& border = S::BACKGROUND,
      String const& method = S::FAST
);
inline Image VectorDistanceTransform(
      Image const& in,
      String const& border = S::BACKGROUND,
      String const& method = S::FAST
) {
   Image out;
   VectorDistanceTransform( in, out, border, method );
   return out;
}

/// \brief Grey-weighted distance transform
///
/// `%GreyWeightedDistanceTransform` determines the grey weighted distance transform of the object elements in
/// the `bin` image, using the sample values in `grey` as the local weights for the distances. That is, it computes
/// the integral of `grey` along a path from each pixel that is set in `bin` (foreground) to any pixel that is not set
/// in `bin` (background), with the path chosen such that this integral is minimal.
///
/// The images `bin` and `grey` must have the same sizes. `bin` is a binary image, `grey` is real-valued, and both
/// must be scalar. `out` will have type `dip::DT_SFLOAT`. If `grey` is not forged, it is assumed to be valued 1
/// everywhere.
///
/// `mask` is an optional input that further constrains the paths taken by the distance transform. Paths only go
/// through those pixels that are set in `mask`. Pixels not set in mask will have either a value of 0 or infinity
/// in the output, depending on whether `bin` was set or not. If `mask` is not forged, paths are not constrained.
/// If `mask` is forged, it must be of the same sizes as `bin` and `grey`, and be binary and scalar.
///
/// This function uses one of two algorithms: the fast marching algorithm (Sethian, 1996), or a simpler propagation
/// algorithm that uses a chamfer metric (after work by Verwer and Strasters). `metric` is used only in the latter
/// case. `mode` selects the algorithm used and what output is produced:
///  - `"fast marching"` uses the fast marching algorithm. This is the default.
///  - `"chamfer"` uses the chamfer metric algorithm.
///  - `"length"` also uses the chamfer metric algorithm, but outputs the length of the optimal path, rather
///    than the integral along the path.
///
/// The chamfer metric is defined by the parameter `metric`. Any metric can be given, but a 3x3 or 5x5 chamfer metric
/// is recommended for unbiased distances. See `dip::Metric` for more information. If the `metric` doesn't have a
/// pixel size set, but either `grey` or `bin` have a pixel size defined, then that pixel size will be added to the
/// metric (the pixel size in `grey` will have precedence over the one in `bin` if they both have one defined). To
/// avoid the use of any pixel size, define `metric` with a pixel size of 1. The magnitudes of the pixel sizes are
/// used, ignoring any units.
///
/// With the fast marching algorithm, the pixel size in either `grey` or `bin` will be used to weight distances. The
/// magnitudes of the pixel sizes are used, ignoring any units.
///
/// The fast marching algorithm approximates Euclidean distances. It yields the most isotropic result, though it
/// is biased. The chamfer metric algorithm uses the metric as specified by `metric`, which could be, for example,
/// `dip::Metric("city")`. The metrics `dip::Metric("chamfer", 3)` or `dip::Metric("chamfer", 5)` are to be preferred,
/// as they produce unbiased distances (with octagonal and dodecagonal unit circles, respectively).
/// The larger neighborhood produces more precise distances than the smaller neighborhood.
///
/// The chamfer metric algorithm is a little faster than the fast marching algorithm,
/// with smaller neighborhoods being faster than larger neighborhoods.
///
/// \literature
/// <li>J.A. Sethian, "A fast marching level set method for monotonically advancing fronts", Proceedings of the
///     National Academy of Sciences 93(4):1591-1595, 1996.
/// <li>B.J.H. Verwer, P.W. Verbeek and S.T. Dekker, "An efficient uniform cost algorithm applied to distance
///     transforms", IEEE Transactions on Pattern Analysis and Machine Intelligence 11(4):425-429, 1989.
/// <li>P.W. Verbeek and B.J.H. Verwer, "Shading from shape, the eikonal equation solved by grey-weighted distance
///     transform", Pattern Recognition Letters 11(10):681-690, 1990.
/// <li>B.J.H. Verwer, "Distance Transforms, Metrics, Algorithms, and Applications", Ph.D. thesis, Delft University
///     of Technology, The Netherlands, 1991.
/// <li>K.C. Strasters, A.W.M. Smeulders and H.T.M. van der Voort, "3-D Texture characterized by accessibility
///     measurements, based on the grey weighted distance transform", BioImaging 2(1):1-21, 1994.
/// <li>K.C. Strasters, "Quantitative Analysis in Confocal %Image Cytometry", Ph.D. thesis, Delft University of
///     Technology, The Netherlands, 1994.
/// \endliterature
DIP_EXPORT void GreyWeightedDistanceTransform(
      Image const& grey,
      Image const& bin,
      Image const& mask,
      Image&  out,
      Metric metric = { S::CHAMFER, 2 },
      String const& mode = S::FASTMARCHING
);
inline Image GreyWeightedDistanceTransform(
      Image const& grey,
      Image const& bin,
      Image const& mask = {},
      Metric const& metric = { S::CHAMFER, 2 },
      String const& mode = S::FASTMARCHING
) {
   Image out;
   GreyWeightedDistanceTransform( grey, bin, mask, out, metric, mode );
   return out;
}

/// \brief Geodesic distance transform
///
/// This function computes the geodesic distance transform of the object elements in the `marker` image, with paths
/// constrained to the `condition` image. That is, for each set pixel in `marker`, the distance to the background in
/// `marker` is computed, along a path that stays completely within set pixels of `condition`. Pixels where `marker`
/// or `condition` are not set, the output will not be set. Specifically, if a pixel is set in `marker` but not in
/// `condition`, then that pixel will have a value of 0 in the output, but this value will not be used as a seed
/// for paths, so that its neighbors can have a large distance value.
///
/// Non-isotropic pixel sizes are supported. The pixel sizes of `marker` are used, those of `condition` are ignored.
///
/// The images `marker` and `condition` must have the same sizes, and be scalar and binary.
/// `out` will have type `dip::DT_SFLOAT`.
///
/// This function is currently implemented in terms of `dip::GreyWeightedDistanceTransform`, see that function for
/// literature and implementation details. It uses the fast marching algorithm to produce a reasonable approximation
/// of Euclidean distances.
inline void GeodesicDistanceTransform(
      Image const& marker,
      Image const& condition,
      Image& out
) {
   GreyWeightedDistanceTransform( {}, marker, condition, out, {}, S::FASTMARCHING );
}
inline Image GeodesicDistanceTransform(
      Image const& marker,
      Image const& condition
) {
   Image out;
   GeodesicDistanceTransform( marker, condition, out );
   return out;
}

/// \}

} // namespace dip

#endif // DIP_DISTANCE_H
