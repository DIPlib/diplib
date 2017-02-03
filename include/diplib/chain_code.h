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
/// \brief Declares the `dip::ChainCode` class, and releated functions.
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


/// \brief Encodes a location in a 2D image
template< typename T >
struct Vertex {
   T x;   ///< The x-coordinate
   T y;   ///< The y-coordinate

   /// Default constructor
   Vertex() : x( T( 0 ) ), y( T( 0 ) ) {}
   /// Constructor
   Vertex( T x, T y ) : x( x ), y( y ) {}

   /// Add a vertex
   template< typename V >
   Vertex& operator+=( Vertex< V > v ) {
      *this = *this + v;
      return *this;
   }
   /// Subtract a vertex
   template< typename V >
   Vertex& operator-=( Vertex< V > v ) {
      *this = *this - v;
      return *this;
   }
   /// Add a constant to both coordinate components
   Vertex& operator+=( T n ) {
      *this = *this + n;
      return *this;
   }
   /// Subtract a constant from both coordinate components
   Vertex& operator-=( T n ) {
      *this = *this - n;
      return *this;
   }
   /// Scale by a constant, isotropically
   Vertex& operator*=( dfloat n ) {
      *this = *this * n;
      return *this;
   }
   /// Add two vertices together
   template< typename T2 >
   Vertex< T > operator+( Vertex< T2 > const& v2 ) const {
      return Vertex<T>(x+v2.x, y+v2.y);
   }
   /// Subtract two vertices from each other
   template< typename T2 >
   Vertex< T > operator-( Vertex< T2 > const& v2 ) const {
      return Vertex<T>(x-v2.x, y-v2.y);
   }
   /// Add a vertex and a constant
   Vertex operator+( T n ) const {
      return Vertex<T>(x+n, y+n);
   }
   /// Subtract a vertex and a constant
   Vertex operator-( T n ) const {
      return Vertex<T>(x-n, y-n);
   }
   /// Multiply a vertex and a constant
   Vertex operator*( dfloat n ) const {
      return Vertex<T>(x*n, y*n);
   }

   /// Compare two vertices
   template< typename T2 >
   bool operator==( Vertex< T2 > const& v2 ) const {
      return ( x == v2.x ) && ( y == v2.y );
   }
   /// Compare two vertices
   template< typename T2 >
   bool operator!=( Vertex< T2 > const& v2 ) const {
      return !( *this == v2 );
   }
};

/// \brief The norm of the vector v2-v1.
template< typename T >
dfloat Distance( Vertex< T > const& v1, Vertex< T > const& v2 ) {
   Vertex< T > v = v2 - v1;
   return( std::hypot( v.x, v.y ));
}

/// \brief The square norm of the vector v2-v1.
template< typename T >
dfloat DistanceSquare( Vertex< T > const& v1, Vertex< T > const& v2 ) {
   Vertex< T > v = v2 - v1;
   return( v.x * v.x + v.y * v.y );
}

/// \brief The angle of the vector v2-v1.
template< typename T >
dfloat Angle( Vertex< T > const& v1, Vertex< T > const& v2 ) {
   Vertex< T > v = v2 - v1;
   return( std::atan2( v.y, v.x ));
}

/// \brief Compute the z component of the cross product of vectors v2-v1 and v3-v1
template< typename T >
dfloat ParallelogramSignedArea( Vertex< T > const& v1, Vertex< T > const& v2, Vertex< T > const& v3 ) {
   Vertex< T > vA = v2 - v1;
   Vertex< T > vB = v3 - v1;
   return ( vA.x * vB.y - vA.y * vB.x );
}

/// \brief Compute the area of the triangle formed by vertices v1, v2 and v3
template< typename T >
dfloat TriangleArea( Vertex< T > const& v1, Vertex< T > const& v2, Vertex< T > const& v3 ) {
   return ( std::abs( ParallelogramSignedArea< T >( v1, v2, v3 ) / 2.0 ) );
}

/// \brief Compute the height of the triangle formed by vertices v1, v2 and v3, with v3 the tip
template< typename T >
dfloat TriangleHeight( Vertex< T > const& v1, Vertex< T > const& v2, Vertex< T > const& v3 ) {
   return ( std::abs( ParallelogramSignedArea< T >( v1, v2, v3 ) / Distance< T >( v1, v2 ) ) );
}

using VertexFloat = Vertex< dfloat >;        ///< A vertex with floating-point coordinates
using VertexInteger = Vertex< dip::sint >;   ///< A vertex with integer coordinates

using Polygon = std::vector< VertexFloat >;  ///< A polygon with floating-point vertices

/// \brief A convex hull as a sequence of vertices (i.e. a closed polygon).
struct ConvexHull {
   Polygon vertices;  ///< The vertices

   /// Returns the area of the convex hull
   dfloat Area() const;

   /// Returns the perimeter of the convex hull
   dfloat Perimeter() const;

   /// Returns the Feret diameters of the convex hull
   FeretValues Feret() const;
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
   /// represents the closed contour of an object, add pi to the result.
   dfloat Length() const;

   /// \brief Returns the Feret diameters, using an angular step size in radian of `angleStep`.
   /// It is better to use `chainCode.ConvexHull().Feret()`.
   FeretValues Feret( dfloat angleStep ) const;

   /// \brief Computes the bending energy.
   dfloat BendingEnergy() const;

   /// Holds the various output values of the `dip::ChainCode::Radius` function.
   struct RadiusValues {
      dfloat max = 0.0;    ///< Maximum radius
      dfloat mean = 0.0;   ///< Mean radius
      dfloat min = 0.0;    ///< Minimum radius
      dfloat var = 0.0;    ///< Radius variance
   };
   /// \brief Returns statistics on the radii of the object. The radii are the distances between the centroid and
   /// each of the boundary pixels.
   RadiusValues Radius() const;

   /// Returns the length of the longest run of idenitcal chain codes.
   dip::uint LongestRun() const;

   /// Returns the convex hull of the object.
   dip::ConvexHull ConvexHull() const;
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
