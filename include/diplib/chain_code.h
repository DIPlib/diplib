/*
 * DIPlib 3.0
 * This file contains declarations for measurement-related classes
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIP_CHAIN_CODE_H
#define DIP_CHAIN_CODE_H

#include <vector>
#include <cmath>

#include "diplib.h"


/// \file
/// \brief Declares `dip::ChainCode`, `dip::Polygon`, `dip::ConvexHull`, and related functions.
/// \see measurement


namespace dip {


/// \addtogroup infrastructure
/// \{


/// \brief Contains the various Feret diameters as returned by `dip::ConvexHull::Feret` and `dip::ChainCode::Feret`.
struct FeretValues {
   dfloat maxDiameter = 0.0;        ///< The maximum Feret diameter
   dfloat minDiameter = 0.0;        ///< The minimum Feret diameter
   dfloat maxPerpendicular = 0.0;   ///< The Feret diameter perpendicular to `minDiameter`
   dfloat maxAngle = 0.0;           ///< The angle at which `maxDiameter` was measured
   dfloat minAngle = 0.0;           ///< The angle at which `minDiameter` was measured
};

/// Holds the various output values of the `dip::RadiusStatistics` and `dip::ChainCode::RadiusStatistics` function.
struct RadiusValues {
   dfloat max = 0.0;    ///< Maximum radius
   dfloat mean = 0.0;   ///< Mean radius
   dfloat min = 0.0;    ///< Minimum radius
   dfloat var = 0.0;    ///< Radius variance
};

/// \brief Encodes a location in a 2D image
template< typename T >
struct Vertex {
   T x;   ///< The x-coordinate
   T y;   ///< The y-coordinate

   /// Default constructor
   Vertex() : x( T( 0 )), y( T( 0 )) {}
   /// Constructor
   Vertex( T x, T y ) : x( x ), y( y ) {}

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
   /// Scale by a inverse of a constant, isotropically
   Vertex& operator/=( dfloat n ) {
      x = T( dfloat( x ) / n );
      y = T( dfloat( y ) / n );
      return *this;
   }
   /// Add two vertices together
   template< typename T1, typename T2 >
   friend Vertex< T1 > operator+( Vertex< T1 > v1, Vertex< T2 > const& v2 ) {
      v1 += v2;
      return v1;
   }
   /// Subtract two vertices from each other
   template< typename T1, typename T2 >
   friend Vertex< T1 > operator-( Vertex< T1 > v1, Vertex< T2 > const& v2 ) {
      v1 -= v2;
      return v1;
   }
   /// Add a vertex and a constant
   friend Vertex operator+( Vertex v, T n ) {
      v += n;
      return v;
   }
   /// Subtract a vertex and a constant
   friend Vertex operator-( Vertex v, T n ) {
      v -= n;
      return v;
   }
   /// Multiply a vertex and a constant
   friend Vertex operator*( Vertex v, dfloat n ) {
      v *= n;
      return v;
   }
   /// Divide a vertex and a constant
   friend Vertex operator/( Vertex v, dfloat n ) {
      v /= n;
      return v;
   }

   /// Compare two vertices
   friend bool operator==( Vertex v1, Vertex v2 ) {
      return ( v1.x == v2.x ) && ( v1.y == v2.y );
   }
   /// Compare two vertices
   friend bool operator!=( Vertex v1, Vertex v2 ) {
      return !( v1 == v2 );
   }
};

/// \brief The norm of the vector v2-v1.
template< typename T >
dfloat Distance( Vertex< T > const& v1, Vertex< T > const& v2 ) {
   Vertex< T > v = v2 - v1;
   return std::hypot( v.x, v.y );
}

/// \brief The square norm of the vector v2-v1.
template< typename T >
dfloat DistanceSquare( Vertex< T > const& v1, Vertex< T > const& v2 ) {
   Vertex< T > v = v2 - v1;
   return v.x * v.x + v.y * v.y;
}

/// \brief The angle of the vector v2-v1.
template< typename T >
dfloat Angle( Vertex< T > const& v1, Vertex< T > const& v2 ) {
   Vertex< T > v = v2 - v1;
   return std::atan2( v.y, v.x );
}

/// \brief Compute the z component of the cross product of vectors v1 and v2
template< typename T >
dfloat CrossProduct( Vertex< T > const& v1, Vertex< T > const& v2 ) {
   return v1.x * v2.y - v1.y * v2.x;
}

/// \brief Compute the z component of the cross product of vectors v2-v1 and v3-v1
template< typename T >
dfloat ParallelogramSignedArea( Vertex< T > const& v1, Vertex< T > const& v2, Vertex< T > const& v3 ) {
   return CrossProduct( v2 - v1, v3 - v1 );
}

/// \brief Compute the area of the triangle formed by vertices v1, v2 and v3
template< typename T >
dfloat TriangleArea( Vertex< T > const& v1, Vertex< T > const& v2, Vertex< T > const& v3 ) {
   return std::abs( ParallelogramSignedArea< T >( v1, v2, v3 ) / 2.0 );
}

/// \brief Compute the height of the triangle formed by vertices v1, v2 and v3, with v3 the tip
template< typename T >
dfloat TriangleHeight( Vertex< T > const& v1, Vertex< T > const& v2, Vertex< T > const& v3 ) {
   return std::abs( ParallelogramSignedArea< T >( v1, v2, v3 ) / Distance< T >( v1, v2 ));
}

using VertexFloat = Vertex< dfloat >;        ///< A vertex with floating-point coordinates
using VertexInteger = Vertex< dip::sint >;   ///< A vertex with integer coordinates

/// \brief A polygon with floating-point vertices.
struct Polygon {

   std::vector< VertexFloat > vertices;  ///< The vertices

   /// \brief Computes the area of a polygon
   dfloat Area() const {
      if( vertices.size() < 3 ) {
         return 0; // Should we generate an error instead?
      }
      dfloat sum = CrossProduct( vertices.back(), vertices[ 0 ] );
      for( dip::uint ii = 1; ii < vertices.size(); ++ii ) {
         sum += CrossProduct( vertices[ ii - 1 ], vertices[ ii ] );
      }
      return sum / 2.0;
   }

   /// \brief Computes the centroid of a polygon
   VertexFloat Centroid() const {
      if( vertices.size() < 3 ) {
         return { 0, 0 }; // Should we generate an error instead?
      }
      dfloat v = CrossProduct( vertices.back(), vertices[ 0 ] );
      dfloat sum = v;
      dfloat xsum = vertices.back().x + vertices[ 0 ].x;
      dfloat ysum = vertices.back().y + vertices[ 0 ].y;
      for( dip::uint ii = 1; ii < vertices.size(); ++ii ) {
         v = CrossProduct( vertices[ ii - 1 ], vertices[ ii ] );
         sum += v;
         xsum += ( vertices[ ii - 1 ].x + vertices[ ii ].x ) * v;
         ysum += ( vertices[ ii - 1 ].y + vertices[ ii ].y ) * v;
      }
      return VertexFloat{ xsum, ysum } / ( 3 * sum );
   }

   /// \brief Computes the lenght of a vertices (i.e. perimeter). If the vertices represents a pixelated object,
   /// this function will overestimate the object's perimeter. Use `dip::ChainCode::Length` instead.
   dfloat Length() const {
      if( vertices.size() < 2 ) {
         return 0; // Should we generate an error instead?
      }
      dfloat sum = Distance( vertices.back(), vertices[ 0 ] );
      for( dip::uint ii = 1; ii < vertices.size(); ++ii ) {
         sum += Distance( vertices[ ii - 1 ], vertices[ ii ] );
      }
      return sum;
   }

   /// \brief Returns statistics on the radii of the poligon. The radii are the distances between the centroid and
   /// each of the vertices.
   RadiusValues RadiusStatistics() const;
};

/// \brief A convex hull as a sequence of vertices (i.e. a closed polygon).
class ConvexHull {
   public:

      /// Default-constructed ConvexHull (without vertices)
      ConvexHull() {};

      /// Constructs a convex hull of a polygon
      ConvexHull( Polygon const&& polygon );

      /// Retrive the vertices that represent the convex hull
      std::vector< VertexFloat > const& Vertices() const {
         return vertices_.vertices;
      }

      /// Returns the area of the convex hull
      dfloat Area() const {
         return vertices_.Area();
      }

      /// Returns the perimeter of the convex hull
      dfloat Perimeter() const {
         return vertices_.Length();
      }

      /// Returns the Feret diameters of the convex hull
      FeretValues Feret() const;

      /// Returns the centroid of the convex hull
      VertexFloat Centroid() const {
         return vertices_.Centroid();
      }

      /// Converts the `%ConvexHull` object to a `dip::Polygon`
      operator Polygon() const {
         return vertices_;
      }

   private:
      Polygon vertices_;
};


/// \brief The contour of an object as a chain code sequence.
struct ChainCode {

   /// \brief Encodes a single chain code, as used by `dip::ChainCode`. Chain codes are between 0 and 3 for connectivity = 1,
   /// and between 0 and 7 for connectivity = 2. The border flag marks pixels at the border of the image.
   class Code {
      public:
         /// Default constructor
         Code() : value( 0 ) {}
         /// Constructor
         Code( int code, bool border = false ) { value = static_cast< dip::uint8 >( code & 7 ) | (( static_cast< dip::uint8 >( border )) << 3 ); }
         /// Returns whether the border flag is set
         bool IsBorder() const { return static_cast< bool >( value & 8 ); }
         /// Returns the chain code
         operator int() const { return value & 7; }
         /// Is it an even code?
         bool IsEven() const { return !(value & 1); }
         /// Is it an off code?
         bool IsOdd() const { return !IsEven(); }
         /// Compare codes
         friend bool operator==( Code const& c1, Code const& c2 ) {
            return ( c1.value & 7 ) == ( c2.value & 7 );
         }
         /// Compare codes
         friend bool operator!=( Code const& c1, Code const& c2 ) {
            return !( c1 == c2 );
         }
      private:
         dip::uint8 value;
   };

   std::vector< Code > codes;  ///< The chain codes
   Vertex< dip::sint > start = { 0, 0 };         ///< The coordinates of the start pixel
   dip::uint objectID;              ///< The label of the object from which this chaincode is taken
   bool is8connected = true;        ///< Is false when connectivity = 1, true when connectivity = 2

   /// Adds a code to the end of the chain.
   void Push( Code const& code ) { codes.push_back( code ); }

   /// \brief Returns the length of the chain code using the method by Vossepoel and Smeulders. If the chain code
   /// represents the closed contour of an object, add pi to the result to determine the object's perimeter.
   dfloat Length() const;

   /// \brief Returns the Feret diameters, using an angular step size in radian of `angleStep`.
   /// It is better to use `chainCode.ConvexHull().Feret()`.
   FeretValues Feret( dfloat angleStep ) const;

   /// Computes the bending energy.
   dfloat BendingEnergy() const;

   /// \brief Computes the area of the solid object described by the chain code. Uses the result of
   /// `dip::ChainCode::Polygon`, so if you plan to do multiple similar measures, extract the polygon and
   /// compute the measures on that.
   dfloat Area() const {
      return Polygon().Area();
   }

   /// \brief Computes the centroid of the solid object described by the chain code. Uses the result of
   /// `dip::ChainCode::Polygon`, so if you plan to do multiple similar measures, extract the polygon and
   /// compute the measures on that.
   VertexFloat Centroid() const {
      return Polygon().Centroid();
   }

   /// \brief Returns statistics on the radii of the object.
   ///
   /// The radii are the distances between the centroid and each of the boundary mid-points. These
   /// are the mid-points between an object pixel and a background pixel that are edge-connected neighbors.
   /// See `dip::ChainCode::Polygon` for a more detailed explanation.
   ///
   /// Uses the result of `dip::ChainCode::Polygon`, so if you plan to do multiple similar measures, extract the
   /// polygon and compute the measures on that.
   RadiusValues RadiusStatistics() const {
      return Polygon().RadiusStatistics();
   }

   /// Returns the length of the longest run of idenitcal chain codes.
   dip::uint LongestRun() const;

   /// \brief Returns a polygon representation of the object.
   ///
   /// Creates a polygon by joining the mid-points between an object pixel and a background pixel that are
   /// edge-connected neighbors. The polygon follows the "crack" between pixels, but without the biases
   /// one gets when joining pixel vertices into a polygon.
   ///
   /// This idea comes from Steve Eddins:
   /// http://blogs.mathworks.com/steve/2011/10/04/binary-image-convex-hull-algorithm-notes/
   dip::Polygon Polygon() const;

   /// Returns the convex hull of the object, see `dip::ChainCode::Polygon`.
   dip::ConvexHull ConvexHull() const {
      return dip::ConvexHull( Polygon() );
   }
};

/// \brief A collection of object contours
using ChainCodeArray = std::vector< ChainCode >;

/// \brief Returns the set of chain codes sequences that encode the countours of the given objects in a labelled image.
/// Note that only the first closed contour for each label is found; if an object has multiple connected components,
/// only part of the object is found.
ChainCodeArray GetImageChainCodes(
      Image const& labels,
      UnsignedArray const& objectIDs,
      dip::uint connectivity = 2
);

/// \}

} // namespace dip

#endif // DIP_CHAIN_CODE_H
