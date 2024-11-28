/*
 * (c)2016-2024, Cris Luengo.
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

#ifndef DIP_REGIONS_H
#define DIP_REGIONS_H

#include <utility>
#include <vector>

#include "diplib.h"
#include "diplib/graph.h"
#include "diplib/measurement.h"  // Sadly, we need to include this full header just for the `Measurement::IteratorFeature` reference in one function here. We cannot forward-declare a nested class.
#include "diplib/neighborlist.h"


/// \file
/// \brief Functions to label connected components and process labeled images.
/// See \ref regions.


namespace dip {


/// \group regions Labeled regions
/// \brief Label connected components and process labeled images.
///
/// Labeled images are of any unsigned integer type.
/// \addtogroup

/// \brief Labels the connected components in a binary image
///
/// The output is an unsigned integer image. Each object (respecting the connectivity,
/// see \ref connectivity) in the input image receives a unique number. This number ranges
/// from 1 to the number of objects in the image. The pixels in the output image corresponding
/// to a given object are set to this number (label). The remaining pixels in the output image
/// are set to 0.
///
/// The `minSize` and `maxSize` set limits on the size of the objects: Objects smaller than `minSize`
/// or larger than `maxSize` do not receive a label and the corresponding pixels in the output
/// image are set to zero. Setting either to zero disables the corresponding check. Setting both
/// to zero causes all objects to be labeled, irrespective of size.
///
/// The `boundaryCondition` array contains a boundary condition string per image dimension, or one
/// string to be used for all dimensions. Valid strings are:
///
/// - `""` and `"mirror"`: the default behavior, causing the labeling to simply stop at the edges.
/// - `"periodic"`: imposing a periodic boundary condition, such that objects touching opposite
///   edges of the image are considered the same object.
/// - `"remove"`: causing objects that touch the image edge to be removed.
///
/// `boundaryCondition` can also be an empty array, using the default behavior for all dimensions.
///
/// `mode` can be `"all"` (default) or `"largest"`. If set to `"largest"`, only the largest object
/// is retained, and will have a label of 1.
///
/// Returns the number of connected components found. The returned value is thus the maximum
/// value in the output image.
DIP_EXPORT dip::uint Label(
      Image const& binary,
      Image& out,
      dip::uint connectivity = 0,
      dip::uint minSize = 0,
      dip::uint maxSize = 0,
      StringArray boundaryCondition = {},
      String const& mode = S::ALL
);
DIP_NODISCARD inline Image Label(
      Image const& binary,
      dip::uint connectivity = 0,
      dip::uint minSize = 0,
      dip::uint maxSize = 0,
      StringArray boundaryCondition = {},
      String const& mode = S::ALL
) {
   Image out;
   Label( binary, out, connectivity, minSize, maxSize, std::move( boundaryCondition ), mode );
   return out;
}

/// \brief Gets a list of object labels in the labeled image. A labeled image must be of an unsigned type.
///
/// If `background` is `"include"`, the label ID 0 will be included in the result if present in the image.
/// Otherwise, `background` is `"exclude"`, and the label ID 0 will be ignored.
///
/// If `region` is `"edges"`, only the labels of objects touching the image edges will be listed. By default,
/// the labels of all objects are listed.
DIP_NODISCARD DIP_EXPORT std::vector< LabelType > ListObjectLabels(
      Image const& label,
      Image const& mask = {},
      String const& background = S::EXCLUDE,
      String const& region = ""
);
DIP_NODISCARD inline std::vector< LabelType > ListObjectLabels(
      Image::View const& label,
      String const& background = S::EXCLUDE,
      String const& region = ""
) {
   if( label.Offsets().empty() ) {
      // This code works if either the view is regular or has a mask.
      return ListObjectLabels( label.Reference(), label.Mask(), background, region );
   }
   // When the view uses indices, we copy the data over to a new image, it's not worth while writing separate code for this case.
   return ListObjectLabels( Image( label ), {}, background, region );
}

[[ deprecated( "Use dip::ListObjectLabels instead." ) ]]
DIP_NODISCARD inline UnsignedArray GetObjectLabels(
      Image const& label,
      Image const& mask = {},
      String const& background = S::EXCLUDE
) {
   auto labels = ListObjectLabels( label, mask, background );
   UnsignedArray out( labels.size() );
   std::copy( labels.begin(), labels.end(), out.begin() );
   return out;
}
[[ deprecated( "Use dip::ListObjectLabels instead." ) ]]
DIP_NODISCARD inline UnsignedArray GetObjectLabels(
      Image::View const& label,
      String const& background = S::EXCLUDE
) {
#if defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // GCC warns that we're using a deprecated function here. Duh!
#endif
   if( label.Offsets().empty() ) {
      // This code works if either the view is regular or has a mask.
      return GetObjectLabels( label.Reference(), label.Mask(), background );
   }
   // When the view uses indices, we copy the data over to a new image, it's not worth while writing separate code for this case.
   return GetObjectLabels( Image( label ), {}, background );
#if defined(__GNUG__)
#pragma GCC diagnostic pop
#endif
}

/// \brief Re-assigns labels to objects in a labeled image, such that all labels are consecutive.
///
/// Note that disjoint objects will remain disjoint, as this function only replaces each label ID with
/// a new value. The output image will have consecutive label IDs, in the range [1, *N*], with *N* the
/// number of unique labels in `label`. Pixels with a value of 0 will remain 0 (background).
///
/// \ref dip::GetObjectLabels returns a list of unique labels in `label`, and can be used to determine *N*.
DIP_EXPORT void Relabel( Image const& label, Image& out );
DIP_NODISCARD inline Image Relabel( Image const& label ) {
   Image out;
   Relabel( label, out );
   return out;
}

/// \brief Re-assigns labels to objects in a labeled image, such that regions joined by an edge in `graph` obtain the same label.
///
/// `graph` should be obtained through \ref dip::RegionAdjacencyGraph and modified to obtain a useful segmentation.
/// For example:
///
/// ```cpp
/// dip::Image input = ...
/// dip::Image label = dip::Watershed( dip::GradientMagnitude( input, { 2 } ), {}, 2, 1, 0, { "labels" } );
/// dip::MeasurementTool measurementTool;
/// auto msr = measurementTool.Measure( label, input, { "Mean" } );
/// dip::Graph graph = dip::RegionAdjacencyGraph( label, msr[ "Mean" ], "watershed" );
/// graph = dip::MinimumSpanningForest( graph, { 1 } ); // make sure we don't use the unconnected vertex 0 as root.
/// graph.RemoveLargestEdges( 100 );
/// dip::Relabel( label, label, graph );
/// ```
DIP_EXPORT void Relabel( Image const& label, Image& out, Graph const& graph );
DIP_NODISCARD inline Image Relabel( Image const& label, Graph const& graph ) {
   Image out;
   Relabel( label, out, graph );
   return out;
}

/// \brief Re-assigns labels to objects in a labeled image, such that regions joined by an edge in `graph` obtain the same label.
///
/// \ref dip::DirectedGraph version of the function above.
DIP_EXPORT void Relabel( Image const& label, Image& out, DirectedGraph const& graph );
DIP_NODISCARD inline Image Relabel( Image const& label, DirectedGraph const& graph ) {
   Image out;
   Relabel( label, out, graph );
   return out;
}

/// \brief Removes small objects from a labeled or binary image.
///
/// If `in` is an unsigned integer image, it is assumed to be a labeled image. The size of the objects
/// are measured using \ref dip::MeasurementTool, and the labels for the objects with fewer than `threshold`
/// pixels are removed. The `connectivity` parameter is ignored. Note that if this image contains disjoint
/// objects (i.e. multiple connected components with the same label), it is the size of the object as a
/// whole that counts, not the size of individual connected components.
///
/// If `in` is a binary image, \ref dip::Label is called with `minSize` set to `threshold`, and the result
/// is binarized again. `connectivity` is passed to the labeling function.
///
/// The operation on a binary image is equivalent to an area opening with parameter `threshold`
/// (see \ref dip::AreaOpening). The same is not true for the labeled image case, if labeled regions
/// are touching or if objects are disjoint.
DIP_EXPORT void SmallObjectsRemove(
      Image const& in,
      Image& out,
      dip::uint threshold,
      dip::uint connectivity = 0
);
DIP_NODISCARD inline Image SmallObjectsRemove(
      Image const& in,
      dip::uint threshold,
      dip::uint connectivity = 0
) {
   Image out;
   SmallObjectsRemove( in, out, threshold, connectivity );
   return out;
}

/// \brief  Grow (dilate) labeled regions uniformly.
///
/// The regions in the labeled image `label` are dilated `iterations` steps, according to `connectivity`,
/// and optionally constrained by `mask`. If `iterations` is 0, the objects are dilated until no further
/// change is possible.
///
/// If a `mask` is given, this is the labeled equivalent to \ref dip::BinaryPropagation, otherwise it works as
/// \ref dip::BinaryDilation on each label. The difference between `dip::GrowRegions` and \ref dip::Dilation
/// (which can also be applied to a labeled image) is that here growing stops when different labels meet,
/// whereas in a normal dilation, the label with the larger value would grow over the one with the smaller value.
///
/// The `connectivity` parameter defines the metric, that is, the shape of the structuring element
/// (see \ref connectivity). Alternating connectivity is only implemented for 2D and 3D images.
///
/// If isotropy in the dilation is very important, or if the image has anisotropic sampling density, use
/// \ref GrowRegionsWeighted instead.
///
/// \see dip::GrowRegionsWeighted
DIP_EXPORT void GrowRegions(
      Image const& label,
      Image const& mask,
      Image& out,
      dip::sint connectivity = -1,
      dip::uint iterations = 0
);
DIP_NODISCARD inline Image GrowRegions(
      Image const& label,
      Image const& mask = {},
      dip::sint connectivity = -1,
      dip::uint iterations = 0
) {
   Image out;
   GrowRegions( label, mask, out, connectivity, iterations );
   return out;
}

/// \brief Grow labeled regions with a speed function given by a grey-value image.
///
/// The regions in the input image `label` are grown according to a grey-weighted distance
/// metric; the weights are given by `grey`. The optional mask image `mask` limits the
/// growing. All three images must be scalar. `label` must be of an unsigned integer type,
/// `grey` must be real-valued, and `mask` must be binary.
///
/// If `grey` is not forged, uniform weights are assumed. If both `grey` and `mask` are
/// not forged, regions will grow according to the Euclidean distances, see below.
///
/// `out` is of the type \ref dip::DT_LABEL, and contains the grown regions.
///
/// This function works by first computing distances from the labeled pixels using
/// \ref dip::GreyWeightedDistanceTransform, or using \ref dip::EuclideanDistanceTransform
/// if both `grey` and `mask` are not forged. These distances are limited by `distance`.
/// Then it applies \ref dip::SeededWatershed.
///
/// This function will correctly handle anisotropic sampling densities. Pixel sizes are taken from `grey`,
/// and if it doesn't have pixel sizes, they are taken from `label`. To ignore the sampling density and grow
/// isotropically, reset the pixel size (\ref dip::Image::ResetPixelSize) of both images. Note that
/// these pixel sizes influence the distances computed, and so `distance` is affected by these sizes.
///
/// \see dip::GrowRegions, dip::GreyWeightedDistanceTransform, dip::EuclideanDistanceTransform, dip::SeededWatershed
DIP_EXPORT void GrowRegionsWeighted(
      Image const& label,
      Image const& grey,
      Image const& mask,
      Image& out,
      dfloat distance = infinity
);
DIP_NODISCARD inline Image GrowRegionsWeighted(
      Image const& label,
      Image const& grey,
      Image const& mask = {},
      dfloat distance = infinity
) {
   Image out;
   GrowRegionsWeighted( label, grey, mask, out, distance );
   return out;
}

[[ deprecated( "The `Metric` argument is ignored." ) ]]
inline void GrowRegionsWeighted(
      Image const& label,
      Image const& grey,
      Image const& mask,
      Image& out,
      Metric const& /*ignored*/
) {
   GrowRegionsWeighted( label, grey, mask, out );
}
[[ deprecated( "The `Metric` argument is ignored." ) ]]
DIP_NODISCARD inline Image GrowRegionsWeighted(
      Image const& label,
      Image const& grey,
      Image const& mask,
      Metric const& /*ignored*/
) {
   return GrowRegionsWeighted( label, grey, mask );
}

/// \brief Ensures a gap between regions with unequal labels.
///
/// In the output image, no two regions will be connected according to `connectivity`. `out` is of the same
/// type as `label`, which must be an unsigned integer type, and scalar.
///
/// To create a one-pixel gap between regions, regions must shrink, and they must shrink unequally. If all regions
/// were to shrink equally, we would create a two-pixel gap. This function chooses to be biased towards larger-valued
/// labels: where two objects touch, the lower-valued region shrinks.
///
/// This function works by finding pixels that have a neighbor with a larger value, and setting these pixels to
/// zero (the background label).
DIP_EXPORT void SplitRegions( Image const& label, Image& out, dip::uint connectivity = 0 );
DIP_NODISCARD inline Image SplitRegions( Image const& label, dip::uint connectivity = 0 ) {
   Image out;
   SplitRegions( label, out, connectivity );
   return out;
}

/// \brief Make each object a single, convex shape.
///
/// For each label, it finds the convex hull and draws it into the image. Where the convex hulls of objects
/// overlaps, the larger label prevails, meaning that the other object might not be convex any more where the
/// larger label overlaps it.
///
/// `mode` can be `"filled"` (the default) or `"hollow"`. With `"hollow"`, the convex hull outlines are drawn
/// into an empty image, such that overlapping convex hulls can be visually identified. With `"filled"`, the
/// operation is increasing (like a morphological closing), meaning that `out >= label` is guaranteed.
///
/// Note that defining a convex hull in a discrete space is ambiguous, and therefore this function is not
/// idempotent (i.e. running it a second time on its own output can produce a different result).
///
/// If the input image is binary, a single convex hull is computed and drawn. The output image will be binary also.
///
/// The image must have two dimensions, and be scalar.
DIP_EXPORT void MakeRegionsConvex2D( Image const& label, Image& out, String const& mode = S::FILLED );
DIP_NODISCARD inline Image MakeRegionsConvex2D( Image const& label, String const& mode = S::FILLED ) {
   Image out;
   MakeRegionsConvex2D( label, out, mode );
   return out;
}

/// \brief Returns the bounding box for all pixels with label `objectID` in the labeled or binary image `label`.
///
/// `label` must be a labeled image (of type unsigned integer) or binary, and must be scalar and have at least
/// one dimension.
///
/// In the case of a binary image, `objectID` should be 1 or 0, no other values are possible in a binary image.
///
/// When no pixels with the value `objectID` exist in the image, the output \ref dip::RangeArray will be an
/// empty array. Otherwise, it will have as many \ref dip::Range elements as dimensions are in the image.
///
/// To obtain the bounding box of all labels at once, use \ref dip::MeasurementTool::Measure with the
/// features \ref size_features_Minimum and \ref size_features_Maximum.
RangeArray DIP_EXPORT GetLabelBoundingBox( Image const& label, LabelType objectID );

/// \brief Construct a graph for the given labeled image.
///
/// Each region (object/label) in the image `labels` is a node. Edges represent neighborhood relations between regions.
/// Because the \ref dip::Graph class uses the vertex ID as an index into an array, it is recommended that this function
/// be called with a labeled image where labels are more or less consecutive. If `dip::Maximum( labels ).As< dip::uint >()`
/// (i.e. the largest label ID) is much larger than `dip::GetObjectLabels( labels ).size()` (i.e. the number of labels),
/// then the \ref dip::Graph object will have many unused vertices, and hence waste space. Use \ref dip::Relabel to modify
/// the `labels` image to have consecutive labels.
///
/// `mode` indicates how to construct the graph. It can be one of the following strings:
///
/// - `"touching"`: two regions are neighbors if they have at least one pixel that is 1-connected to the other
///   region. That is, the two regions directly touch.
/// - `"watershed"`: two regions are neighbors if there is a background pixel that is 1-connected to the two
///   regions. That is, the two regions are separated by a 1-pixel watershed line. Note that, in this case,
///   two regions that directly touch will not be recognized as neighbors!
///
/// In both modes, region with ID 0 (the background) is not included in the graph. But note that there is a vertex
/// with index 0, which will not be connected to any other vertex.
/// To include the background, simply increment the label image by 1.
///
/// Edge weights are computed as follows: The fraction of boundary pixels for region 1 that connect to
/// region 2 is determined. The fraction of boundary pixels for region 2 that connect to region 1 is determined.
/// One minus the larger of these two fractions is the edge weight. Thus, edge weights have a value in the
/// half-open interval [0,1). If one of the two regions has a very large fraction of its perimeter connected
/// to another region, then the edge weight is very small to indicate a strong connection.
///
/// Vertex values are not assigned.
///
/// After modifying the graph (removing edges to split the graph into separate islands), use
/// \ref dip::Relabel(dip::Image const&, dip::Image&, dip::Graph const&) to update the `labels` image.
DIP_EXPORT Graph RegionAdjacencyGraph( Image const& labels, String const& mode = "touching" );

/// \brief Construct a graph for the given labeled image.
///
/// This function is similar to the one above, but edge weights are derived from the absolute difference
/// between `featureValues` for the two regions joined by the edge. Vertex values are set to the feature value
/// for the region.
///
/// The input `featureValues` is a view over a specific feature in a \ref dip::Measurement object. Only the
/// first value of the feature is used. For features with multiple values, select a value using the
/// \ref dip::Measurement::IteratorFeature::Subset method, or pick a column in the \ref dip::Measurement object
/// directly using \ref dip::Measurement::FeatureValuesView.
///
/// For the labels that do not appear in `featureValues`, their vertex value will be set to 0.
DIP_EXPORT Graph RegionAdjacencyGraph(
   Image const& labels,
   Measurement::IteratorFeature const& featureValues,
   String const& mode = "touching"
);


/// \endgroup

} // namespace dip

#endif // DIP_REGIONS_H
