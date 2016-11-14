/*
 * DIPlib 3.0
 * This file contains definitions for functionality related to the pixel size.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


//
// NOTE!
// This file is included through diplib.h -- no need to include directly
//


#ifndef DIP_PHYSDIMS_H
#define DIP_PHYSDIMS_H

#include <array>

#include "diplib/error.h"
#include "diplib/types.h"
#include "diplib/numeric.h"


/// \file
/// Defines support for units, physical quantities and pixel sizes.  This file is always included through diplib.h.


namespace dip {

/// Encapsulates the concepts of physical units, using SI units. It is possible
/// to multiply or divide units, and raise to arbitrary integer powers with the
/// class method Power. To associate a magnitude to the units, see the class
/// dip::PhysicalQuantity. Note that radian, though dimensionless, is treated
/// as a specific unit here. Also, mass is measured in grams, rather than kilograms,
/// because it simplifies writing prefixes (we presume the Kg won't be used much
/// in DIPlib...).
class Units {
      // Note: this class encapsulates units defined at run time, not at
      // compile time as in most C++ unit libraries:
      //   https://github.com/martinmoene/PhysUnits-CT-Cpp11
      //   http://www.boost.org/doc/libs/1_61_0/doc/html/boost_units.html
      // We need run-time unit management because usually we won't know the
      // units at compile time, and we want to be able to manage groups of
      // physical quantities with heterogeneous units.

   public:

      /// These are the base units for the SI system.
      enum class BaseUnits {
            LENGTH = 0,          ///< m
            MASS,                ///< g (should be Kg, but this is easier when working with prefixes.
            TIME,                ///< s
            CURRENT,             ///< A
            TEMPERATURE,         ///< K
            LUMINOUSINTENSITY,   ///< cd
            ANGLE,               ///< rad
      }; // NOTE: when adding or re-ordering these, take care to change things marked below.

      /// A default-constructed Units is dimensionless.
      Units() {};

      /// Construct a Units for a specific unit.
      Units( BaseUnits bu, dip::sint8 power = 1 ) { power_[ int( bu ) ] = power; };

      // Specific useful units
      /// Meter units (m)
      static Units Meter() { return dip::Units( dip::Units::BaseUnits::LENGTH ); }
      /// Square meter units (m^2)
      static Units SquareMeter() { return dip::Units( dip::Units::BaseUnits::LENGTH, 2 ); }
      /// Cubic meter units (m^3)
      static Units CubicMeter() { return dip::Units( dip::Units::BaseUnits::LENGTH, 3 ); }
      /// Second units (s)
      static Units Second() { return dip::Units( dip::Units::BaseUnits::TIME ); }
      /// Hertz units (s^-1)
      static Units Hertz() { return dip::Units( dip::Units::BaseUnits::TIME, -1 ); }
      /// Radian units (rad)
      static Units Radian() { return dip::Units( dip::Units::BaseUnits::ANGLE ); }

      /// Elevates `*this` to the power `p`.
      void Power( dip::sint8 power ) {
         for( auto& p : power_ ) {
            p = p * power;
         }
      }

      /// Multiplies two units objects.
      Units& operator*=( Units const& other ) {
         for( dip::uint ii = 0; ii < ndims_; ++ii ) {
            power_[ ii ] += other.power_[ ii ];
         }
         return *this;
      }

      /// Divides two units objects.
      Units& operator/=( Units const& other ) {
         for( dip::uint ii = 0; ii < ndims_; ++ii ) {
            power_[ ii ] -= other.power_[ ii ];
         }
         return *this;
      }

      /// Multiplies two units objects.
      friend Units operator*( Units lhs, Units const& rhs ) {
         lhs *= rhs;
         return lhs;
      }

      /// Divides two units objects.
      friend Units operator/( Units lhs, Units const& rhs ) {
         lhs /= rhs;
         return lhs;
      }

      /// Compares two units objects.
      friend bool operator==( Units const& lhs, Units const& rhs ) {
         for( dip::uint ii = 0; ii < ndims_; ++ii ) {
            if( lhs.power_[ ii ] != rhs.power_[ ii ] ) {
               return false;
            }
         }
         return true;
      }

      /// Compares two units objects.
      friend bool operator!=( Units const& lhs, Units const& rhs ) {
         return !( lhs == rhs );
      }

      /// Test to see if the units are dimensionless.
      bool IsDimensionless() const {
         for( auto& p : power_ ) {
            if( p != 0 ) {
               return false;
            }
         }
         return true;
      }

      /// Returns the power of the first unit to be written out, used to add an SI prefix to the unit.
      dip::sint FirstPower() const {
         for( auto& p : power_ ) {
            if( p > 0 ) {
               return p;
            }
         }
         for( auto& p : power_ ) {
            if( p != 0 ) {
               return p;
            }
         }
         return 0;
      }

      /// Insert physical quantity to an output stream as a string of base units;
      /// no attempty is (yet?) made to produce derived SI units or to translate
      /// to different units.
      friend std::ostream& operator<<( std::ostream& os, Units const& units ) {
         // NOTE: when changing BaseUnits in any way, adjusthere as necessary
         bool prefix = false;
         // We write out positive powers first
         prefix = WritePositivePower( os, "m", units.power_[ 0 ], prefix );
         prefix = WritePositivePower( os, "g", units.power_[ 1 ], prefix );
         prefix = WritePositivePower( os, "s", units.power_[ 2 ], prefix );
         prefix = WritePositivePower( os, "A", units.power_[ 3 ], prefix );
         prefix = WritePositivePower( os, "K", units.power_[ 4 ], prefix );
         prefix = WritePositivePower( os, "cd", units.power_[ 5 ], prefix );
         prefix = WritePositivePower( os, "rad", units.power_[ 6 ], prefix );
         // and negative powers at the end
         prefix = WriteNegativePower( os, "m", units.power_[ 0 ], prefix );
         prefix = WriteNegativePower( os, "g", units.power_[ 1 ], prefix );
         prefix = WriteNegativePower( os, "s", units.power_[ 2 ], prefix );
         prefix = WriteNegativePower( os, "A", units.power_[ 3 ], prefix );
         prefix = WriteNegativePower( os, "K", units.power_[ 4 ], prefix );
         prefix = WriteNegativePower( os, "cd", units.power_[ 5 ], prefix );
         prefix = WriteNegativePower( os, "rad", units.power_[ 6 ], prefix );
         return os;
      }

      /// Swaps the values of `*this` and `other`.
      void swap( Units& other ) {
         using std::swap;
         swap( power_, other.power_ );
      }

   private:

      // NOTE: ndims_ needs to be the number of elements in the BaseUnits enum.
      constexpr static dip::uint ndims_ = 7;
      std::array< sint8, ndims_ > power_ = { { 0, 0, 0, 0, 0, 0, 0 } };

      static bool WritePositivePower( std::ostream& os, const char* s, dip::sint8 p, bool prefix ) {
         if( p > 0 ) {
            if( prefix ) {
               os << "."; // TODO: output 'cdot' character?
            }
            os << s;
            if( p != 1 ) {
               os << "^" << ( int )p;
            }
            prefix = true;
         }
         return prefix;
      }
      static bool WriteNegativePower( std::ostream& os, const char* s, dip::sint8 p, bool prefix ) {
         if( p < 0 ) {
            if( prefix ) {
               os << "/";
               p = -p;
            }
            os << s;
            if( p != 1 ) {
               os << "^" << ( int )p;
            }
            prefix = true;
         }
         return prefix;
      }
};

inline void swap( Units& v1, Units& v2 ) {
   v1.swap( v2 );
}


/// Encapsulates a quantity with phyisical units. Multiplying a double by a
/// dip::Units object yields a PhysicalQuantity object. Numbers and units implicity
/// convert to a PhysicalQuantity. It is possible to multiply and divide any physical
/// quantities, but adding and subtracting is only possible if the units match.
///
/// ```
/// dip::PhysicalQuantity a = 50 * dip::Units( dip::Units::BaseUnits::LENGTH );
/// ```
struct PhysicalQuantity {

   /// A default-constructed PhysicalQuantity has magnitude 0 and is unitless.
   PhysicalQuantity() {};

   /// Create an arbitrary physical quantity.
   PhysicalQuantity( double m, Units const& u = {} ) : magnitude( m ), units( u ) {};

   /// Create a unit-valued physical quantity.
   PhysicalQuantity( Units const& u ) : magnitude( 1 ), units( u ) {};

   /// One nanometer.
   static PhysicalQuantity Nanometer() { return PhysicalQuantity( 1e-9, dip::Units::Meter() ); }
   /// One micrometer.
   static PhysicalQuantity Micrometer() { return PhysicalQuantity( 1e-6, dip::Units::Meter() ); }
   /// One millimeter.
   static PhysicalQuantity Millimeter() { return PhysicalQuantity( 1e-3, dip::Units::Meter() ); }
   /// One meter.
   static PhysicalQuantity Meter() { return PhysicalQuantity( 1, dip::Units::Meter() ); }
   /// One kilometer.
   static PhysicalQuantity Kilometer() { return PhysicalQuantity( 1e3, dip::Units::Meter() ); }
   /// One inch.
   static PhysicalQuantity Inch() { return PhysicalQuantity( 0.0254, dip::Units::Meter() ); }
   /// One mile.
   static PhysicalQuantity Mile() { return PhysicalQuantity( 1609.34, dip::Units::Meter() ); }
   /// One millisecond
   static PhysicalQuantity Milllisecond() { return PhysicalQuantity( 1e-3, dip::Units::Second() ); }
   /// One second
   static PhysicalQuantity Second() { return PhysicalQuantity( 1, dip::Units::Second() ); }
   /// One minute
   static PhysicalQuantity Minute() { return PhysicalQuantity( 60, dip::Units::Second() ); }
   /// One hour
   static PhysicalQuantity Hour() { return PhysicalQuantity( 3600, dip::Units::Second() ); }
   /// One day
   static PhysicalQuantity Day() { return PhysicalQuantity( 86400, dip::Units::Second() ); }
   /// One radian
   static PhysicalQuantity Radian() { return PhysicalQuantity( 1, dip::Units::Radian() ); }
   /// One degree
   static PhysicalQuantity Degree() { return PhysicalQuantity( pi / 180, dip::Units::Radian() ); }

   /// Multiplies two physical quantities.
   PhysicalQuantity& operator*=( PhysicalQuantity const& other ) {
      magnitude *= other.magnitude;
      units *= other.units;
      return *this;
   }
   /// Multiplies two physical quantities.
   friend PhysicalQuantity operator*( PhysicalQuantity lhs, PhysicalQuantity const& rhs ) {
      lhs *= rhs;
      return lhs;
   }
   /// Scaling of a physical quantity.
   PhysicalQuantity& operator*=( double other ) {
      magnitude *= other;
      return *this;
   }
   /// Scaling of a physical quantity.
   friend PhysicalQuantity operator*( PhysicalQuantity lhs, double rhs ) {
      lhs *= rhs;
      return lhs;
   }
   /// Scaling of a physical quantity.
   friend PhysicalQuantity operator*( double lhs, PhysicalQuantity rhs ) {
      rhs *= lhs;
      return rhs;
   }

   /// Divides two physical quantities.
   PhysicalQuantity& operator/=( PhysicalQuantity const& other ) {
      magnitude /= other.magnitude;
      units /= other.units;
      return *this;
   }
   /// Divides two physical quantities.
   friend PhysicalQuantity operator/( PhysicalQuantity lhs, PhysicalQuantity const& rhs ) {
      lhs /= rhs;
      return lhs;
   }
   /// Scaling of a physical quantity.
   PhysicalQuantity& operator/=( double other ) {
      magnitude /= other;
      return *this;
   }
   /// Scaling of a physical quantity.
   friend PhysicalQuantity operator/( PhysicalQuantity lhs, double rhs ) {
      lhs /= rhs;
      return lhs;
   }
   /// Scaling of a physical quantity.
   friend PhysicalQuantity operator/( double lhs, PhysicalQuantity rhs ) {
      rhs.Power( -1 );
      rhs *= lhs;
      return rhs;
   }

   /// Computes a physical quantity to the power of `p`.
   void Power( dip::sint8 p ) {
      magnitude = std::pow( magnitude, p );
      units.Power( p );
   }

   /// Unary negation.
   friend PhysicalQuantity operator-( PhysicalQuantity pq ) {
      pq.magnitude = -pq.magnitude;
      return pq;
   }

   /// Addition of two physical quantities.
   PhysicalQuantity& operator+=( PhysicalQuantity const& other ) {
      dip_ThrowIf( units != other.units, "Units don't match" );
      magnitude += other.magnitude;
      return *this;
   }
   /// Addition of two physical quantities.
   friend PhysicalQuantity operator+( PhysicalQuantity lhs, PhysicalQuantity const& rhs ) {
      lhs += rhs;
      return lhs;
   }

   /// Subtraction of two physical quantities.
   PhysicalQuantity& operator-=( PhysicalQuantity const& other ) {
      dip_ThrowIf( units != other.units, "Units don't match" );
      magnitude -= other.magnitude;
      return *this;
   }
   /// Subtraction of two physical quantities.
   friend PhysicalQuantity operator-( PhysicalQuantity lhs, PhysicalQuantity const& rhs ) {
      lhs -= rhs;
      return lhs;
   }

   /// Comparison of two physical quantities.
   friend bool operator==( PhysicalQuantity const& lhs, PhysicalQuantity const& rhs ) {
      return ( lhs.magnitude == rhs.magnitude ) && ( lhs.units == rhs.units );
   }

   /// Comparison of two physical quantities.
   friend bool operator!=( PhysicalQuantity const& lhs, PhysicalQuantity const& rhs ) {
      return !( lhs == rhs );
   }

   /// Test to see if the physical quantity is dimensionless.
   bool IsDimensionless() const {
      return units.IsDimensionless();
   }

   /// Insert physical quantity to an output stream.
   friend std::ostream& operator<<( std::ostream& os, PhysicalQuantity const& pq ) {
      double magnitude = pq.magnitude;
      dip::sint p = pq.units.FirstPower();
      if( p == 0 ) {
         // Dimensionless quantity
         os << magnitude;
      } else {
         double nzeros = std::floor( std::log10( pq.magnitude ) );
         nzeros = std::round( nzeros / p / 3 - 0.1 ) *
                  3; // Using round here, with a small decrement, so that we get values [0.1,100) for ^1 and [0.01,10000) for ^2.
         nzeros = dip::clamp( nzeros, -15.0, 18.0 );
         magnitude /= std::pow( 10.0, nzeros * p );
         os << magnitude << " ";
         if( nzeros != 0 ) {
            const char* prefixes = "fpnum kMGTPE";
            os << prefixes[ dip::sint( nzeros ) / 3 + 5 ];
         }
         os << pq.units;
      }
      return os;
   }

   /// Retrieve the magnitude, discaring units.
   explicit operator double() const { return magnitude; };

   /// Retrieve the magnitude, discaring units.
   explicit operator bool() const { return static_cast< bool >( magnitude ); };

   /// Swaps the values of `*this` and `other`.
   void swap( PhysicalQuantity& other ) {
      using std::swap;
      swap( magnitude, other.magnitude );
      swap( units, other.units );
   }

   double magnitude = 0; ///< The magnitude
   Units units;   ///< The units
};

inline void swap( PhysicalQuantity& v1, PhysicalQuantity& v2 ) {
   v1.swap( v2 );
}

/// An array to hold physical quantities, such as a pixel's size.
using PhysicalQuantityArray = DimensionArray< PhysicalQuantity >;

/// Create an arbitrary physical quantity by multiplying a magnitude with units.
inline PhysicalQuantity operator*( double lhs, Units const& rhs ) {
   return PhysicalQuantity( lhs, rhs );
}

/// Specifies an image's pixel size as physical quantities. The object works like an
/// array with unlimited number of elements. It is possible to set only one value, and
/// that value will be used for all dimensions. In general, if *N* dimensions
/// are set (i.e. the array has *N* elements defined), then dimensions *N* and further
/// have the same value as dimension *N-1*.
///
/// When setting dimension *N-1*, all further dimensions are affected. When setting
/// dimension *N+K*, the new array size will be *N+K+1*. Dimensions *N* through *N+K-1*
/// are assigned the same value as dimension *N-1*, then dimension *N+K* will be assigned
/// the new value, and all subsequent dimensions will implicitly have the same value.
///
/// Thus, it is important to know how many elements are set in the array to know
/// how any modifications will affect it.
///
/// However, SwapDimensions, InsertDimension and EraseDimension will expand the array
/// by one element before modifying the last element in the array. This prevents the
/// implicit elements after the defined ones to be modified. For example, inserting
/// dimension *N+K* first expands the array to size *N+K+2* by setting all the new
/// elements to the same value as element *N-1*, then sets a new value for dimension
/// *N+K*. Dimension *N+K+1* now still has the same value as before (though now it is
/// explicitly defined, whereas before it was implicitly defined).
///
/// The pixel size always needs a unit. Any dimensionless quantity is interpreted
/// as 1, and considered as an "undefined" size. Angles, measured in radian, are
/// not considered dimensionless, though they actually are (see dip::Units).
class PixelSize {

   public:

      /// By default, an image has no physical dimensions. The pixel size is given
      /// as "1 pixel".
      PixelSize() {};

      /// Create an isotropic pixel size based on a physical quantity.
      PixelSize( PhysicalQuantity const& m ) : size_{ m } {};

      /// Create a pixel size based on an array of physical quantities.
      PixelSize( PhysicalQuantityArray const& m ) : size_{ m } {};

      /// Returns the pixel size for the given dimension.
      PhysicalQuantity Get( dip::uint d ) const {
         if( size_.empty() ) {
            return PhysicalQuantity( 1 );
         } else if( d >= size_.size() ) {
            return size_.back();
         } else {
            return size_[ d ];
         }
      }
      /// Returns the pixel size for the given dimension.
      PhysicalQuantity operator[]( dip::uint d ) const {
         return Get( d );
      }

      /// Sets the pixel size in the given dimension. Note that
      /// any subsequent dimension, if not explicitly set, will have the same
      /// size.
      void Set( dip::uint d, PhysicalQuantity const& m ) {
         if( Get( d ) != m ) {
            EnsureDimensionality( d + 1 );
            size_[ d ] = m;
         }
      }

      /// Sets the isotropic pixel size in all dimensions.
      void Set( PhysicalQuantity const& m ) {
         size_.resize( 1 );
         size_[ 0 ] = m;
      }

      /// Sets the pixel size in the given dimension, in nanometers.
      void SetNanometers( dip::uint d, double m ) {
         Set( d, m * PhysicalQuantity::Nanometer() );
      }
      /// Sets the isotropic pixel size, in nanometers.
      void SetNanometers( double m ) {
         Set( m * PhysicalQuantity::Nanometer() );
      }
      /// Sets the pixel size in the given dimension, in micrometers.
      void SetMicrometers( dip::uint d, double m ) {
         Set( d, m * PhysicalQuantity::Micrometer() );
      }
      /// Sets the isotropic pixel size, in micrometers.
      void SetMicrometers( double m ) {
         Set( m * PhysicalQuantity::Micrometer() );
      }
      /// Sets the pixel size in the given dimension, in millimeters.
      void SetMillimeters( dip::uint d, double m ) {
         Set( d, m * PhysicalQuantity::Millimeter() );
      }
      /// Sets the isotropic pixel size, in millimeters.
      void SetMillimeters( double m ) {
         Set( m * PhysicalQuantity::Millimeter() );
      }
      /// Sets the pixel size in the given dimension, in meters.
      void SetMeters( dip::uint d, double m ) {
         Set( d, m * PhysicalQuantity::Meter() );
      }
      /// Sets the isotropic pixel size, in meters.
      void SetMeters( double m ) {
         Set( m * PhysicalQuantity::Meter() );
      }
      /// Sets the pixel size in the given dimension, in kilometers.
      void SetKilometers( dip::uint d, double m ) {
         Set( d, m * PhysicalQuantity::Kilometer() );
      }
      /// Sets the isotropic pixel size, in kilometers.
      void SetKilometers( double m ) {
         Set( m * PhysicalQuantity::Kilometer() );
      }

      /// Sets a non-isotropic pixel size.
      void Set( PhysicalQuantityArray const& m ) {
         size_ = m;
      }

      /// Scales the pixel size in the given dimension, if it is defined.
      void Scale( dip::uint d, double s ) {
         if( ( !size_.empty() ) && !Get( d ).IsDimensionless() ) {
            // we add a dimension past `d` here so that, if they were meaningful, dimensions d+1 and further don't change value.
            EnsureDimensionality( d + 2 );
            size_[ d ] *= s;
         }
      }

      /// Scales the pixel size isotropically in the given dimension, if it is defined.
      void Scale( double s ) {
         for( dip::uint ii = 0; ii < size_.size(); ++ii ) {
            if( !size_[ ii ].IsDimensionless() ) {
               size_[ ii ] *= s;
            }
         }
      }

      /// Scales the pixel size non-isotropically in all dimensions, where defined.
      void Scale( FloatArray const& s ) {
         if( !size_.empty() ) {
            // we do not add a dimension past `d` here, assuming that the caller is modifying all useful dimensions.
            EnsureDimensionality( s.size() );
            for( dip::uint ii = 1; ii < s.size(); ++ii ) {
               if( !size_[ ii ].IsDimensionless() ) {
                  size_[ ii ] *= s[ ii ];
               }
            }
         }
      }

      /// Swaps two dimensions.
      void SwapDimensions( dip::uint d1, dip::uint d2 ) {
         using std::swap;
         if( !size_.empty() && ( Get( d1 ) != Get( d2 ) ) ) {
            // we add a dimension past `d` here so that, if they were meaningful, dimensions d+1 and further don't change value.
            EnsureDimensionality( std::max( d1, d2 ) + 2 );
            swap( size_[ d1 ], size_[ d2 ] );
         }
      }

      /// Inserts a dimension, undefined by default.
      void InsertDimension( dip::uint d, PhysicalQuantity const& m = 1 ) {
         if( !m.IsDimensionless() || IsDefined() ) {
            // we add a dimension past `d` here so that, if they were meaningful, dimensions d+1 and further don't change value.
            EnsureDimensionality( d + 1 );
            size_.insert( d, m );
         } // else we don't need to do anything: the pixel is undefined and we add a dimensionless quantity.
      }

      /// Erases a dimension
      void EraseDimension( dip::uint d ) {
         // we don't erase the last element in the array, since that would change all subsequent elemtns too.
         if( d + 1 < size_.size() ) {
            size_.erase( d );
         }
      }

      /// Clears the pixel sizes, reverting to the default undefined state.
      void Clear() {
         size_.clear();
      }

      /// Returns the number of dimensions stored.
      dip::uint Size() const {
         return size_.size();
      }

      /// Removes stored dimensions, keeping the first `d` dimensions only.
      void Resize( dip::uint d ) {
         if( d < size_.size() ) {
            size_.resize( d );
         }
      }

      /// Tests the pixel size for isotropy (the pixel has the same size in all dimensions).
      bool IsIsotropic() const {
         for( dip::uint ii = 1; ii < size_.size(); ++ii ) {
            if( size_[ ii ] != size_[ 0 ] ) {
               return false;
            }
         }
         return true;
      }

      /// Tests to see if the pixel size is defined.
      bool IsDefined() const {
         for( dip::uint ii = 0; ii < size_.size(); ++ii ) {
            if( !size_[ ii ].IsDimensionless() ) {
               return true;
            }
         }
         return false;
      }

      /// Multiplies together the sizes for the first `d` dimensions.
      PhysicalQuantity Product( dip::uint d ) const {
         if( d == 0 ) {
            return PhysicalQuantity( 1 );
         }
         PhysicalQuantity out = Get( 0 );
         if( out.IsDimensionless() ) {
            out = 1;
         }
         for( dip::uint ii = 1; ii < d; ++ii ) {
            PhysicalQuantity v = Get( ii );
            if( !v.IsDimensionless() ) {
               out = out * Get( ii );
            }
         }
         return out;
      }

      /// Compares two pixel sizes
      friend bool operator==( PixelSize const& lhs, PixelSize const& rhs ) {
         dip::uint d = std::max( lhs.size_.size(), rhs.size_.size() );
         for( dip::uint ii = 0; ii < d; ++ii ) {
            if( lhs.Get( ii ) != rhs.Get( ii ) ) {
               return false;
            }
         }
         return true;
      }

      /// Compares two pixel sizes
      friend bool operator!=( PixelSize const& lhs, PixelSize const& rhs ) {
         return !( lhs == rhs );
      }

      /// Converts physical units to pixels.
      FloatArray ToPixels( PhysicalQuantityArray const& in ) const {
         FloatArray out( in.size() );
         for( dip::uint ii = 0; ii < in.size(); ++ii ) {
            PhysicalQuantity v = Get( ii );
            dip_ThrowIf( in[ ii ].units != v.units, "Units don't match" );
            out[ ii ] = in[ ii ].magnitude / v.magnitude;
         }
         return out;
      }

      /// Converts pixels to meters.
      PhysicalQuantityArray ToPhysical( FloatArray const& in ) const {
         PhysicalQuantityArray out( in.size() );
         for( dip::uint ii = 0; ii < in.size(); ++ii ) {
            out[ ii ] = PhysicalQuantity( in[ ii ] ) * Get( ii );
         }
         return out;
      }

      /// Swaps the values of `*this` and `other`.
      void swap( PixelSize& other ) {
         using std::swap;
         swap( size_, other.size_ );
      }

   private:
      // The array below stores a series of values. If the image has more dimensions
      // that this array, the last element is presumed repeated across non-defined
      // dimensions. This is useful because many images have isotropic pixels, and
      // therefore need to store only one value.
      PhysicalQuantityArray size_;

      // Adds dimensions to the `size_` array, if necessary, such that there are
      // at least `d` dimensions. The last element is repeated if the array is
      // extended.
      void EnsureDimensionality( dip::uint d ) {
         if( size_.empty() ) {
            size_.resize( d, 1 );
         } else if( size_.size() < d ) {
            size_.resize( d, size_.back() );
         }
      }

};

inline void swap( PixelSize& v1, PixelSize& v2 ) {
   v1.swap( v2 );
}

} // namespace dip

#endif // DIP_PHYSDIMS_H
