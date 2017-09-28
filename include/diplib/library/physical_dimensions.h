/*
 * DIPlib 3.0
 * This file contains definitions for functionality related to the pixel size.
 *
 * (c)2016-2017, Cris Luengo.
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


//
// NOTE!
// This file is included through diplib.h -- no need to include directly
//


#ifndef DIP_PHYSICAL_DIMENSIONS_H
#define DIP_PHYSICAL_DIMENSIONS_H

#include <array>

#include "diplib/library/clamp_cast.h"


/// \file
/// \brief Support for units, physical quantities and pixel sizes.
/// This file is always included through `diplib.h`.
/// \see infrastructure


namespace dip {


/// \defgroup physical_dimensions Physical dimensions
/// \brief Support for units, physical quantities and pixel sizes.
/// \ingroup infrastructure
/// \{


/// \brief Encapsulates the concepts of physical units, using SI units.
///
/// It is possible
/// to multiply or divide units, and raise to arbitrary integer powers with the
/// class method Power. To associate a magnitude to the units, see the class
/// `dip::PhysicalQuantity`.
///
/// Note that radian (`BaseUnits::ANGLE`), though dimensionless, is treated
/// as a specific unit here. Also, mass is measured in grams, rather than kilograms,
/// because it simplifies writing prefixes (we presume the Kg won't be used much
/// in *DIPlib*...).
///
/// Prefixes are recorded with the `BaseUnits::THOUSANDS` value. It indicates how
/// often to multiply by 10^3. Thus, a value of 1 here corresponds to the 'k'
/// prefix, 3 with 'G', and -2 with 'u' (==micro). Note that for 'mm^2', the
/// value for length is 2, and that for thousands is -2. if thousands were -1,
/// the units would have to be formatted as '10^-3.m^2'. `dip::Units::AdjustThousands`
/// adjusts this power so that it can always be formatted with an SI prefix,
/// returning a magnitude that can be handled elsewhere (the `dip::PhysicalQuantity`
/// class uses this feature).
///
/// The `BaseUnits::PIXEL` value is for non-physical quantities, which typically
/// represent a magnitude with unknown or arbitrary units. For example,
/// `dip::MeasurementTool` uses it when an image has no pixel size.
/// `IsPhysical` tests whether there are pixel units present or not.
class DIP_NO_EXPORT Units {
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
            THOUSANDS = 0,       ///< prefix
            // NOTE: THOUSANDS must be the first element here and have a value of 0.
            LENGTH,              ///< m
            MASS,                ///< g (should be Kg, but this is easier when working with prefixes)
            TIME,                ///< s
            CURRENT,             ///< A
            TEMPERATURE,         ///< K
            LUMINOUSINTENSITY,   ///< cd
            ANGLE,               ///< rad (though really dimensionless)
            PIXEL,               ///< px (units to use when the image has no dimension information)
            // NOTE: PIXEL is assumed to be the last element here, and is used to determine ndims_.
      };


      /// A default-constructed `%Units` is dimensionless.
      Units() {
         power_.fill( 0 );
      }

      /// Construct a `%Units` for a specific unit.
      explicit Units( BaseUnits bu, dip::sint8 power = 1 ) {
         power_.fill( 0 );
         power_[ static_cast< dip::uint >( bu ) ] = power;
      }

      /// \brief Construct a `%Units` from a string representation of units. The string representation should be as
      /// produced by `dip::Units::String`.
      DIP_EXPORT explicit Units( dip::String const& string );

      // Specific useful powers
      /// Dimensionless nano magnitude (n)
      static Units Nano() { return Units( BaseUnits::THOUSANDS, -3 ); }
      /// Dimensionless micro magnitude (u)
      static Units Micro() { return Units( BaseUnits::THOUSANDS, -2 ); }
      /// Dimensionless milli magnitude (m)
      static Units Milli() { return Units( BaseUnits::THOUSANDS, -1 ); }
      /// Dimensionless kilo magnitude (k)
      static Units Kilo() { return Units( BaseUnits::THOUSANDS, 1 ); }
      /// Dimensionless mega magnitude (M)
      static Units Mega() { return Units( BaseUnits::THOUSANDS, 2 ); }
      /// Dimensionless giga magnitude (G)
      static Units Giga() { return Units( BaseUnits::THOUSANDS, 3 ); }

      // Specific useful units
      /// Meter units (m)
      static Units Meter() { return Units( BaseUnits::LENGTH ); }
      /// Square meter units (m^2)
      static Units SquareMeter() { return Units( BaseUnits::LENGTH, 2 ); }
      /// Cubic meter units (m^3)
      static Units CubicMeter() { return Units( BaseUnits::LENGTH, 3 ); }
      /// Nanometer units (nm)
      static Units Nanometer() { Units out = Meter(); out.power_[ 0 ] = -3; return out; }
      /// Micrometer units (um)
      static Units Micrometer() { Units out = Meter(); out.power_[ 0 ] = -2; return out; }
      /// Millimeter units (mm)
      static Units Millimeter() { Units out = Meter(); out.power_[ 0 ] = -1; return out; }
      /// Kilometer units (km)
      static Units Kilometer() { Units out = Meter(); out.power_[ 0 ] = 1; return out; }
      /// Square micrometer units (um^2)
      static Units SquareMicrometer() { Units out = SquareMeter(); out.power_[ 0 ] = -4; return out; }
      /// Square millimeter units (mm^2)
      static Units SquareMillimeter() { Units out = SquareMeter(); out.power_[ 0 ] = -2; return out; }
      /// Cubic millimeter units (mm^3)
      static Units CubicMillimeter() { Units out = CubicMeter(); out.power_[ 0 ] = -3; return out; }
      /// Second units (s)
      static Units Second() { return Units( BaseUnits::TIME ); }
      /// Millisecond units (ms)
      static Units Millisecond() { Units out = Second(); out.power_[ 0 ] = -1; return out; }
      /// Hertz units (s^-1)
      static Units Hertz() { return Units( BaseUnits::TIME, -1 ); }
      /// Kilohertz units (ms^-1)
      static Units Kilohertz() { Units out = Hertz(); out.power_[ 0 ] = -1; return out; }
      /// Megahertz units (us^-1)
      static Units Megahertz() { Units out = Hertz(); out.power_[ 0 ] = -2; return out; }
      /// Gigahertz units (ns^-1)
      static Units Gigahertz() { Units out = Hertz(); out.power_[ 0 ] = -3; return out; }
      /// Radian units (rad)
      static Units Radian() { return Units( BaseUnits::ANGLE ); }
      /// Pixel units (px)
      static Units Pixel() { return Units( BaseUnits::PIXEL ); }
      /// Square pixel units (px^2)
      static Units SquarePixel() { return Units( BaseUnits::PIXEL, 2 ); }
      /// Cubic pixel units (px^3)
      static Units CubicPixel() { return Units( BaseUnits::PIXEL, 3 ); }

      /// Elevates `this` to the power `p`.
      Units& Power( dip::sint8 power ) {
         for( dip::sint8& p : power_ ) {
            p = clamp_cast< dip::sint8 >( p * power );
         }
         return *this;
      }

      /// Multiplies two units objects.
      Units& operator*=( Units const& other ) {
         for( dip::uint ii = 0; ii < ndims_; ++ii ) {
            power_[ ii ] = clamp_cast< dip::sint8 >( power_[ ii ] + other.power_[ ii ] );
         }
         return *this;
      }

      /// Divides two units objects.
      Units& operator/=( Units const& other ) {
         for( dip::uint ii = 0; ii < ndims_; ++ii ) {
            power_[ ii ] = clamp_cast< dip::sint8 >( power_[ ii ] - other.power_[ ii ] );
         }
         return *this;
      }

      /// Compares two units objects.
      bool operator==( Units const& rhs ) const {
         for( dip::uint ii = 0; ii < ndims_; ++ii ) {
            if( power_[ ii ] != rhs.power_[ ii ] ) {
               return false;
            }
         }
         return true;
      }

      /// Compares two units objects.
      bool operator!=( Units const& rhs ) const {
         return !( *this == rhs );
      }

      /// \brief Compares two units objects. This differs from the `==` operator in that `km` and `um` test equal.
      /// That is, the SI prefix is ignored.
      bool HasSameDimensions( Units const& other ) const {
         for( dip::uint ii = 1; ii < ndims_; ++ii ) { // Not testing the first element
            if( power_[ ii ] != other.power_[ ii ] ) {
               return false;
            }
         }
         return true;
      }

      /// Test to see if the units are dimensionless (has no units).
      bool IsDimensionless() const {
         for( dip::uint ii = 1; ii < ndims_; ++ii ) { // Not testing the first element
            if( power_[ ii ] != 0 ) {
               return false;
            }
         }
         return true;
      }

      /// \brief Test to see if the units are physical. %Units that involve pixels are not physical, and neither are dimensionless units.
      bool IsPhysical() const {
         return ( power_[ int( BaseUnits::PIXEL ) ] == 0 ) && !IsDimensionless();
      }

      /// \brief Adjusts the power of the thousands, so that we can use an SI prefix with the first unit to be written out.
      /// The return value is a number of thousands, which are taken out of the units and should be handled by the caller.
      /// The input `power` is the number of thousands that the caller would like to include into the units.
      dip::sint AdjustThousands( dip::sint power = 0 ) {
         dip::sint thousands = power_[ 0 ] + power;
         if( thousands == 0 ) {
            // No need for checks, this one is easy
            power_[ 0 ] = 0;
            return 0;
         } else {
            dip::sint fp = FirstPower();
            dip::sint newpower = div_floor( thousands, fp ) * fp;
            newpower = clamp< dip::sint >( newpower, -5l, 6l ); // these are the SI prefixes that dip::Units knows.
            power_[ 0 ] = static_cast< dip::sint8 >( newpower );
            thousands -= newpower;
            return thousands;
         }
      }

      /// \brief Returns the power associated with `BaseUnits::THOUSANDS`, corresponding to a given SI prefix.
      dip::sint Thousands() const {
         return power_[ 0 ];
      }

      /// \brief Cast physical units to a string representation, using only ASCII characters.
      ///
      /// No attempt is (yet?) made to produce derived SI units or to translate to different units.
      dip::String String() const {
         return StringRepresentation( false );
      }

      /// \brief Cast physical units to a string representation, using Unicode characters (UTF-8 encoded).
      ///
      /// If Unicode support was disabled during compilation, this function does the same as `dip::Units::String`.
      ///
      /// No attempt is (yet?) made to produce derived SI units or to translate to different units.
      dip::String StringUnicode() const {
         return StringRepresentation( true );
      }

      /// Swaps the values of `this` and `other`.
      void swap( Units& other ) {
         using std::swap;
         swap( power_, other.power_ );
      }

   private:

      constexpr static dip::uint ndims_ = dip::uint( BaseUnits::PIXEL ) + 1u; // The number of different units we have
      std::array< sint8, ndims_ > power_;

      // Returns the power of the first unit to be written out, needed to figure out what the SI prefix must be.
      dip::sint FirstPower() const {
         for( dip::uint ii = 1; ii < ndims_; ++ii ) { // Skipping the first element
            if( power_[ ii ] > 0 ) {
               return power_[ ii ];
            }
         }
         for( dip::uint ii = 1; ii < ndims_; ++ii ) { // Skipping the first element
            if( power_[ ii ] != 0 ) {
               return power_[ ii ];
            }
         }
         return 0;
      }

      DIP_EXPORT dip::String StringRepresentation( bool unicode ) const;
};

/// \brief Multiplies two units objects.
inline Units operator*( Units lhs, Units const& rhs ) {
   lhs *= rhs;
   return lhs;
}

/// \brief Divides two units objects.
inline Units operator/( Units lhs, Units const& rhs ) {
   lhs /= rhs;
   return lhs;
}

/// \brief Insert physical quantity to an output stream as a string of base units. See `dip::Units::String`.
inline std::ostream& operator<<( std::ostream& os, Units const& units ) {
   os << units.StringUnicode();
   return os;
}

inline void swap( Units& v1, Units& v2 ) {
   v1.swap( v2 );
}


/// \brief Encapsulates a quantity with physical units.
///
/// Multiplying a double by a
/// `dip::Units` object yields a `%PhysicalQuantity` object. Numbers and units implicitly
/// convert to a `%PhysicalQuantity`. It is possible to multiply and divide any physical
/// quantities, but adding and subtracting is only possible if the units match.
///
/// ```cpp
///     dip::PhysicalQuantity a = 50 * dip::Units( dip::Units::BaseUnits::LENGTH );
/// ```
struct DIP_NO_EXPORT PhysicalQuantity {

   /// A default-constructed `%PhysicalQuantity` has magnitude 0 and is unitless.
   PhysicalQuantity() {};

   /// Create an arbitrary physical quantity.
   PhysicalQuantity( dip::dfloat m, Units const& u = {} ) : magnitude( m ), units( u ) {};

   /// Create a unit-valued physical quantity.
   PhysicalQuantity( Units const& u ) : magnitude( 1 ), units( u ) {};

   /// One nanometer.
   static PhysicalQuantity Nanometer() { return Units::Nanometer(); }
   /// One micrometer.
   static PhysicalQuantity Micrometer() { return Units::Micrometer(); }
   /// One millimeter.
   static PhysicalQuantity Millimeter() { return Units::Millimeter(); }
   /// One meter.
   static PhysicalQuantity Meter() { return Units::Meter(); }
   /// One kilometer.
   static PhysicalQuantity Kilometer() { return Units::Kilometer(); }
   /// One inch.
   static PhysicalQuantity Inch() { return PhysicalQuantity( 0.0254, Units::Meter() ); }
   /// One mile.
   static PhysicalQuantity Mile() { return PhysicalQuantity( 1609.34, Units::Meter() ); }
   /// One millisecond
   static PhysicalQuantity Millisecond() { return Units::Millisecond(); }
   /// One second
   static PhysicalQuantity Second() { return Units::Second(); }
   /// One minute
   static PhysicalQuantity Minute() { return PhysicalQuantity( 60, Units::Second() ); }
   /// One hour
   static PhysicalQuantity Hour() { return PhysicalQuantity( 3600, Units::Second() ); }
   /// One day
   static PhysicalQuantity Day() { return PhysicalQuantity( 86400, Units::Second() ); }
   /// One radian
   static PhysicalQuantity Radian() { return Units::Radian(); }
   /// One degree
   static PhysicalQuantity Degree() { return PhysicalQuantity( pi / 180, Units::Radian() ); }
   /// One pixel
   static PhysicalQuantity Pixel() { return Units::Pixel(); }
   /// One square pixel
   static PhysicalQuantity SquarePixel() { return Units::SquarePixel(); }
   /// One cubic pixel
   static PhysicalQuantity CubicPixel() { return Units::CubicPixel(); }

   /// Multiplies two physical quantities.
   PhysicalQuantity& operator*=( PhysicalQuantity const& other ) {
      magnitude *= other.magnitude;
      units *= other.units;
      return *this;
   }
   /// Scaling of a physical quantity.
   PhysicalQuantity& operator*=( dip::dfloat other ) {
      magnitude *= other;
      return *this;
   }

   /// Divides two physical quantities.
   PhysicalQuantity& operator/=( PhysicalQuantity const& other ) {
      magnitude /= other.magnitude;
      units /= other.units;
      return *this;
   }
   /// Scaling of a physical quantity.
   PhysicalQuantity& operator/=( dip::dfloat other ) {
      magnitude /= other;
      return *this;
   }

   /// Computes a physical quantity to the power of `p`.
   PhysicalQuantity Power( dip::sint8 p ) const {
      PhysicalQuantity out = *this;
      out.units.Power( p );
      out.magnitude = std::pow( magnitude, p );
      return out;
   }

   /// Computes a physical quantity to the power of -1.
   PhysicalQuantity Invert() const {
      PhysicalQuantity out = *this;
      out.units.Power( -1 );
      out.magnitude = 1.0 / magnitude;
      return out;
   }

   /// Unary negation.
   PhysicalQuantity operator-() const {
      return { -magnitude, units };
   }

   /// Addition of two physical quantities.
   PhysicalQuantity& operator+=( PhysicalQuantity const& other ) {
      DIP_THROW_IF( !units.HasSameDimensions( other.units ), "Units don't match" );
      dip::sint this1000 = units.Thousands();
      dip::sint other1000 = other.units.Thousands();
      if( this1000 > other1000 ) {
         // bring magnitude of other in sync with this
         dip::dfloat otherMag = other.magnitude * pow10( 3 * ( other1000 - this1000 ));
         magnitude += otherMag;
      } else if( this1000 < other1000 ) {
         // bring magnitude of this in sync with other
         magnitude *= pow10( 3 * ( this1000 - other1000 ));
         magnitude += other.magnitude;
         units = other.units;
      } else {
         // just add
         magnitude += other.magnitude;
      }
      return *this;
   }

   /// Subtraction of two physical quantities.
   PhysicalQuantity& operator-=( PhysicalQuantity other ) {
      // Written in terms of `operator+=()` because it's not a trivial function to copy.
      other.magnitude = -other.magnitude;
      return operator+=( other );
   }

   /// Comparison of two physical quantities.
   bool operator==( PhysicalQuantity const& rhs ) const {
      if( units.Thousands() != rhs.units.Thousands() ) {
         if( units.HasSameDimensions( rhs.units )) {
            dip::dfloat lhsmag =     magnitude * pow10( 3 *     units.Thousands() );
            dip::dfloat rhsmag = rhs.magnitude * pow10( 3 * rhs.units.Thousands() );
            return lhsmag == rhsmag;
         } else {
            return false;
         }
      } else {
         return ( magnitude == rhs.magnitude ) && ( units == rhs.units );
      }
   }

   /// Comparison of two physical quantities.
   bool operator!=( PhysicalQuantity const& rhs ) const {
      return !( *this == rhs );
   }

   /// Test to see if the physical quantity is dimensionless (has no units).
   bool IsDimensionless() const {
      return units.IsDimensionless();
   }

   /// \brief Test to see if the physical quantity is actually physical. If pixels are used as units,
   /// it's not a physical quantity, and dimensionless quantities are not physical either.
   bool IsPhysical() const {
      return units.IsPhysical();
   }

   /// \brief Adjusts the SI prefix such that the magnitude of the quantity is readable.
   PhysicalQuantity& Normalize() {
      dip::sint oldthousands = units.Thousands();
      dip::sint zeros = static_cast< dip::sint >( std::floor( std::log10( magnitude ))) + 1; // the +1 gives a nicer range of magnitudes
      // dip::sint newthousands = dip::sint( std::round(( zeros + 3 * oldthousands ) / 3 - 0.1 )); // this gives values [0.1,100) for ^1 and [0.01,10000) for ^2.
      dip::sint newthousands = div_floor< dip::sint >(( zeros + 3 * oldthousands ), 3 ) - oldthousands;
      dip::sint excessthousands = units.AdjustThousands( newthousands );
      magnitude *= std::pow( 10.0, 3 * ( excessthousands - newthousands ));
      return *this;
   }

   /// \brief Removes the SI prefix, such that the quantity is in base units (i.e. m rather than nm).
   PhysicalQuantity& RemovePrefix() {
      dip::sint thousands = units.Thousands();
      units.AdjustThousands( -thousands );
      magnitude *= std::pow( 10.0, 3 * thousands );
      return *this;
   }

   /// Retrieve the magnitude, discarding units.
   explicit operator dip::dfloat() const { return magnitude; };

   /// A physical quantity tests true if it is different from 0.
   explicit operator bool() const { return magnitude != 0; };

   /// Swaps the values of `this` and `other`.
   void swap( PhysicalQuantity& other ) {
      using std::swap;
      swap( magnitude, other.magnitude );
      swap( units, other.units );
   }

   dip::dfloat magnitude = 0; ///< The magnitude
   Units units;               ///< The units
};

/// Multiplies two physical quantities.
inline PhysicalQuantity operator*( PhysicalQuantity lhs, PhysicalQuantity const& rhs ) {
   lhs *= rhs;
   return lhs;
}
/// Scaling of a physical quantity.
inline PhysicalQuantity operator*( PhysicalQuantity lhs, dip::dfloat rhs ) {
   lhs *= rhs;
   return lhs;
}
/// Scaling of a physical quantity.
inline PhysicalQuantity operator*( dip::dfloat lhs, PhysicalQuantity rhs ) {
   rhs *= lhs;
   return rhs;
}

/// Divides two physical quantities.
inline PhysicalQuantity operator/( PhysicalQuantity lhs, PhysicalQuantity const& rhs ) {
   lhs /= rhs;
   return lhs;
}
/// Scaling of a physical quantity.
inline PhysicalQuantity operator/( PhysicalQuantity lhs, dip::dfloat rhs ) {
   lhs /= rhs;
   return lhs;
}
/// Scaling of a physical quantity.
inline PhysicalQuantity operator/( dip::dfloat lhs, PhysicalQuantity rhs ) {
   rhs = rhs.Invert();
   rhs *= lhs;
   return rhs;
}

/// Addition of two physical quantities.
inline PhysicalQuantity operator+( PhysicalQuantity lhs, PhysicalQuantity const& rhs ) {
   lhs += rhs;
   return lhs;
}
/// Subtraction of two physical quantities.
inline PhysicalQuantity operator-( PhysicalQuantity lhs, PhysicalQuantity const& rhs ) {
   lhs -= rhs;
   return lhs;
}

/// Insert physical quantity to an output stream.
inline std::ostream& operator<<( std::ostream& os, PhysicalQuantity const& pq ) {
   os << pq.magnitude << " " << pq.units;
   return os;
}

inline void swap( PhysicalQuantity& v1, PhysicalQuantity& v2 ) {
   v1.swap( v2 );
}

/// An array to hold physical quantities, such as a pixel's size.
using PhysicalQuantityArray = DimensionArray< PhysicalQuantity >;

/// Create an arbitrary physical quantity by multiplying a magnitude with units.
inline PhysicalQuantity operator*( dip::dfloat lhs, Units const& rhs ) {
   return PhysicalQuantity( lhs, rhs );
}


/// \brief Specifies an image's pixel size as physical quantities.
///
/// The object works like an
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
/// However, `dip::PixelSize::SwapDimensions`, `dip::PixelSize::InsertDimension` and
/// `dip::PixelSize::EraseDimension` will expand the array
/// by one element before modifying the last element in the array. This prevents the
/// implicit elements after the defined ones to be modified. For example, inserting
/// dimension *N+K* first expands the array to size *N+K+2* by setting all the new
/// elements to the same value as element *N-1*, then sets a new value for dimension
/// *N+K*. Dimension *N+K+1* now still has the same value as before (though now it is
/// explicitly defined, whereas before it was implicitly defined).
///
/// The pixel size always needs a unit. Any dimensionless quantity is interpreted
/// as a quantity in pixels (px). Pixels are not considered physical units, and are consistently
/// used to represent relative pixel sizes (i.e. sizes in unknown or arbitrary units).
/// Thus, a pixel size of 1 px x 2 px indicates a specific aspect ratio, but does not
/// represent an actual physical size. Use `dip::PhysicalQuantity::IsPhysical` to test
/// for the pixel size being a physical quantity.
/// Angles, measured in radian, are not considered dimensionless here (though radian actually
/// are dimensionless units, see `dip::Units`).
class DIP_NO_EXPORT PixelSize {

   public:

      /// By default, an image has no physical dimensions. The pixel size is given
      /// as "1 pixel".
      PixelSize() {};

      /// Create an isotropic pixel size based on a physical quantity.
      PixelSize( PhysicalQuantity const& m ) {
         Set( m );
      };

      /// Create a pixel size based on an array of physical quantities.
      PixelSize( PhysicalQuantityArray const& m ) {
         Set( m );
      };

      /// Returns the pixel size for the given dimension.
      PhysicalQuantity Get( dip::uint d ) const {
         if( size_.empty() ) {
            return PhysicalQuantity::Pixel();
         } else if( d >= size_.size() ) {
            return size_.back();
         } else {
            return size_[ d ];
         }
      }
      /// \brief Returns the pixel size for the given dimension.
      /// Cannot be used to write to the array, see `Set`.
      PhysicalQuantity operator[]( dip::uint d ) const {
         return Get( d );
      }

      /// \brief Sets the pixel size in the given dimension. Note that
      /// any subsequent dimension, if not explicitly set, will have the same
      /// size.
      void Set( dip::uint d, PhysicalQuantity m ) {
         if( m.IsDimensionless() ) {
            m.units = Units::Pixel();
         }
         if( Get( d ) != m ) {
            EnsureDimensionality( d + 1 );
            size_[ d ] = m;
         }
      }

      /// Sets the isotropic pixel size in all dimensions.
      void Set( PhysicalQuantity const& m ) {
         size_.resize( 1 );
         size_[ 0 ] = m;
         if( m.IsDimensionless() ) {
            size_[ 0 ].units = Units::Pixel();
         }
      }

      /// Sets a non-isotropic pixel size.
      void Set( PhysicalQuantityArray const& m ) {
         size_.resize( m.size() );
         for( dip::uint ii = 0; ii < m.size(); ++ii ) {
            size_[ ii ] = m[ ii ];
            if( m[ ii ].IsDimensionless() ) {
               size_[ ii ].units = Units::Pixel();
            }
         }
      }

      /// Sets the pixel size in the given dimension, in nanometers.
      void SetNanometers( dip::uint d, dip::dfloat m ) {
         Set( d, m * PhysicalQuantity::Nanometer() );
      }
      /// Sets the isotropic pixel size, in nanometers.
      void SetNanometers( dip::dfloat m ) {
         Set( m * PhysicalQuantity::Nanometer() );
      }
      /// Sets the pixel size in the given dimension, in micrometers.
      void SetMicrometers( dip::uint d, dip::dfloat m ) {
         Set( d, m * PhysicalQuantity::Micrometer() );
      }
      /// Sets the isotropic pixel size, in micrometers.
      void SetMicrometers( dip::dfloat m ) {
         Set( m * PhysicalQuantity::Micrometer() );
      }
      /// Sets the pixel size in the given dimension, in millimeters.
      void SetMillimeters( dip::uint d, dip::dfloat m ) {
         Set( d, m * PhysicalQuantity::Millimeter() );
      }
      /// Sets the isotropic pixel size, in millimeters.
      void SetMillimeters( dip::dfloat m ) {
         Set( m * PhysicalQuantity::Millimeter() );
      }
      /// Sets the pixel size in the given dimension, in meters.
      void SetMeters( dip::uint d, dip::dfloat m ) {
         Set( d, m * PhysicalQuantity::Meter() );
      }
      /// Sets the isotropic pixel size, in meters.
      void SetMeters( dip::dfloat m ) {
         Set( m * PhysicalQuantity::Meter() );
      }
      /// Sets the pixel size in the given dimension, in kilometers.
      void SetKilometers( dip::uint d, dip::dfloat m ) {
         Set( d, m * PhysicalQuantity::Kilometer() );
      }
      /// Sets the isotropic pixel size, in kilometers.
      void SetKilometers( dip::dfloat m ) {
         Set( m * PhysicalQuantity::Kilometer() );
      }

      /// Scales the pixel size in the given dimension, if it is defined.
      void Scale( dip::uint d, dip::dfloat s ) {
         if( !size_.empty() ) {
            // we add a dimension past `d` here so that, if they were meaningful, dimensions d+1 and further don't change value.
            EnsureDimensionality( d + 2 );
            size_[ d ] *= s;
         }
      }

      /// Scales the pixel size isotropically.
      void Scale( dip::dfloat s ) {
         for( auto& sz : size_ ) {
            sz *= s;
         }
      }

      /// Scales the pixel size non-isotropically in all dimensions, where defined.
      void Scale( FloatArray const& s ) {
         if( !size_.empty() ) {
            // we do not add a dimension past `d` here, assuming that the caller is modifying all useful dimensions.
            EnsureDimensionality( s.size() );
            for( dip::uint ii = 1; ii < s.size(); ++ii ) {
               size_[ ii ] *= s[ ii ];
            }
         }
      }

      /// Inverts the pixel size in the given dimension, if it is defined.
      void Invert( dip::uint d ) {
         if( !size_.empty() ) {
            // we add a dimension past `d` here so that, if they were meaningful, dimensions d+1 and further don't change value.
            EnsureDimensionality( d + 2 );
            size_[ d ] = size_[ d ].Invert();
         }
      }

      /// Inverts the pixel size in all dimensions, where defined.
      void Invert() {
         for( auto& sz : size_ ) {
            sz = sz.Invert();
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
      void InsertDimension( dip::uint d, PhysicalQuantity m = 1 ) {
         if( m.IsDimensionless() ) {
            m.units = Units::Pixel();
         }
         if( IsDefined() ) {
            // we add a dimension past `d` here so that, if they were meaningful, dimensions d+1 and further don't change value.
            EnsureDimensionality( d + 1 );
            size_.insert( d, m );
         } // else we don't need to do anything: the pixel is undefined and we add a dimensionless quantity.
      }

      /// Erases a dimension
      void EraseDimension( dip::uint d ) {
         // we don't erase the last element in the array, since that would change all subsequent elements too.
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

      /// \brief Returns the aspect ratio of the first `d` dimensions, with respect to the first dimenion. That
      /// is, the output array has `d` elements, where the first one is 1.0. If units differ, the aspect ratio
      /// is 0 for that dimension.
      FloatArray AspectRatio( dip::uint d ) const {
         FloatArray ar( d, 0.0 );
         if( d > 0 ) {
            ar[ 0 ] = 1.0;
            auto m0 = Get( 0 );
            for( dip::uint ii = 1; ii < size_.size(); ++ii ) {
               auto m1 = Get( ii ) / m0;
               if( m1.IsDimensionless() ) {
                  ar[ ii ] = m1.magnitude;
               }
            }
         }
         return ar;
      }

      /// Tests to see if the pixel size is defined.
      bool IsDefined() const {
         return !size_.empty();
      }

      /// Tests to see if the pixel size is physical (i.e. has known physical units).
      bool IsPhysical() const {
         for( auto& sz : size_ ) {
            if( !sz.IsPhysical() ) {
               return false;
            }
         }
         return true;
      }

      /// Multiplies together the sizes for the first `d` dimensions.
      PhysicalQuantity Product( dip::uint d ) const {
         if( d == 0 ) {
            return 1.0;
         }
         PhysicalQuantity out = Get( 0 );
         for( dip::uint ii = 1; ii < d; ++ii ) {
            out = out * Get( ii );
         }
         return out;
      }

      /// Compares two pixel sizes
      bool operator==( PixelSize const& rhs ) const {
         dip::uint d = std::max( size_.size(), rhs.size_.size() );
         for( dip::uint ii = 0; ii < d; ++ii ) {
            if( Get( ii ) != rhs.Get( ii ) ) {
               return false;
            }
         }
         return true;
      }

      /// Compares two pixel sizes
      bool operator!=( PixelSize const& rhs ) const {
         return !( *this == rhs );
      }

      /// Converts physical units to pixels.
      FloatArray ToPixels( PhysicalQuantityArray const& in ) const {
         FloatArray out( in.size() );
         for( dip::uint ii = 0; ii < in.size(); ++ii ) {
            PhysicalQuantity v = Get( ii );
            DIP_THROW_IF( in[ ii ].units != v.units, "Units don't match" );
            out[ ii ] = in[ ii ].magnitude / v.magnitude;
         }
         return out;
      }

      /// Converts pixels to meters.
      PhysicalQuantityArray ToPhysical( FloatArray const& in ) const {
         PhysicalQuantityArray out( in.size() );
         for( dip::uint ii = 0; ii < in.size(); ++ii ) {
            out[ ii ] = in[ ii ] * Get( ii );
         }
         return out;
      }

      /// Swaps the values of `this` and `other`.
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
            size_.resize( d, PhysicalQuantity::Pixel() );
         } else if( size_.size() < d ) {
            size_.resize( d, size_.back() );
         }
      }

};

/// \brief Writes the pixel sizes array to a stream
inline std::ostream& operator<<( std::ostream& os, PixelSize const& ps ) {
   os << "{";
   if( ps.IsDefined() ) {
      os << ps[ 0 ];
      for( dip::uint ii = 1; ii < ps.Size(); ++ii ) {
         os << " x " << ps[ ii ];
      }
   }
   os << "}";
   return os;
}

inline void swap( PixelSize& v1, PixelSize& v2 ) {
   v1.swap( v2 );
}

/// \}

} // namespace dip

#endif // DIP_PHYSICAL_DIMENSIONS_H
