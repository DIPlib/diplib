/*
 * DIPlib 3.0
 * This file contains declarations for distance transforms
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
/// This function computes the Euclidean distance transform of a 2D or 3D input binary image using the vector-based
/// method as opposed to the chamfer method. This method computes distances from the objects (binary 1's) to
/// the nearest background (binary 0's) of `in` and stored the result in `out`. `out` is of type `dip::DT_SFLOAT`.
///
/// Computed distances use the pixel sizes (ignoring any units). To compute distances in pixels, reset the pixel
/// size (dip::Image::ResetPixelSize). Note that, when pixels sizes are correctly set, this function handles
/// anisotropic sampling densities correctly.
///
/// The `border` parameter specifies whether the edge of the image should be treated as objects (`"object"`) or
/// as background (`"background"`).
///
/// The `method` parameter specifies the method to use to compute the distances:
///  - `"fast"`: fastest, but most errors.
///  - `"ties"`: slower, but fewer errors.
///  - `"true"`: slow, uses lots of memory, but is "error free".
///  - `"brute force"`: gives a result from which errors are calculated for the other methods. This method is
///                     extremely slow and should only be used for testing purposes.
///
/// Individual vector components of the Euclidean distance transform can be obtained with `dip::VectorDistanceTransform`.
///
/// **Literature**
///  - P.E. Danielsson, "Euclidean distance mapping", Computer Graphics and Image Processing 14:227-248, 1980.
///  - J.C. Mullikin, "The vector distance transform in two and three dimensions", CVGIP: Graphical Models and Image Processing 54(6):526-535, 1992.
///  - I. Ragnemalm, "Generation of Euclidean Distance Maps", Licentiate thesis, No. 206, Link&ouml;ping University, Sweden, 1990.
///  - Q.Z. Ye, "The signed Euclidean distance transform and its applications", in: 9th International Conference on Pattern Recognition, 495-499, 1988.
///
/// **Known bugs**
///  - The `"true"` transform type is prone to produce an internal buffer overflow when applied to larger (almost)
///    spherical objects. It this case, use a different method.
///  - The option `border` = `"background"` is not supported for the `"brute force"` method.
DIP_EXPORT void EuclideanDistanceTransform(
      Image const& in,
      Image& out,
      String const& border = "background",
      String const& method = "fast"
);
inline Image EuclideanDistanceTransform(
      Image const& in,
      String const& border = "background",
      String const& method = "fast"
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
/// See `dip::EuclideanDistanceTransform` for detailed information about the parameters. `in` should not have any
/// dimension larger than 1e7 pixels, otherwise the vector components will underflow.
DIP_EXPORT void VectorDistanceTransform(
      Image const& in,
      Image& out,
      String const& border = "background",
      String const& method = "fast"
);
inline Image VectorDistanceTransform(
      Image const& in,
      String const& border = "background",
      String const& method = "fast"
) {
   Image out;
   VectorDistanceTransform( in, out, border, method );
   return out;
}

/// \brief Grey-weighted distance transform
///
/// `%GreyWeightedDistanceTransform` determines the grey weighted distance transform of the object elements in
/// the `in` image, using the sample values in `grey` as the local weights for the distances. That is, it computes
/// the integral of `grey` along a path from each pixel that is set in `in` (foreground) to any pixel that is not set
/// in `in` (background), with the path chosen such that this integral is minimal.
///
/// The implemented algorithm uses a heap sort for sorting the pixels to be processed.
///
/// The images `in` and `grey` must have the same sizes. `in` is a binary image, `grey` is real-valued, and both
/// must be scalar. `out` will have type `dip::DT_SFLOAT`.
///
/// Those elements that are neighboring an object element in the output image are considered seeds for the distance
/// propagation. Before any seeds are detected the borders of the `out` image are set to 0. The size of the border
/// is determined by the chamfer metric size. In case of a 3 by 3 chamfer metric, the image border is one element;
/// in case of a 5 by 5 chamfer it is 2 elements. Elements in the border are not considered seeds. If no valid seeds
/// are found the routine will throw an exception.
///
/// The chamfer metric is defined by the parameter `metric`. Any metric can be given, but a 3x3 or 5x5 chamfer metric
/// is recommended for unbiased distances. See `dip::Metric` for more information.
///
/// If `outputMode` is `"GDT"` (the default), then `out` will contain the grey-weighted distance transform. If it is
/// `"Euclidean"`, in will output the Euclidean (geometric) length of the optimal path instead. If it is `"both"`,
/// it will output a tensor image with two components, the first one will be the GDT, and the second one the
/// path length.
///
/// **Literature**
///  - B.J.H. Verwer, P.W. Verbeek and S.T. Dekker, "An efficient uniform cost algorithm applied to distance
///    transforms", IEEE Transactions on Pattern Analysis and Machine Intelligence 11(4):425-429, 1989.
///  - P.W. Verbeek and B.J.H. Verwer, "Shading from shape, the eikonal equation solved by grey-weighted distance
///    transform", Pattern Recognition Letters 11(10):681-690, 1990.
///  - B.J.H. Verwer, "Distance Transforms, Metrics, Algorithms, and Applications", Ph.D. thesis, Delft University
///    of Technology, The Netherlands, 1991.
///  - K.C. Strasters, A.W.M. Smeulders and H.T.M. van der Voort, "3-D Texture characterized by accessibility
///    measurements, based on the grey weighted distance transform", BioImaging 2(1):1-21, 1994.
///  - K.C. Strasters, "Quantitative Analysis in Confocal Image Cytometry", Ph.D. thesis, Delft University of
///    Technology, The Netherlands, 1994.
///
/// **Known issue**
///  - The function produces incomplete results in a 2-pixel border around the edge (or more for larger metric
///    neighborhoods). If this is an issue, consider adding 2 pixels on each side of your image. Make sure that
///    `in` has high grey values in the border to avoid unexpected output.
///
/// The function `dip::GrowRegionsWeighted` produces a grey-weighted distance transform without this or other
/// limitations of `%dip::GreyWeightedDistanceTransform`.
DIP_EXPORT void GreyWeightedDistanceTransform(
      Image const& grey,
      Image const& in,
      Image&  out,
      Metric const& metric = { "chamfer", 2 },
      String const& outputMode = "GDT" // or: "Euclidean" or "both"
);
inline Image GreyWeightedDistanceTransform(
      Image const& grey,
      Image const& in,
      Metric const& metric = { "chamfer", 2 },
      String const& outputMode = "GDT" // or: "Euclidean" or "both"
) {
   Image out;
   GreyWeightedDistanceTransform( grey, in, out, metric, outputMode );
   return out;
}

// TODO: functions to port:
/*
   dip_FastMarching_PlaneWave (dip_distance.h) (this function needs some input image checking!)
   dip_FastMarching_SphericalWave (dip_distance.h) (this function needs some input image checking!)
*/

/// \}

} // namespace dip

#endif // DIP_DISTANCE_H
