/*
 * DIPlib 3.0
 * This file contains declarations and definitions for chain-code--based 2D measurements
 *
 * (c)2016-2019, Cris Luengo.
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

#ifndef DIP_CHAIN_CODE_H
#define DIP_CHAIN_CODE_H

#include "diplib.h"
#include "accumulators.h"

/// \file
/// \brief Support for chain-code and polygon object representation and quantification. Everything declared in
/// this file is explicitly 2D.
/// \see measurement


namespace dip {


/// \addtogroup measurement
/// \{


/// \brief Contains the various %Feret diameters as returned by `dip::ConvexHull::Feret` and `dip::ChainCode::Feret`.
struct DIP_NO_EXPORT FeretValues {
   dfloat maxDiameter = 0.0;        ///< The maximum %Feret diameter
   dfloat minDiameter = 0.0;        ///< The minimum %Feret diameter
   dfloat maxPerpendicular = 0.0;   ///< The %Feret diameter perpendicular to `minDiameter`
   dfloat maxAngle = 0.0;           ///< The angle at which `maxDiameter` was measured
   dfloat minAngle = 0.0;           ///< The angle at which `minDiameter` was measured
};

/// \brief Holds the various output values of the `dip::RadiusStatistics` and `dip::ConvexHull::RadiusStatistics` function.
class DIP_NO_EXPORT RadiusValues {
   public:
      /// Returns the mean radius
      dfloat Mean() const { return vacc.Mean(); }
      /// Returns the standard deviation of radii
      dfloat StandardDeviation() const { return vacc.StandardDeviation(); }
      /// Returns the variance of radii
      dfloat Variance() const { return vacc.Variance(); }
      /// Returns the maximum radius
      dfloat Maximum() const { return macc.Maximum(); }
      /// Returns the minimum radius
      dfloat Minimum() const { return macc.Minimum(); }

      /// Computes a circularity measure given by the coefficient of variation of the radii of the object.
      dfloat Circularity() const {
         return vacc.Mean() == 0.0 ? 0.0 : vacc.StandardDeviation() / vacc.Mean();
      }

      /// Multiple `%RadiusValues` objects can be added together.
      RadiusValues& operator+=( RadiusValues const& other ) {
         vacc += other.vacc;
         macc += other.macc;
         return *this;
      }

      void Push( dfloat x ) {
         vacc.Push( x );
         macc.Push( x );
      }

   private:
      VarianceAccumulator vacc;
      MinMaxAccumulator macc;
};


//
// Vertex of a polygon
//


/// \brief Encodes a location in a 2D image
template< typename T >
struct DIP_NO_EXPORT Vertex {
   T x;   ///< The x-coordinate
   T y;   ///< The y-coordinate

   /// Default constructor
   constexpr Vertex() : x( T( 0 )), y( T( 0 )) {}
   /// Constructor
   constexpr Vertex( T x, T y ) : x( x ), y( y ) {}
   /// Constructor
   template< typename V >
   explicit Vertex( Vertex< V > v ) : x( static_cast< T >( v.x )), y( static_cast< T >( v.y )) {}

   /// Add a vertex
   template< typename V >
   Vertex& operator+=( Vertex< V > v ) {
      x += T( v.x );
      y += T( v.y );
      return *this;
   }
   /// Subtract a vertex
   template< typename V >
   Vertex& operator-=( Vertex< V > v ) {
      x -= T( v.x );
      y -= T( v.y );
      return *this;
   }
   /// Add a constant to both coordinate components
   Vertex& operator+=( T n ) {
      x += n;
      y += n;
      return *this;
   }
   /// Subtract a constant from both coordinate components
   Vertex& operator-=( T n ) {
      x -= n;
      y -= n;
      return *this;
   }
   /// Scale by a constant, isotropically
   Vertex& operator*=( dfloat n ) {
      x = T( dfloat( x ) * n );
      y = T( dfloat( y ) * n );
      return *this;
   }
   /// Scale by the inverse of a constant, isotropically
   Vertex& operator/=( dfloat n ) {
      x = T( dfloat( x ) / n );
      y = T( dfloat( y ) / n );
      return *this;
   }
   /// Round coordinates to nearest integer
   Vertex Round() const {
      return{ std::round( x ), std::round( y ) };
   }
};

/// \brief A vertex with floating-point coordinates
/// \relates dip::Vertex
using VertexFloat = Vertex< dfloat >;

/// \brief A vertex with integer coordinates
/// \relates dip::Vertex
using VertexInteger = Vertex< dip::sint >;

/// \brief Compare two vertices
/// \relates dip::Vertex
template< typename T >
inline bool operator==( Vertex< T > v1, Vertex< T > v2 ) {
   return ( v1.x == v2.x ) && ( v1.y == v2.y );
}

template< typename T >
/// \brief Compare two vertices
/// \relates dip::Vertex
inline bool operator!=( Vertex< T > v1, Vertex< T > v2 ) {
   return !( v1 == v2 );
}

/// \brief The norm of the vector `v`.
/// \relates dip::Vertex
template< typename T >
inline dfloat Norm( Vertex< T > const& v ) {
   return std::hypot( v.x, v.y );
}

/// \brief The square of the norm of the vector `v`.
/// \relates dip::Vertex
template< typename T >
inline dfloat NormSquare( Vertex< T > const& v ) {
   return v.x * v.x + v.y * v.y;
}

/// \brief The norm of the vector `v2-v1`.
/// \relates dip::Vertex
template< typename T >
inline dfloat Distance( Vertex< T > const& v1, Vertex< T > const& v2 ) {
   return Norm( v2 - v1 );
}

/// \brief The square norm of the vector `v2-v1`.
/// \relates dip::Vertex
template< typename T >
inline dfloat DistanceSquare( Vertex< T > const& v1, Vertex< T > const& v2 ) {
   return NormSquare( v2 - v1 );
}

/// \brief The angle of the vector `v2-v1`.
/// \relates dip::Vertex
template< typename T >
inline dfloat Angle( Vertex< T > const& v1, Vertex< T > const& v2 ) {
   Vertex< T > v = v2 - v1;
   return std::atan2( v.y, v.x );
}

/// \brief Compute the z component of the cross product of vectors `v1` and `v2`
/// \relates dip::Vertex
template< typename T >
inline dfloat CrossProduct( Vertex< T > const& v1, Vertex< T > const& v2 ) {
   return v1.x * v2.y - v1.y * v2.x;
}

/// \brief Compute the z component of the cross product of vectors `v2-v1` and `v3-v1`
/// \relates dip::Vertex
template< typename T >
inline dfloat ParallelogramSignedArea( Vertex< T > const& v1, Vertex< T > const& v2, Vertex< T > const& v3 ) {
   return CrossProduct( v2 - v1, v3 - v1 );
}

/// \brief Compute the area of the triangle formed by vertices `v1`, `v2` and `v3`
/// \relates dip::Vertex
template< typename T >
inline dfloat TriangleArea( Vertex< T > const& v1, Vertex< T > const& v2, Vertex< T > const& v3 ) {
   return std::abs( ParallelogramSignedArea< T >( v1, v2, v3 ) / 2.0 );
}

/// \brief Compute the height of the triangle formed by vertices `v1`, `v2` and `v3`, with `v3` the tip
/// \relates dip::Vertex
template< typename T >
inline dfloat TriangleHeight( Vertex< T > const& v1, Vertex< T > const& v2, Vertex< T > const& v3 ) {
   return std::abs( ParallelogramSignedArea< T >( v1, v2, v3 ) / Distance< T >( v1, v2 ));
}

/// \brief Add two vertices together, with identical types
/// \relates dip::Vertex
template< typename T >
inline Vertex< T > operator+( Vertex< T > lhs, Vertex< T > const& rhs ) {
   lhs += rhs;
   return lhs;
}

/// \brief Add two vertices together, where the LHS is floating-point and the RHS is integer
/// \relates dip::Vertex
inline VertexFloat operator+( VertexFloat lhs, VertexInteger const& rhs ) {
   lhs += rhs;
   return lhs;
}

/// \brief Add two vertices together, where the LHS is integer and the RHS is floating-point
/// \relates dip::Vertex
inline VertexFloat operator+( VertexInteger const& lhs, VertexFloat rhs ) {
   rhs += lhs;
   return rhs;
}

/// \brief Subtract two vertices from each other
/// \relates dip::Vertex
template< typename T >
inline Vertex< T > operator-( Vertex< T > lhs, Vertex< T > const& rhs ) {
   lhs -= rhs;
   return lhs;
}

/// \brief Subtract two vertices from each other, where the LHS is floating-point and the RHS is integer
/// \relates dip::Vertex
inline VertexFloat operator-( VertexFloat lhs, VertexInteger const& rhs ) {
   lhs -= rhs;
   return lhs;
}

/// \brief Subtract two vertices from each other, where the LHS is integer and the RHS is floating-point
/// \relates dip::Vertex
inline VertexFloat operator-( VertexInteger const& lhs, VertexFloat const& rhs ) {
   VertexFloat out{ static_cast< dfloat >( lhs.x ),
                    static_cast< dfloat >( lhs.y ) };
   out -= rhs;
   return out;
}

/// \brief Add a vertex and a constant
/// \relates dip::Vertex
template< typename T, typename S >
inline Vertex< T > operator+( Vertex< T > v, S n ) {
   v += T( n );
   return v;
}

/// \brief Subtract a vertex and a constant
/// \relates dip::Vertex
template< typename T, typename S >
inline Vertex< T > operator-( Vertex< T > v, S n ) {
   v -= T( n );
   return v;
}

/// \brief Multiply a vertex and a constant
/// \relates dip::Vertex
template< typename T >
inline Vertex< T > operator*( Vertex< T > v, dfloat n ) {
   v *= n;
   return v;
}

/// \brief Divide a vertex by a constant
/// \relates dip::Vertex
template< typename T >
inline Vertex< T > operator/( Vertex< T > v, dfloat n ) {
   v /= n;
   return v;
}

/// \brief Encodes a bounding box in a 2D image by the top left and bottom right corners (both coordinates included in the box).
template< typename T >
struct DIP_NO_EXPORT BoundingBox {
   using VertexType = Vertex< T >;  ///< The bounding box is defined in terms of two vertices
   VertexType topLeft;     ///< Top-left corner of the box
   VertexType bottomRight; ///< Bottom-right corner of the box

   /// Default constructor, yields a bounding box of a single pixel at `{0,0}`
   constexpr BoundingBox() = default;
   /// Constructor, yields a bounding box of a single pixel at `pt`
   constexpr explicit BoundingBox( VertexType pt ) : topLeft( pt ), bottomRight( pt ) {}
   /// Constructor, yields a bounding box with the two points as two of its vertices
   BoundingBox( VertexType a, VertexType b ) {
         if( a.x < b.x ) {
            topLeft.x = a.x;
            bottomRight.x = b.x;
         } else {
            topLeft.x = b.x;
            bottomRight.x = a.x;
         }
         if( a.y < b.y ) {
            topLeft.y = a.y;
            bottomRight.y = b.y;
         } else {
            topLeft.y = b.y;
            bottomRight.y = a.y;
         }
   }
   /// Expand bounding box to include given point.
   void Expand( VertexType pt ) {
      if( pt.x < topLeft.x ) {
         topLeft.x = pt.x;
      } else if( pt.x > bottomRight.x ) {
         bottomRight.x = pt.x;
      }
      if( pt.y < topLeft.y ) {
         topLeft.y = pt.y;
      } else if( pt.y > bottomRight.y ) {
         bottomRight.y = pt.y;
      }
   }
   /// Tests to see if the given point is inside the bounding box.
   bool Contains( VertexInteger pt ) {
      return !(( static_cast< T >( pt.x ) < topLeft.x ) || ( static_cast< T >( pt.x ) > bottomRight.x ) ||
               ( static_cast< T >( pt.y ) < topLeft.y ) || ( static_cast< T >( pt.y ) > bottomRight.y ));
   }
   /// Tests to see if the given point is inside the bounding box.
   bool Contains( VertexFloat pt ) {
      return !(( pt.x < static_cast< dfloat >( topLeft.x )) || ( pt.x > static_cast< dfloat >( bottomRight.x )) ||
               ( pt.y < static_cast< dfloat >( topLeft.y )) || ( pt.y > static_cast< dfloat >( bottomRight.y )));
   }
   /// Returns the size of the bounding box.
   DimensionArray< T > Size() const;
};

/// \brief A bounding box with floating-point coordinates
/// \relates dip::BoundingBox
using BoundingBoxFloat = BoundingBox< dfloat >;

/// \brief A bounding box with integer coordinates
/// \relates dip::BoundingBox
using BoundingBoxInteger = BoundingBox< dip::sint >;

template<>
inline IntegerArray BoundingBox< dip::sint >::Size() const {
   auto res = bottomRight - topLeft + 1;
   return { res.x, res.y };
}

template<>
inline FloatArray BoundingBox< dfloat >::Size() const {
   auto res = bottomRight - topLeft;
   return { res.x, res.y };
}

//
// Covariance matrix
//


/// \brief A 2D covariance matrix for computation with 2D vertices.
///
/// The matrix is real, symmetric, positive semidefinite. See `dip::Polygon::CovarianceMatrix`
/// for how to create a covariance matrix.
///
/// The elements stored are `xx`, `xy` and `yy`, with `xx` the top-left element, and `xy` both
/// the off-diagonal elements, which are equal by definition.
class DIP_NO_EXPORT CovarianceMatrix {
   public:
      /// \brief Default-initialized covariance matrix is all zeros
      CovarianceMatrix() = default;
      /// \brief Construct a covariance matrix as the outer product of a vector and itself
      explicit CovarianceMatrix( VertexFloat v ) : xx_( v.x * v.x ), xy_( v.x * v.y ), yy_( v.y * v.y ) {}
      /// \brief Read matrix element
      dfloat xx() const { return xx_; }
      /// \brief Read matrix element
      dfloat xy() const { return xy_; }
      /// \brief Read matrix element
      dfloat yy() const { return yy_; }
      /// \brief Compute determinant of matrix
      dfloat Det() const { return xx_ * yy_ - xy_ * xy_; }
      /// \brief Compute inverse of matrix
      CovarianceMatrix Inv() const {
         dfloat d = Det();
         CovarianceMatrix out;
         if( d != 0.0 ) {
            out.xx_ =  yy_ / d;
            out.xy_ = -xy_ / d;
            out.yy_ =  xx_ / d;
         }
         return out;
      }
      /// \brief Add other matrix to this matrix
      CovarianceMatrix& operator+=( CovarianceMatrix const& other ) {
         xx_ += other.xx_;
         xy_ += other.xy_;
         yy_ += other.yy_;
         return * this;
      }
      /// \brief Scale matrix
      CovarianceMatrix& operator*=( dfloat d ) {
         xx_ *= d;
         xy_ *= d;
         yy_ *= d;
         return * this;
      }
      /// \brief Scale matrix
      CovarianceMatrix& operator/=( dfloat d ) {
         return operator*=( 1.0 / d );
      }
      /// \brief Computes v' * C * v, with v' the transpose of v.
      /// This is a positive scalar if v is non-zero, because C (this matrix) is positive semidefinite.
      dfloat Project( VertexFloat const& v ) const {
         return v.x * v.x * xx_ + 2 * v.x * v.y * xy_ + v.y * v.y * yy_;
      }

      /// \brief Container for matrix eigenvalues
      struct Eigenvalues {
         dfloat largest;   ///< Largest eigenvalue
         dfloat smallest;  ///< Smallest eigenvalue

         /// \brief Computes eccentricity using the two eigenvalues of the covariance matrix.
         dfloat Eccentricity() const {
            // Eccentricity according to https://en.wikipedia.org/wiki/Image_moment
            // largest cannot be negative; if largest == 0, then smallest == 0 also.
            return largest <= 0.0 ? 0.0 : std::sqrt( 1.0 - smallest / largest );
         }
      };
      /// \brief Compute eigenvalues of matrix
      Eigenvalues Eig() const {
         // Eigenvalue calculation according to e.g. http://www.math.harvard.edu/archive/21b_fall_04/exhibits/2dmatrices/index.html
         dfloat mmu2 = ( xx_ + yy_ ) / 2.0;
         dfloat dmu2 = ( xx_ - yy_ ) / 2.0;
         dfloat sqroot = std::sqrt( xy_ * xy_ + dmu2 * dmu2 );
         return { mmu2 + sqroot, mmu2 - sqroot };
      }

      /// \brief Container for ellipse parameters
      struct EllipseParameters {
         dfloat majorAxis;    ///< Major axis length
         dfloat minorAxis;    ///< Minor axis length
         dfloat orientation;  ///< Orientation of major axis
         dfloat eccentricity; ///< Ellipse eccentricity
      };
      /// \brief Compute parameters of ellipse with same covariance matrix.
      EllipseParameters Ellipse() const {
         // Eigenvector calculation according to e.g. http://www.math.harvard.edu/archive/21b_fall_04/exhibits/2dmatrices/index.html
         Eigenvalues lambda = Eig();
         return {
               std::sqrt( 8.0 * lambda.largest ),
               std::sqrt( 8.0 * lambda.smallest ),
               // eigenvector is {xy, lambda.largest - xx}, always has an angle in the range [0,pi).
               std::atan2( lambda.largest - xx_, xy_ ),
               lambda.Eccentricity()
         };
      }

   private:
      dfloat xx_ = 0, xy_ = 0, yy_ = 0;
};


//
// Polygon, convex hull
//


class DIP_NO_EXPORT ConvexHull; // Forward declaration

/// \brief A polygon with floating-point vertices.
struct DIP_NO_EXPORT Polygon {

   using Vertices = std::vector< VertexFloat >;
   Vertices vertices;  ///< The vertices

   /// \brief Returns the bounding box of the polygon
   DIP_EXPORT BoundingBoxFloat BoundingBox() const;

   /// \brief Determine the orientation of the polygon (if constructed from a chain code, should return true)
   DIP_EXPORT bool IsClockWise() const;

   /// \brief Computes the (signed) area of the polygon. Default, clockwise polygons have a positive area.
   DIP_EXPORT dfloat Area() const;

   /// \brief Computes the centroid of the polygon
   DIP_EXPORT VertexFloat Centroid() const;

   /// \brief Returns the covariance matrix for the vertices of the polygon, using centroid `g`.
   DIP_EXPORT dip::CovarianceMatrix CovarianceMatrix( VertexFloat const& g ) const;

   /// \brief Returns the covariance matrix for the vertices of the polygon.
   dip::CovarianceMatrix CovarianceMatrix() const {
      return this->CovarianceMatrix( Centroid() );
   }

   /// \brief Computes the length of the polygon (i.e. perimeter). If the polygon represents a pixelated object,
   /// this function will overestimate the object's perimeter. Use `dip::ChainCode::Length` instead.
   DIP_EXPORT dfloat Length() const;

   /// \brief Returns statistics on the radii of the polygon. The radii are the distances between the centroid
   /// and each of the vertices.
   RadiusValues RadiusStatistics() const {
      VertexFloat g = Centroid();
      return RadiusStatistics( g );
   }

   /// \brief Returns statistics on the radii of the polygon. The radii are the distances between the given centroid
   /// and each of the vertices.
   DIP_EXPORT RadiusValues RadiusStatistics( VertexFloat const& g ) const;

   /// \brief Compares a polygon to the ellipse with the same covariance matrix, returning the coefficient of
   /// variation of the distance of vertices to the ellipse.
   ///
   /// \literature
   /// <li>M. Yang, K. Kpalma and J. Ronsin, "A Survey of Shape Feature Extraction Techniques",
   ///     in: Pattern Recognition Techniques, Technology and Applications, P.Y. Yin (Editor), I-Tech, 2008.
   /// \endliterature
   dfloat EllipseVariance() const {
       // Covariance matrix of polygon vertices
       VertexFloat g = Centroid();
       dip::CovarianceMatrix C = this->CovarianceMatrix(g);
       return EllipseVariance( g, C );
   }

   /// \brief Compares a polygon to the ellipse described by the given centroid and covariance matrix, returning
   /// the coefficient of variation of the distance of vertices to the ellipse.
   DIP_EXPORT dfloat EllipseVariance( VertexFloat const& g, dip::CovarianceMatrix const& C ) const;

   /// \brief Computes the fractal dimension of a polygon.
   ///
   /// Fractal dimension is defined as the slope of the polygon length as a function of scale, in a log-log plot.
   /// Scale is obtained by smoothing the polygon using `dip::Polygon::Smooth`. Therefore, it is important that
   /// the polygon be densely sampled, use `dip::Polygon::Augment` if necessary.
   ///
   /// `length` is the length of the polygon (see `dip::Polygon::Length`). It determines the range of scales used
   /// to compute the fractal dimension, so a rough estimate is sufficient. If zero is given as length (the default
   /// value), then it is computed.
   DIP_EXPORT dfloat FractalDimension( dfloat length = 0 ) const;

   /// \brief Computes the bending energy of a polygon.
   ///
   /// The bending energy is the integral along the contour of the square of the curvature.
   /// We approximate curvature by, at each vertex, taking the difference in angle between
   /// the two edges, and dividing by half the length of the two edges (this is the portion
   /// of the boundary associated to the edge).
   ///
   /// Note that this approximation is poor when the points are far apart. `dip::Polygon::Augment` should
   /// be used to obtain a densely sampled polygon. It is also beneficial to sufficiently smooth the
   /// polygon so it better approximates a smooth curve around the object being measured, see `dip::Polygon::Smooth`.
   ///
   /// \literature
   /// <li>I.T. Young, J.E. Walker and J.E. Bowie, "An Analysis Technique for Biological Shape I",
   ///     Information and Control 25(4):357-370, 1974.
   /// <li>J.E. Bowie and I.T. Young, "An Analysis Technique for Biological Shape - II",
   ///     Acta Cytologica 21(5):455-464, 1977.
   /// \endliterature
   DIP_EXPORT dfloat BendingEnergy() const;

   /// \brief Simplifies the polygon using the Douglas-Peucker algorithm.
   ///
   /// For a polygon derived from a chain code, setting tolerance to 0.5 leads to a maximum-length digital straight
   /// segment representation of the object.
   DIP_EXPORT Polygon& Simplify( dfloat tolerance = 0.5 );

   /// \brief Adds vertices along each edge of the polygon such that the distance between two consecutive
   /// vertices is never more than `distance`.
   DIP_EXPORT Polygon& Augment( dfloat distance = 1.0 );

   /// \brief Locally averages the location of vertices of a polygon so it becomes smoother.
   ///
   /// Uses a Gaussian filter with parameter `sigma`, which is not interpreted as a physical distance between
   /// vertices, but as a distance in number of vertices. That is, the neighboring vertex is at a distance of 1,
   /// the next one over at a distance of 2, etc. Therefore, it is important that vertices are approximately equally
   /// spaced. `dip::Polygon::Augment` modifies any polygon to satisfy that requirement.
   ///
   /// A polygon derived from the chain code of an object without high curvature, when smoothed with a `sigma` of 2,
   /// will fairly well approximate the original smooth boundary. For objects with higher curvature (including very
   /// small objects), choose a smaller `sigma`.
   DIP_EXPORT Polygon& Smooth( dfloat sigma = 1.0 );

   /// \brief Returns the convex hull of the polygon.
   DIP_EXPORT dip::ConvexHull ConvexHull() const;
};

/// \brief A convex hull is a convex polygon. It can be constructed from a `dip::Polygon`, and a const reference
/// to the underlying `dip::Polygon` object can be obtained. It is guaranteed clockwise.
class DIP_NO_EXPORT ConvexHull {
   public:

      /// Default-constructed ConvexHull (without vertices)
      ConvexHull() = default;

      /// Constructs a convex hull of a polygon
      DIP_EXPORT explicit ConvexHull( dip::Polygon const& polygon );

      /// Returns the polygon representing the convex hull
      dip::Polygon const& Polygon() const {
         return polygon_;
      }

      /// Returns the area of the convex hull
      dfloat Area() const {
         return polygon_.Area();
      }

      /// Returns the perimeter of the convex hull
      dfloat Perimeter() const {
         return polygon_.Length();
      }

      /// Returns the %Feret diameters of the convex hull
      ///
      /// The Feret diameters of the convex hull correspond to the Feret diameters of the original polygon.
      /// Feret diameters are the lengths of the projections. This function determines the longest and the shortest
      /// projections, as well as the length of the projection perpendicular to the shortest.
      ///
      /// These values are obtained by enumerating anti-podal pairs using the "rotating calipers" algorithm by
      /// Preparata and Shamos (1985).
      ///
      /// \literature
      /// <li>F.P. Preparata and M.I. Shamos, "Computational Geometry: an Introduction", Springer-Verlag, 1985.
      /// \endliterature
      DIP_EXPORT FeretValues Feret() const;

   private:
      dip::Polygon polygon_;
};

/// \}

// This function cannot be written inside the dip::Polygon class because it needs to know about the dip::ConvexHull
// class, which in turn needs to know about the dip::Polygon class.
inline dip::ConvexHull Polygon::ConvexHull() const {
   return dip::ConvexHull( *this );
}

/// \addtogroup measurement
/// \{


//
// Chain code
//


/// \brief The contour of an object as a chain code sequence.
struct DIP_NO_EXPORT ChainCode {

   DIP_EXPORT static constexpr VertexInteger deltas4[4] = {{  1,  0 },
                                                           {  0, -1 },
                                                           { -1,  0 },
                                                           {  0,  1 }};
   DIP_EXPORT static constexpr VertexInteger deltas8[8] = {{  1,  0 },
                                                           {  1, -1 },
                                                           {  0, -1 },
                                                           { -1, -1 },
                                                           { -1,  0 },
                                                           { -1,  1 },
                                                           {  0,  1 },
                                                           {  1,  1 }};

   /// \brief Provides data that are helpful when processing chain codes.
   ///
   /// The table is prepared using the `dip::ChainCode::PrepareCodeTable` method. The method takes a stride array,
   /// which is expected to have exactly two elements (as chain codes only work with 2D images). The returned
   /// table contains a value `pos[code]` that says how the coordinates change when moving in the direction of the
   /// `code`, and a value `offset[code]` that says how to modify the image data pointer to reach the new pixel.
   ///
   /// `pos[code]` is identical to `code.Delta8()` or `code.Delta4()` (depending on connectivity).
   ///
   /// No checking is done when indexing. If the `%CodeTable` is derived from a 4-connected chain code, only the
   /// first four table elements can be used. Otherwise, eight table elements exist and are valid.
   struct DIP_NO_EXPORT CodeTable {
         VertexInteger const* pos; ///< Array with position offsets for each chain code.
         std::array< dip::sint, 8 > offset; ///< Array with pointer offsets for each chain code.
      private:
         friend struct ChainCode; // make it so that we can only create one of these tables through the dip::ChainCode::PrepareCodeTable method.
         CodeTable( bool is8connected, IntegerArray strides ) {
            dip::sint xS = strides[ 0 ];
            dip::sint yS = strides[ 1 ];
            pos = is8connected ? deltas8 : deltas4;
            for( dip::uint ii = 0; ii < ( is8connected ? 8u : 4u ); ++ii ) {
               offset[ ii ] = pos[ ii ].x * xS + pos[ ii ].y * yS;
            }
         }
   };

   /// \brief Encodes a single chain code, as used by `dip::ChainCode`. Chain codes are between 0 and 3 for connectivity = 1,
   /// and between 0 and 7 for connectivity = 2. The border flag marks pixels at the border of the image.
   class DIP_NO_EXPORT Code {
      public:
         /// Default constructor
         Code() = default;
         /// Constructor
         Code( unsigned code, bool border = false ) { value = static_cast< dip::uint8 >(( code & 7u ) | ( static_cast< unsigned >( border ) << 3u )); }
         /// Returns whether the border flag is set
         bool IsBorder() const { return static_cast< bool >( isBorder() ); }
         /// Returns the chain code
         operator unsigned() const { return code8(); }
         /// Is it an even code?
         bool IsEven() const { return !( value & 1u ); }
         /// Is it an off code?
         bool IsOdd() const { return !IsEven(); }
         /// The change in coordinates for an 8-connected chain code
         VertexInteger const& Delta8() const { return deltas8[ code8() ]; }
         /// The change in coordinates for a 4-connected chain code
         VertexInteger const& Delta4() const { return deltas4[ code4() ]; }
         /// Compare codes
         bool operator==( Code const& c2 ) const {
            return code8() == c2.code8();
         }
         /// Compare codes
         bool operator!=( Code const& c2 ) const {
            return !( *this == c2 );
         }
      private:
         dip::uint8 value = 0;
         unsigned code8() const { return value & 7u; }
         unsigned code4() const { return value & 3u; }
         bool isBorder() const { return static_cast< bool >( value & 8u ); }
   };

   std::vector< Code > codes;       ///< The chain codes
   VertexInteger start = { 0, 0 };  ///< The coordinates of the start pixel
   dip::uint objectID = 0;          ///< The label of the object from which this chain code is taken
   bool is8connected = true;        ///< Is false when connectivity = 1, true when connectivity = 2

   /// Adds a code to the end of the chain.
   void Push( Code const& code ) { codes.push_back( code ); }

   /// \brief Returns a table that is useful when processing the chain code
   CodeTable PrepareCodeTable( IntegerArray const& strides ) const {
      DIP_THROW_IF( strides.size() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
      return CodeTable( is8connected, strides );
   }

   /// \brief Returns a table that is useful when processing the chain code
   static CodeTable PrepareCodeTable( dip::uint connectivity, IntegerArray const& strides ) {
      DIP_THROW_IF( strides.size() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
      DIP_THROW_IF( connectivity > 2, E::CONNECTIVITY_NOT_SUPPORTED );
      return CodeTable( connectivity != 1, strides ); // 0 means 8-connected also
   }

   /// \brief Creates a new chain code object that is 8-connected and represents the same shape.
   DIP_EXPORT ChainCode ConvertTo8Connected() const;

   /// \brief Returns the length of the chain code using the method by Vossepoel and Smeulders.
   ///
   /// If the chain code represents the closed contour of an object, add &pi; to the result to determine
   /// the object's perimeter.
   ///
   /// Any portions of the chain code that run along the image edge are not measured. That is, for
   /// an object that is only partially inside the image, the portion of the object's perimeter that
   /// is inside of the image is measured, the edge created by cutting the object is not.
   ///
   /// \literature
   /// <li>A.M. Vossepoel and A.W.M. Smeulders, "Vector code probability and metrication error in the representation
   ///     of straight lines of finite length", Computer Graphics and %Image Processing 20(4):347-364, 1982.
   /// \endliterature
   DIP_EXPORT dfloat Length() const;

   /// \brief Returns the %Feret diameters, using an angular step size in radian of `angleStep`.
   /// It is better to use `dip::ConvexHull::Feret`.
   DIP_EXPORT FeretValues Feret( dfloat angleStep = 5.0 / 180.0 * pi ) const;

   /// Computes the bending energy.
   ///
   /// Computes the bending energy directly from the chain code. The algorithm is rather imprecise. It is better
   /// to use `dip::Polygon::BendingEnergy`.
   ///
   /// \literature
   /// <li>I.T. Young, J.E. Walker and J.E. Bowie, "An Analysis Technique for Biological Shape I",
   ///     Information and Control 25(4):357-370, 1974.
   /// <li>J.E. Bowie and I.T. Young, "An Analysis Technique for Biological Shape - II",
   ///     Acta Cytologica 21(5):455-464, 1977.
   /// \endliterature
   DIP_EXPORT dfloat BendingEnergy() const;

   /// \brief Computes the area of the solid object described by the chain code. Uses the result of
   /// `dip::ChainCode::Polygon`, so if you plan to do multiple similar measures, extract the polygon and
   /// compute the measures on that.
   dfloat Area() const {
      // There's another algorithm to compute this, that doesn't depend on the polygon. Should we implement that?
      return Polygon().Area() + 0.5;
   }

   /// \brief Computes the centroid of the solid object described by the chain code. Uses the result of
   /// `dip::ChainCode::Polygon`, so if you plan to do multiple similar measures, extract the polygon and
   /// compute the measures on that.
   VertexFloat Centroid() const {
      // There's another algorithm to compute this, that doesn't depend on the polygon. Should we implement that?
      return Polygon().Centroid();
   }

   /// \brief Finds the bounding box for the object described by the chain code
   DIP_EXPORT BoundingBoxInteger BoundingBox() const;

   /// Returns the length of the longest run of identical chain codes.
   DIP_EXPORT dip::uint LongestRun() const;

   /// \brief Returns a polygon representation of the object.
   ///
   /// Creates a polygon by joining the mid-points between an object pixel and a background pixel that are
   /// edge-connected neighbors. The polygon follows the "crack" between pixels, but without the biases
   /// one gets when joining pixel vertices into a polygon. The polygon always has an area exactly half a
   /// pixel smaller than the solid binary object it represents.
   ///
   /// This idea comes from Steve Eddins:
   /// http://blogs.mathworks.com/steve/2011/10/04/binary-image-convex-hull-algorithm-notes/
   DIP_EXPORT dip::Polygon Polygon() const;

   /// Returns the convex hull of the object, see `dip::ChainCode::Polygon`.
   dip::ConvexHull ConvexHull() const {
      return Polygon().ConvexHull();
   }

   /// \brief Paints the pixels traced by the chain code in a binary image. The image has the size of the
   /// `dip::ChainCode::BoundingBox`.
   DIP_EXPORT void Image( dip::Image& out ) const;
   dip::Image Image() const {
      dip::Image out;
      Image( out );
      return out;
   }

   /// \brief Create a new chain code that goes around the object in the same direction, but traces the background
   /// pixels that are 4-connected to the object. That is, it grows the object by one pixel. Only defined for
   /// 8-connected chain codes.
   DIP_EXPORT ChainCode Offset() const;
};

/// \brief A collection of object contours
/// \relates dip::ChainCode
using ChainCodeArray = std::vector< ChainCode >;

/// \brief Returns the set of chain codes sequences that encode the contours of the given objects in a labeled image.
///
/// Note that only the first closed contour for each label is found; if an object has multiple connected components,
/// only part of it is found. The chain code traces the outer perimeter of the object, holes are ignored.
///
/// `objectIDs` is a list with object IDs present in the labeled image. If an empty array is given, all objects in
/// the image are used.
ChainCodeArray DIP_EXPORT GetImageChainCodes(
      Image const& labels,                   ///< Labeled image, unsigned integer type
      UnsignedArray const& objectIDs = {},   ///< A list of object IDs to get chain codes for
      dip::uint connectivity = 2             ///< Connectivity, see \ref connectivity
);

/// \brief Returns the chain codes sequence that encodes the contour of one object in a binary or labeled image.
///
/// Note that only one closed contour is found; if the object has multiple connected components,
/// only part of it is found. The chain code traces the outer perimeter of the object, holes are ignored.
///
/// `startCoord` is the 2D coordinates of a boundary pixel. If it points to a zero-valued pixel or a pixel not on
/// the boundary of an object, an exception will be thrown.
ChainCode DIP_EXPORT GetSingleChainCode(
      Image const& labels,             ///< Labeled or binary image (unsigned integer type or binary type)
      UnsignedArray const& startCoord, ///< The starting coordinates for the chain code; must point to a non-zero pixel in `labels`
      dip::uint connectivity = 2       ///< Connectivity, see \ref connectivity
);

/// \}

} // namespace dip

#endif // DIP_CHAIN_CODE_H
