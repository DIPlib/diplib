\comment DIPlib 3.0

\comment (c)2016-2020, Cris Luengo.
\comment Based on original DIPlib code: (c)1995-2014, Delft University of Technology.

\comment Licensed under the Apache License, Version 2.0 [the "License"];
\comment you may not use this file except in compliance with the License.
\comment You may obtain a copy of the License at
\comment
\comment    http://www.apache.org/licenses/LICENSE-2.0
\comment
\comment Unless required by applicable law or agreed to in writing, software
\comment distributed under the License is distributed on an "AS IS" BASIS,
\comment WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
\comment See the License for the specific language governing permissions and
\comment limitations under the License.


\page features The features known by the measurement tool

This page describes all the features known to \ref dip::MeasurementTool by default. They
are sorted into sections in the same way as the table in the documentation to
`dip::MeasurementTool`


\comment --------------------------------------------------------------

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
\ref dip::ChainCode::Length. If the object touches the image edge, only the portion of the
perimeter that does not coincide with the image edge is measured.

Note that the chain code measures work only for 2D images, and expect objects to be a single
connected component. If multiple connected components have the same label, only the first
connected component found for that label will be measured.
Any holes in the object are not included in the perimeter either.

\subsection size_features_SurfaceArea SurfaceArea
The 3D equivalent to the \ref size_features_Perimeter feature. It does not assume a single connected component,
and will include all surfaces in the measurement, including those of holes.

!!! literature
    - J.C. Mullikin and P.W. Verbeek, "Surface area estimation of digitized planes," Bioimaging 1(1):6-16, 1993.

\subsection size_features_Feret Feret
Computes the maximum and minimum object diameters from the object's convex hull, using
\ref dip::ConvexHull::Feret. The convex hull is computed from the chain code using \ref dip::ChainCode::ConvexHull.

Note that the chain code measures work only for 2D images, and expect objects to be a single
connected component. If multiple connected components have the same label, only the first
connected component found for that label will be measured.

Five values are returned:

 - 0: `Max`, the maximum Feret diameter, the length of the object.
 - 1: `Min`, the minimum Feret diameter, the width of the object.
 - 2: `PerpMin`, the Feret diameter perpendicular to `Min`, which is not necessarily
   equal to `Max`. `Min` and `PerpMin` are (usually, but not necessarily)
   the sizes of the minimum bounding box.
 - 3: `MaxAng`, the angle at which `Max` was obtained.
 - 4: `MinAng`, the angle at which `Min` was obtained.

\subsection size_features_SolidArea SolidArea
Computes the area of the object ignoring any holes. It uses the object's chain code
and the \ref dip::ChainCode::Area method.

Note that the chain code measures work only for 2D images, and expect objects to be a single
connected component. If multiple connected components have the same label, only the first
connected component found for that label will be measured.

\subsection size_features_ConvexArea ConvexArea
The area of the convex hull of the object. The convex hull is computed from the chain code
using \ref dip::ChainCode::ConvexHull.

Note that the chain code measures work only for 2D images, and expect objects to be a single
connected component. If multiple connected components have the same label, only the first
connected component found for that label will be measured.

\subsection size_features_ConvexPerimeter ConvexPerimeter
The perimeter of the convex hull. The convex hull is computed from the chain code.

Note that the chain code measures work only for 2D images, and expect objects to be a single
connected component. If multiple connected components have the same label, only the first
connected component found for that label will be measured.


\comment --------------------------------------------------------------

\section shape_features Shape features

\subsection shape_features_AspectRatioFeret AspectRatioFeret
The ratio `PerpMin`/`Min`, two of the values returned by the \ref size_features_Feret feature.

\subsection shape_features_Radius Radius
Statistics on the radius of the object, computed from the chain code using
\ref dip::ChainCode::Polygon and \ref dip::Polygon::RadiusStatistics.

Note that the chain code measures work only for 2D images, and expect objects to be a single
connected component. If multiple connected components have the same label, only the first
connected component found for that label will be measured.

This feature takes the distances from the object's centroid (as determined from the chain
code, not influenced by any holes in the object) to each of the border pixels. From these
distances it computes four values:
 - 0: `Max`, the largest distance.
 - 1: `Mean`, the average distance.
 - 2: `Min`, the shortest distance.
 - 3: `StD`, the standard deviation.

Note that the centroid does not necessarily lie within the object.

\subsection shape_features_P2A P2A
Computes:

 - 2D: $\frac{p^2}{4 \pi a}$, where $p$ is the perimeter and $a$ is the area.
 - 3D: $\frac{\sqrt{a^3}}{6v\sqrt{\pi}}$, where $a$ is the surface area and
   $v$ is the volume.

See \ref size_features_Perimeter,
\ref size_features_SurfaceArea and \ref size_features_Size. For solid objects, this
measure is the reciprocal of \ref shape_features_Roundness.

\subsection shape_features_Roundness Roundness
Computes $\frac{4\pi a}{p^2}$, where $a$ is the solid area and $p$ is the perimeter,
using the features \ref size_features_SolidArea and \ref size_features_Perimeter.
This measure is in the range (0,1], with 1 for a perfect circle.
For solid objects, it is the reciprocal of \ref shape_features_P2A, but for objects with holes,
this measure takes only the outer boundary into account.

\subsection shape_features_Circularity Circularity
Circularity is a measure of similarity to a circle, and is given by coefficient of variation
of the radii of the object. It is computed by the ratio `StD`/`Mean`
of the \ref shape_features_Radius feature, and is 0 for a perfect circle.
See `dip::RadiusValues::Circularity`.

\subsection shape_features_PodczeckShapes PodczeckShapes
Computes the 5 Podczeck shape descriptors using the results of features \ref size_features_Size,
\ref size_features_Feret and \ref size_features_Perimeter. The shape descriptors are:

 - 0: `Square`, similarity to a square, $\frac{a}{wh}$.
 - 1: `Circle`, similarity to a circle, $\frac{4a}{\pi h^2}$.
 - 2: `Triangle`, similarity to a triangle, $\frac{2a}{wh}$.
 - 3: `Ellipse`, similarity to an ellipse, $\frac{4a}{\pi wh}$.
 - 4: `Elongation`, object elongation, $\frac{p}{l}$.

where $a$ is the object area, $p$ the perimeter, $l$ the largest Feret diameter, $w$ the
smallest Feret diameter, and $h$ the diameter perpendicular to the smallest diameter
(`PerpMin` value of the \ref size_features_Feret feature).

\subsection shape_features_Solidity Solidity
The ratio `Size`/`ConvexArea` of the features \ref size_features_Size and \ref size_features_ConvexArea.
It is in the range (0,1], with 1 for a convex object.

\subsection shape_features_Convexity Convexity
The ratio `ConvexPerimeter`/`Perimeter` of the features \ref size_features_Perimeter and
\ref size_features_ConvexPerimeter. It is in the range (0,1], with 1 for a convex object.

\subsection shape_features_EllipseVariance EllipseVariance
A measure for deviation from an elliptic shape, computed using \ref dip::ChainCode::Polygon and
\ref dip::Polygon::EllipseVariance from the chain code.

Note that the chain code measures work only for 2D images, and expect objects to be a single
connected component. If multiple connected components have the same label, only the first
connected component found for that label will be measured.

\subsection shape_features_Eccentricity Eccentricity
Aspect ratio of the best fit ellipse, computed using \ref dip::CovarianceMatrix::Eigenvalues::Eccentricity
from the covariance matrix of the chain code. Eccentricity is defined as
$\sqrt{1-\frac{\lambda_2}{\lambda_1}}$, with $\lambda_1$ the largest eigenvalue
and $\lambda_2$ the smallest eigenvalue of the covariance matrix of the boundary
pixels of the object.

Note that the chain code measures work only for 2D images, and expect objects to be a single
connected component. If multiple connected components have the same label, only the first
connected component found for that label will be measured.

\subsection shape_features_BendingEnergy BendingEnergy
Bending energy of object perimeter, computed using \ref dip::ChainCode::BendingEnergy from the
object's chain code.

Note that the chain code measures work only for 2D images, and expect objects to be a single
connected component. If multiple connected components have the same label, only the first
connected component found for that label will be measured.


\comment --------------------------------------------------------------

\section intensity_features Intensity features

\subsection intensity_features_Mass Mass
The sum of the grey-value image intensities across the object.
The `grey` image can be a tensor image, one value per tensor element (channel) is produced.

\subsection intensity_features_Mean Mean
The mean of the grey-value image intensities across the object.
The `grey` image can be a tensor image, one value per tensor element (channel) is produced.

\subsection intensity_features_StandardDeviation StandardDeviation
The standard deviation of the grey-value image intensities across the object.
The `grey` image can be a tensor image, one value per tensor element (channel) is produced.

A fast algorithm is used that could result in catastrophic cancellation if
the mean is much larger than the variance, see \ref dip::FastVarianceAccumulator.
If there is a potential for this to happen, choose the \ref intensity_features_Statistics
feature instead.

\subsection intensity_features_Statistics Statistics
The mean, standard deviation, skewness and excess kurtosis of the grey-value image intensities
across the object.
This feature has 4 values, `grey` must be scalar.

A stable algorithm is used that prevents catastrophic cancellation, see \ref dip::StatisticsAccumulator.

\subsection intensity_features_DirectionalStatistics DirectionalStatistics
The directional mean and standard deviation of the grey-value image intensities
across the object.
Directional statistics assume the input values are angles.
This feature has 2 values, `grey` must be scalar.

\subsection intensity_features_MaxVal MaxVal
The maximum grey-value image intensity within the object.
The `grey` image can be a tensor image, one value per tensor element (channel) is produced.

\subsection intensity_features_MinVal MinVal
The minimum grey-value image intensity within the object.
The `grey` image can be a tensor image, one value per tensor element (channel) is produced.

\subsection intensity_features_MaxPos MaxPos
The position of the pixel with maximum intensity within the object. There is one value per
image dimension. If the image has a known pixel size, the values represent the distances
to the lower image edge along each dimension.

If multiple pixels have the same minimum value, the position of the first one encountered
is returned. Note that the order in which pixels are examined depends on the image sizes and
strides, and is not given by the linear index.

The `grey` image must be a scalar image.

\subsection intensity_features_MinPos MinPos
The position of the pixel with minimum intensity within the object. There is one value per
image dimension. If the image has a known pixel size, the values represent the distances
to the lower image edge along each dimension.

If multiple pixels have the same minimum value, the position of the first one encountered
is returned. Note that the order in which pixels are examined depends on the image sizes and
strides, and is not given by the linear index.

The `grey` image must be a scalar image.


\comment --------------------------------------------------------------

\section binary_moments Moments of binary object

\subsection binary_moments_Size Size
The \ref size_features_Size feature is the zero order moment of the binary object.

\subsection binary_moments_Center Center
Coordinates of the geometric mean (centroid) of the object (with sub-pixel precision),
which is the first order normalized moment of the binary shape.
There is one value per image dimension. If the image has a known pixel size, the values
represent the distances to the lower image edge along each dimension.

\subsection binary_moments_Mu Mu
Elements of the inertia tensor of the object, which is composed of second order
normalized central moments of the binary shape. For an image with $n$ dimensions,
there are $\frac{1}{2}n(n+1)$ values. These are stored in the same order as symmetric
tensors are stored in an image (see \ref dip::Tensor::Shape).

For more information, see \ref dip::MomentAccumulator::SecondOrder.

\subsection binary_moments_Inertia Inertia
Moments of inertia of the binary object, the eigenvalues of the tensor computed by
feature \ref binary_moments_Mu. There is one value per image dimension. The eigenvectors are sorted
largest to smallest.

\subsection binary_moments_MajorAxes MajorAxes
Principal axes of the binary object, the eigenvectors of the tensor computed by
feature \ref binary_moments_Mu. For an image with $n$ dimensions, there are $n^2$ values.
The first $n$ values are the eigenvector associated to the largest eigenvalue, etc.

\subsection binary_moments_DimensionsCube DimensionsCube
Lengths of the sides of a rectangle (2D) or box (3D) with the same moments of inertia
as the binary object. Derived from feature \ref binary_moments_Inertia.

Currently defined only for 2D and 3D images.

\subsection binary_moments_DimensionsEllipsoid DimensionsEllipsoid
Diameters of an ellipse (2D) or ellipsoid (3D) with the same moments of inertia as the
binary object. Derived from feature \ref binary_moments_Inertia.

Currently defined only for 2D and 3D images.


\comment --------------------------------------------------------------

\section grey_moments Moments of grey-value object

\subsection grey_moments_GreySize GreySize
The zero order moment of the object. Same as the \ref intensity_features_Mass feature,
but multiplied by the physical size of a pixel. If object pixels have a value of 1, and
background pixels have a value of 0, then this feature is the size of the grey-value
object.

\subsection grey_moments_Gravity Gravity
Coordinates of the center of mass of the object, which is the first order normalized
moment of the binary shape weighted by the grey-value image's intensities.
There is one value per image dimension. If the image has a known pixel size, the values
represent the distances of the center of mass to the lower image edge along each dimension.

Identical to feature \ref binary_moments_Center but using the grey-value image as weighting.
`grey` must be scalar.

\subsection grey_moments_GreyMu GreyMu
Elements of the inertia tensor of the grey-weighted object, which is composed of second order
normalized central moments of the binary shape weighted by the grey-value image's intensities.
For an image with $n$ dimensions, there are $\frac{1}{2}n(n+1)$ values. These are stored
in the same order as symmetric tensors are stored in an image (see \ref dip::Tensor::Shape).

For more information, see \ref dip::MomentAccumulator::SecondOrder.

Identical to feature \ref binary_moments_Mu but using the grey-value image as weighting.
`grey` must be scalar.

\subsection grey_moments_GreyInertia GreyInertia
Moments of inertia of the grey-weighted object, the eigenvalues of the tensor computed by
feature \ref grey_moments_GreyMu. There is one value per image dimension. The eigenvectors are sorted
largest to smallest.

Identical to feature \ref binary_moments_Inertia but using the grey-value image as weighting.
`grey` must be scalar.

\subsection grey_moments_GreyMajorAxes GreyMajorAxes
Principal axes of the grey-weighted object, the eigenvectors of the tensor computed by
feature \ref grey_moments_GreyMu. For an image with $n$ dimensions, there are $n^2$ values.
The first $n$ values are the eigenvector associated to the largest eigenvalue, etc.

Identical to feature \ref binary_moments_MajorAxes but using the grey-value image as weighting.
`grey` must be scalar.

\subsection grey_moments_GreyDimensionsCube GreyDimensionsCube
Lengths of the sides of a rectangle (2D) or box (3D) with the same moments of inertia
as the grey-weighted object. Derived from feature \ref grey_moments_GreyInertia.

Currently defined only for 2D and 3D images.

Identical to feature \ref binary_moments_DimensionsCube but using the grey-value image as weighting.
`grey` must be scalar.

\subsection grey_moments_GreyDimensionsEllipsoid GreyDimensionsEllipsoid
Diameters of an ellipse (2D) or ellipsoid (3D) with the same moments of inertia as the
grey-weighted object. Derived from feature \ref grey_moments_GreyInertia.

Currently defined only for 2D and 3D images.

Identical to feature \ref binary_moments_DimensionsEllipsoid but using the grey-value image as weighting.
`grey` must be scalar.
