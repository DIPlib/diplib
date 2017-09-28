# The features known by the measurement tool {#features}

[//]: # (DIPlib 3.0)

[//]: # ([c]2017, Cris Luengo.)
[//]: # (Based on original DIPlib code: [c]1995-2014, Delft University of Technology.)

[//]: # (Licensed under the Apache License, Version 2.0 [the "License"];)
[//]: # (you may not use this file except in compliance with the License.)
[//]: # (You may obtain a copy of the License at)
[//]: # ()
[//]: # (   http://www.apache.org/licenses/LICENSE-2.0)
[//]: # ()
[//]: # (Unless required by applicable law or agreed to in writing, software)
[//]: # (distributed under the License is distributed on an "AS IS" BASIS,)
[//]: # (WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.)
[//]: # (See the License for the specific language governing permissions and)
[//]: # (limitations under the License.)

This page describes all the features known to `dip::MeasurementTool` by default. They
are sorted into sections in the same way as the table in the documentation to
`dip::MeasurementTool`

\tableofcontents

[//]: # (--------------------------------------------------------------)

\section size_features Size features

\subsection size_features_Size Size
Computes the number of object pixels. If the image has a known pixel size, the number
of pixels is multiplied by the area of each pixel to give an unbiased estimate of the
object's area (2D), volume (3D), or hyper-volume (nD).

\subsection size_features_CartesianBox CartesianBox
The sizes of the smallest box around the object that is aligned with the grid axes.
There is one value per image dimension. The size is in pixels or in physical units if known.

\subsection size_features_Minimum Minimum
The smallest index along each dimension to a pixel within the object. This
corresponds to the top-left corner of the cartesian bounding box. There is one value per
image dimension. If the image has a known pixel size, the values represent the distances
to the lower image edge along each dimension.

\subsection size_features_Maximum Maximum
The largest index along each dimension to a pixel within the object. This
corresponds to the bottom-right corner of the cartesian bounding box. There is one value per
image dimension. If the image has a known pixel size, the values represent the distances
to the lower image edge along each dimension.

\subsection size_features_Perimeter Perimeter
Computes the length of the object perimeter using the object's chain code, using
`dip::ChainCode::Length`.

Note that the chain code measures work only for 2D images, and expect objects to be a single
connected component. If multiple connected components have the same label, only the first
connected component found for that label will be measured.
Any holes in the object are not included in the perimeter either.

\subsection size_features_SurfaceArea SurfaceArea
The 3D equivalent to the "Perimeter" feature. It does not assume a single connected component,
and will include all surfaces in the measurement, including those of holes.

**Literature**
 - J.C. Mullikin and P.W. Verbeek, "Surface area estimation of digitized planes," Bioimaging 1(1):6-16, 1993.

\subsection size_features_Feret Feret
Computes the maximum and minimum object diameters from the object's convex hull, using
`dip::ConvexHull::Feret`. The convex hull is computed from the chain code.

Note that the chain code measures work only for 2D images, and expect objects to be a single
connected component. If multiple connected components have the same label, only the first
connected component found for that label will be measured.

Five values are returned:
 - "FeretMax" is the maximum Feret diameter, the length of the object.
 - "FeretMin" in the minimum Feret diameter, the width of the object.
 - "FeretPerpMin", the Feret diameter perpendicular to "FeretMin", which is not necessarily
   equal to "FeretMax". "FeretMin" and "FeretPerpMin" are (usually, but not necessarily)
   the sizes of the minimum bounding box.
 - "FeretMaxAng" is the angle at which "FeretMax" was obtained.
 - "FeretMinAng" is the angle at which "FeretMin" was obtained.

\subsection size_features_ConvexArea ConvexArea
The area of the convex hull of the object. The convex hull is computed from the chain code.

Note that the chain code measures work only for 2D images, and expect objects to be a single
connected component. If multiple connected components have the same label, only the first
connected component found for that label will be measured.

\subsection size_features_ConvexPerimeter ConvexPerimeter
The perimeter of the convex hull. The convex hull is computed from the chain code.

Note that the chain code measures work only for 2D images, and expect objects to be a single
connected component. If multiple connected components have the same label, only the first
connected component found for that label will be measured.

[//]: # (--------------------------------------------------------------)

\section shape_features Shape features

\subsection shape_features_AspectRatioFeret AspectRatioFeret
The ratio `FeretPerpMin / FeretMin`, two of the values returned by the "Feret" feature.

\subsection shape_features_Radius Radius
Statistics on the radius of the object, computed from the chain code using
`dip::Polygon::RadiusStatistics`.

Note that the chain code measures work only for 2D images, and expect objects to be a single
connected component. If multiple connected components have the same label, only the first
connected component found for that label will be measured.

This feature takes the distances from the object's centroid (as determined from the chain
code, not influenced by any holes in the object) to each of the border pixels. From these
distances it computes four values:
 - "RadiusMax", the largest distance.
 - "RadiusMean", the average distance.
 - "RadiusMin", the shortest distance.
 - "RadiusStD", the standard deviation.

Note that the centroid does not necessarily lie within the object.

\subsection shape_features_EllipseVariance EllipseVariance
A measure for deviation from an elliptic shape, computed using `dip::Polygon::EllipseVariance`
from the chain code.

Note that the chain code measures work only for 2D images, and expect objects to be a single
connected component. If multiple connected components have the same label, only the first
connected component found for that label will be measured.

\subsection shape_features_P2A P2A
Computes \f$p^2/(4 \pi a)\f$, where *p* is the perimeter and *a* is the area, for
2D objects, and \f$\sqrt{a^3}/(6v\sqrt{\pi})\f$, where *a* is the surface area and
*v* is the volume, for 3D objects. See "Perimeter", "SurfaceArea" and "Size".

\subsection shape_features_PodczeckShapes PodczeckShapes
Computes the 5 Podczeck shape descriptors using the results of features "Size", "Feret"
and "Perimeter". The shape descriptors are:
 - "Square": similarity to a square: \f$a/(w h)\f$.
 - "Circle": similarity to a circle: \f$4 a/(\pi h^2)\f$.
 - "Triangle": similarity to a triangle: \f$2 a/(w h)\f$.
 - "Ellipse": similarity to an ellipse: \f$4 a/(\pi w h)\f$.
 - "Elongation": object elongation: \f$p/l\f$.

where *a* is the object area, *p* the perimeter, *l* the largest Feret diameter, *w* the
smallest Feret diameter, and *h* the diameter perpendicular to the smallest diameter
("FeretPerpMin" value).

\subsection shape_features_Convexity Convexity
The ratio `Size/ConvexArea` of the features "Size" and "ConvexArea".

\subsection shape_features_BendingEnergy BendingEnergy
Bending energy of object perimeter, computed using `dip::ChainCode::BendingEnergy` from the
object's chain code.

Note that the chain code measures work only for 2D images, and expect objects to be a single
connected component. If multiple connected components have the same label, only the first
connected component found for that label will be measured.

[//]: # (--------------------------------------------------------------)

\section intensity_features Intensity features

\subsection intensity_features_Mass Mass
The sum of the grey-value image intensities across the object.

\subsection intensity_features_Mean Mean
The mean of the grey-value image intensities across the object.

\subsection intensity_features_StandardDeviation StandardDeviation
The standard deviation of the grey-value image intensities across the object.

\subsection intensity_features_Statistics Statistics
The mean, standard deviation, skewness and excess kurtosis of the grey-value image intensities
across the object. This feature has 4 values.

\subsection intensity_features_MaxVal MaxVal
The maximum grey-value image intensity within the object.

\subsection intensity_features_MinVal MinVal
The minimum grey-value image intensity within the object.

[//]: # (--------------------------------------------------------------)

\section binary_moments Moments of binary object

\subsection binary_moments_Center Center
Coordinates of the geometric mean (centroid) of the object (with sub-pixel precision),
which is the first order normalized moment of the binary shape.
There is one value per image dimension. If the image has a known pixel size, the values
represent the distances to the lower image edge along each dimension.

\subsection binary_moments_Mu Mu
Elements of the inertia tensor of the object, which is composed of second order
normalized central moments of the binary shape. For an image with *n* dimensions,
there are \f$n(n+1)/2\f$ values. These are stored in the same order as symmetric
tensors are stored in an image (see `dip::Tensor::Shape`).

For more information, see `dip::MomentAccumulator::SecondOrder`.

\subsection binary_moments_Inertia Inertia
Moments of inertia of the binary object, the eigenvalues of the tensor computed by
feature "Mu". There is one value per image dimension. The eigenvectors are sorted
largest to smallest.

\subsection binary_moments_MajorAxes MajorAxes
Principal axes of the binary object, the eigenvectors of the tensor computed by
feature "Mu". For an image with *n* dimensions, there are \f$n^2\f$ values. The first
*n* values are the eigenvector associated to the largest eigenvalue, etc.

\subsection binary_moments_DimensionsCube DimensionsCube
Lengths of the sides of a rectangle (2D) or box (3D) with the same moments of inertia
as the binary object. Derived from feature "Inertia".

Currently defined only for 2D and 3D images.

\subsection binary_moments_DimensionsEllipsoid DimensionsEllipsoid
Diameters of an ellipse (2D) or ellipsoid (3D) with the same moments of inertia as the
binary object. Derived from feature "Inertia".

Currently defined only for 2D and 3D images.

[//]: # (--------------------------------------------------------------)

\section grey_moments Moments of grey-value object

\subsection grey_moments_Gravity Gravity
Coordinates of the center of mass of the object, which is the first order normalized
moment of the binary shape weighted by the grey-value image's intensities.
There is one value per image dimension. If the image has a known pixel size, the values
represent the distances of the center of mass to the lower image edge along each dimension.

Identical to feature "Center" but using the grey-value image as weighting.

\subsection grey_moments_GreyMu GreyMu
Elements of the inertia tensor of the grey-weighted object, which is composed of second order
normalized central moments of the binary shape weighted by the grey-value image's intensities.
For an image with *n* dimensions, there are \f$n(n+1)/2\f$ values. These are stored in the same
order as symmetric tensors are stored in an image (see `dip::Tensor::Shape`).

For more information, see `dip::MomentAccumulator::SecondOrder`.

Identical to feature "Mu" but using the grey-value image as weighting.

\subsection grey_moments_GreyInertia GreyInertia
Moments of inertia of the grey-weighted object, the eigenvalues of the tensor computed by
feature "GreyMu". There is one value per image dimension. The eigenvectors are sorted
largest to smallest.

Identical to feature "Inertia" but using the grey-value image as weighting.

\subsection grey_moments_GreyMajorAxes GreyMajorAxes
Principal axes of the grey-weighted object, the eigenvectors of the tensor computed by
feature "GreyMu". For an image with *n* dimensions, there are \f$n^2\f$ values. The first
*n* values are the eigenvector associated to the largest eigenvalue, etc.

Identical to feature "MajorAxes" but using the grey-value image as weighting.

\subsection grey_moments_GreyDimensionsCube GreyDimensionsCube
Lengths of the sides of a rectangle (2D) or box (3D) with the same moments of inertia
as the grey-weighted object. Derived from feature "GreyInertia".

Currently defined only for 2D and 3D images.

Identical to feature "DimensionsCube" but using the grey-value image as weighting.

\subsection grey_moments_GreyDimensionsEllipsoid GreyDimensionsEllipsoid
Diameters of an ellipse (2D) or ellipsoid (3D) with the same moments of inertia as the
grey-weighted object. Derived from feature "GreyInertia".

Currently defined only for 2D and 3D images.

Identical to feature "DimensionsEllipsoid" but using the grey-value image as weighting.
