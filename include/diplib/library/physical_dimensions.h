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
            // NOTE: these are used as indices into an array, so we must start at 1 here and use unit increments.
            THOUSANDS = 0,       ///< prefix
            LENGTH,              ///< m
            MASS,                ///< g (should be Kg, but this is easier when working with prefixes)
            TIME,                ///< s
            CURRENT,             ///< A
            TEMPERATURE,         ///< K
            LUMINOUSINTENSITY,   ///< cd
            ANGLE,               ///< rad (though really dimensionless)
            PIXEL,               ///< px (units to use when the image has no dimension information)
      };
      constexpr static dip::uint thousandsIndex = static_cast< dip::uint >( BaseUnits::THOUSANDS );
      // We sometimes skip the 0 index into the array, meaning to skip the thousands element. Don't move it from 0!
      static_assert( thousandsIndex == 0, "dip::Units::BaseUnits::THOUSANDS is not 0!" );

      /// A default-constructed `%Units` is dimensionless.
      constexpr Units() = default;

      /// Construct a `%Units` for a specific unit.
      constexpr explicit Units( BaseUnits bu, dip::sint8 power = 1 ) {
         power_[ static_cast< dip::uint >( bu ) ] = power;
      }

      /// \brief Construct a `%Units` from a string representation of units. The string representation should be as
      /// produced by `dip::Units::String`.
      DIP_EXPORT explicit Units( dip::String const& string );

      /// Elevates `this` to the power `p`.
      constexpr Units& Power( dip::sint8 power ) {
         for( dip::uint ii = 0; ii < nUnits_; ++ii ) {
            power_[ ii ] = clamp_cast< dip::sint8 >( power_[ ii ] * power );
         }
         return *this;
      }

      /// Multiplies two units objects.
      constexpr Units& operator*=( Units const& other ) {
         for( dip::uint ii = 0; ii < nUnits_; ++ii ) {
            power_[ ii ] = clamp_cast< dip::sint8 >( power_[ ii ] + other.power_[ ii ] );
         }
         return *this;
      }

      /// \brief Multiplies two units objects.
      friend constexpr inline Units operator*( Units lhs, Units const& rhs ) {
         lhs *= rhs;
         return lhs;
      }


      /// Divides two units objects.
      constexpr Units& operator/=( Units const& other ) {
         for( dip::uint ii = 0; ii < nUnits_; ++ii ) {
            power_[ ii ] = clamp_cast< dip::sint8 >( power_[ ii ] - other.power_[ ii ] );
         }
         return *this;
      }

      /// \brief Divides two units objects.
      friend constexpr inline Units operator/( Units lhs, Units const& rhs ) {
         lhs /= rhs;
         return lhs;
      }

      /// Compares two units objects.
      constexpr bool operator==( Units const& rhs ) const {
         for( dip::uint ii = 0; ii < nUnits_; ++ii ) {
            if( power_[ ii ] != rhs.power_[ ii ] ) {
               return false;
            }
         }
         return true;
      }

      /// Compares two units objects.
      constexpr bool operator!=( Units const& rhs ) const {
         return !( *this == rhs );
      }

      /// \brief Compares two units objects. This differs from the `==` operator in that `km` and `um` test equal.
      /// That is, the SI prefix is ignored.
      constexpr bool HasSameDimensions( Units const& other ) const {
         for( dip::uint ii = 1; ii < nUnits_; ++ii ) { // Not testing the first element
            if( power_[ ii ] != other.power_[ ii ] ) {
               return false;
            }
         }
         return true;
      }

      /// Test to see if the units are dimensionless (has no units).
      constexpr bool IsDimensionless() const {
         for( dip::uint ii = 1; ii < nUnits_; ++ii ) { // Not testing the first element
            if( power_[ ii ] != 0 ) {
               return false;
            }
         }
         return true;
      }

      /// \brief Test to see if the units are physical. %Units that involve pixels are not physical, and neither are dimensionless units.
      constexpr bool IsPhysical() const {
         return ( power_[ static_cast< dip::uint >( BaseUnits::PIXEL ) ] == 0 ) && !IsDimensionless();
      }

      /// \brief Adjusts the power of the thousands, so that we can use an SI prefix with the first unit to be written out.
      ///
      /// The return value is a number of thousands, which are taken out of the units and should be handled by the caller.
      /// The input `power` is the number of thousands that the caller would like to include into the units.
      constexpr dip::sint AdjustThousands( dip::sint power = 0 ) {
         dip::sint thousands = power_[ thousandsIndex ] + power;
         if( thousands == 0 ) {
            // No need for checks, this one is easy
            power_[ thousandsIndex ] = 0;
            return 0;
         }
         dip::sint fp = FirstPower();
         dip::sint newPower = div_floor( thousands, fp ) * fp;
         newPower = clamp< dip::sint >( newPower, -5l, 6l ); // these are the SI prefixes that dip::Units knows.
         power_[ thousandsIndex ] = static_cast< dip::sint8 >( newPower );
         thousands -= newPower;
         return thousands;
      }

      /// \brief Returns the power associated with `BaseUnits::THOUSANDS`, corresponding to a given SI prefix.
      constexpr dip::sint Thousands() const {
         return power_[ thousandsIndex ];
      }

      /// \brief Cast physical units to a string representation, using only ASCII characters.
      ///
      /// No attempt is (yet?) made to produce derived SI units or to translate to different units.
      ///
      /// Calling the `dip::Units` constructor on the output of this function yields `this`.
      dip::String String() const {
         return StringRepresentation( false );
      }

      /// \brief Cast physical units to a string representation, using Unicode characters (UTF-8 encoded).
      ///
      /// If Unicode support was disabled during compilation, this function does the same as `dip::Units::String`.
      ///
      /// No attempt is (yet?) made to produce derived SI units or to translate to different units.
      ///
      /// Calling the `dip::Units` constructor on the output of this function yields `this`.
      dip::String StringUnicode() const {
         return StringRepresentation( true );
      }

      /// \brief Sets `this` to the units represented by the string. This function recognizes more strings than
      /// what the constructor recognizes.
      void FromString( dip::String const& string );

      /// Swaps the values of `this` and `other`.
      void swap( Units& other ) {
         using std::swap;
         swap( power_, other.power_ );
      }

      // Specific useful powers
      /// Dimensionless nano magnitude (n)
      constexpr static Units Nano() { return Units( BaseUnits::THOUSANDS, -3 ); }
      /// Dimensionless micro magnitude (u)
      constexpr static Units Micro() { return Units( BaseUnits::THOUSANDS, -2 ); }
      /// Dimensionless milli magnitude (m)
      constexpr static Units Milli() { return Units( BaseUnits::THOUSANDS, -1 ); }
      /// Dimensionless kilo magnitude (k)
      constexpr static Units Kilo() { return Units( BaseUnits::THOUSANDS, 1 ); }
      /// Dimensionless mega magnitude (M)
      constexpr static Units Mega() { return Units( BaseUnits::THOUSANDS, 2 ); }
      /// Dimensionless giga magnitude (G)
      constexpr static Units Giga() { return Units( BaseUnits::THOUSANDS, 3 ); }

      // Specific useful units
      /// Meter units (m)
      constexpr static Units Meter() { return Units( BaseUnits::LENGTH ); }
      /// Square meter units (m^2)
      constexpr static Units SquareMeter() { return Units( BaseUnits::LENGTH, 2 ); }
      /// Cubic meter units (m^3)
      constexpr static Units CubicMeter() { return Units( BaseUnits::LENGTH, 3 ); }
      /// Nanometer units (nm)
      constexpr static Units Nanometer() { return Nano() * Meter(); }
      /// Micrometer units (um)
      constexpr static Units Micrometer() { return Micro() * Meter(); }
      /// Millimeter units (mm)
      constexpr static Units Millimeter() { return Milli() * Meter(); }
      /// Kilometer units (km)
      constexpr static Units Kilometer() { return Kilo() * Meter(); }
      /// Square micrometer units (um^2)
      constexpr static Units SquareMicrometer() { return Micrometer().Power( 2 ); }
      /// Square millimeter units (mm^2)
      constexpr static Units SquareMillimeter() { return Millimeter().Power( 2 ); }
      /// Cubic millimeter units (mm^3)
      constexpr static Units CubicMillimeter() { return Millimeter().Power( 3 ); }
      /// Second units (s)
      constexpr static Units Second() { return Units( BaseUnits::TIME ); }
      /// Millisecond units (ms)
      constexpr static Units Millisecond() { return Milli() * Second(); }
      /// Hertz units (s^-1)
      constexpr static Units Hertz() { return Units( BaseUnits::TIME, -1 ); }
      /// Kilohertz units (ms^-1)
      constexpr static Units Kilohertz() { return Kilo() * Hertz(); }
      /// Megahertz units (us^-1)
      constexpr static Units Megahertz() { return Mega() * Hertz(); }
      /// Gigahertz units (ns^-1)
      constexpr static Units Gigahertz() { return Giga() * Hertz(); }
      /// Radian units (rad)
      constexpr static Units Radian() { return Units( BaseUnits::ANGLE ); }
      /// Pixel units (px)
      constexpr static Units Pixel() { return Units( BaseUnits::PIXEL ); }
      /// Square pixel units (px^2)
      constexpr static Units SquarePixel() { return Units( BaseUnits::PIXEL, 2 ); }
      /// Cubic pixel units (px^3)
      constexpr static Units CubicPixel() { return Units( BaseUnits::PIXEL, 3 ); }

   private:
      constexpr static dip::uint nUnits_ = 9; // Number of elements in dip::Units::BaseUnits
      static_assert( static_cast< dip::uint >( BaseUnits::PIXEL ) + 1u == nUnits_, "Inconsistency in dip::Units::nUnits_ static value" );

      sint8 power_[ nUnits_ ] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // Keep length of zeros in sync with nUnits_!
      // Note: we used std::array here earlier, but that makes all constexpr stuff really hard.
      // For example, std::array::operator[] is not constexpr, you're supposed to use std::get<>().
      // This means that there is no way to read array elements in a constexpr function where the index
      // might be a run-time value (i.e. it's not constexpr, or the index is always constexpr).
      // I consider that a bug in C++14, it was fixed in C++17.

      // Returns the power of the first unit to be written out, needed to figure out what the SI prefix must be.
      constexpr dip::sint FirstPower() const {
         for( dip::uint ii = 1; ii < nUnits_; ++ii ) { // Skipping the first element
            if( power_[ ii ] > 0 ) {
               return power_[ ii ];
            }
         }
         for( dip::uint ii = 1; ii < nUnits_; ++ii ) { // Skipping the first element
            if( power_[ ii ] != 0 ) {
               return power_[ ii ];
            }
         }
         return 0;
      }

      DIP_EXPORT dip::String StringRepresentation( bool unicode ) const;
};

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
   constexpr PhysicalQuantity() = default;

   /// Create an arbitrary physical quantity.
   constexpr PhysicalQuantity( dip::dfloat m, Units const& u = {} ) : magnitude( m ), units( u ) {}

   /// Create a unit-valued physical quantity.
   constexpr PhysicalQuantity( Units const& u ) : magnitude( 1 ), units( u ) {}

   /// One nanometer.
   constexpr static PhysicalQuantity Nanometer() { return Units::Nanometer(); }
   /// One micrometer.
   constexpr static PhysicalQuantity Micrometer() { return Units::Micrometer(); }
   /// One millimeter.
   constexpr static PhysicalQuantity Millimeter() { return Units::Millimeter(); }
   /// One centimeter.
   constexpr static PhysicalQuantity Centimeter() { return { 0.01, Units::Meter() }; }
   /// One meter.
   constexpr static PhysicalQuantity Meter() { return Units::Meter(); }
   /// One kilometer.
   constexpr static PhysicalQuantity Kilometer() { return Units::Kilometer(); }
   /// One inch.
   constexpr static PhysicalQuantity Inch() { return { 0.0254, Units::Meter() }; }
   /// One mile.
   constexpr static PhysicalQuantity Mile() { return { 1609.34, Units::Meter() }; }
   /// One millisecond
   constexpr static PhysicalQuantity Millisecond() { return Units::Millisecond(); }
   /// One second
   constexpr static PhysicalQuantity Second() { return Units::Second(); }
   /// One minute
   constexpr static PhysicalQuantity Minute() { return { 60, Units::Second() }; }
   /// One hour
   constexpr static PhysicalQuantity Hour() { return { 3600, Units::Second() }; }
   /// One day
   constexpr static PhysicalQuantity Day() { return { 86400, Units::Second() }; }
   /// One radian
   constexpr static PhysicalQuantity Radian() { return Units::Radian(); }
   /// One degree
   constexpr static PhysicalQuantity Degree() { return { pi / 180, Units::Radian() }; }
   /// One pixel
   constexpr static PhysicalQuantity Pixel() { return Units::Pixel(); }
   /// One square pixel
   constexpr static PhysicalQuantity SquarePixel() { return Units::SquarePixel(); }
   /// One cubic pixel
   constexpr static PhysicalQuantity CubicPixel() { return Units::CubicPixel(); }

   /// Multiplies two physical quantities.
   constexpr PhysicalQuantity& operator*=( PhysicalQuantity const& other ) {
      magnitude *= other.magnitude;
      units *= other.units;
      return *this;
   }
   /// Scaling of a physical quantity.
   constexpr PhysicalQuantity& operator*=( dip::dfloat other ) {
      magnitude *= other;
      return *this;
   }

   /// Divides two physical quantities.
   constexpr PhysicalQuantity& operator/=( PhysicalQuantity const& other ) {
      magnitude /= other.magnitude;
      units /= other.units;
      return *this;
   }
   /// Scaling of a physical quantity.
   constexpr PhysicalQuantity& operator/=( dip::dfloat other ) {
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
   constexpr PhysicalQuantity Invert() const {
      PhysicalQuantity out = *this;
      out.units.Power( -1 );
      out.magnitude = 1.0 / magnitude;
      return out;
   }

   /// Unary negation.
   constexpr PhysicalQuantity operator-() const {
      return { -magnitude, units };
   }

   /// Addition of two physical quantities.
   constexpr PhysicalQuantity& operator+=( PhysicalQuantity const& other ) {
      CheckHasSameDimensions( other );
      return Add( other.units, other.magnitude );
   }

   /// Subtraction of two physical quantities.
   constexpr PhysicalQuantity& operator-=( PhysicalQuantity other ) {
      CheckHasSameDimensions( other );
      return Add( other.units, -other.magnitude );
   }

   /// Exact equality comparison of two physical quantities.
   constexpr bool operator==( PhysicalQuantity const& rhs ) const {
      return ApproximatelyEquals( rhs, 0.0 );
   }

   /// Exact inequality comparison of two physical quantities
   constexpr bool operator!=( PhysicalQuantity const& rhs ) const {
      return NotApproximatelyEquals( rhs, 0.0 );
   }

   /// Approximate equality comparison of two physical quantities.
   constexpr bool ApproximatelyEquals( PhysicalQuantity const& rhs, dfloat tolerance = 1e-6 ) const {
      if( !units.HasSameDimensions( rhs.units )) {
         return false;
      }
      if( units.Thousands() != rhs.units.Thousands() ) {
         dip::dfloat lhs_mag =     magnitude * pow10( 3 *     units.Thousands() );
         dip::dfloat rhs_mag = rhs.magnitude * pow10( 3 * rhs.units.Thousands() );
         return dip::ApproximatelyEquals( lhs_mag, rhs_mag, tolerance );
      }
      return dip::ApproximatelyEquals( magnitude, rhs.magnitude, tolerance );
   }

   /// Exact inequality comparison of two physical quantities
   constexpr bool NotApproximatelyEquals( PhysicalQuantity const& rhs, dfloat tolerance = 1e-6 ) const {
      return !ApproximatelyEquals( rhs, tolerance );
   }

   /// Test to see if the physical quantities can be added together.
   constexpr bool HasSameDimensions( PhysicalQuantity const& other ) const {
      return units.HasSameDimensions( other.units );
   }


   /// Test to see if the physical quantity is dimensionless (has no units).
   constexpr bool IsDimensionless() const {
      return units.IsDimensionless();
   }

   /// \brief Test to see if the physical quantity is actually physical. If pixels are used as units,
   /// it's not a physical quantity, and dimensionless quantities are not physical either.
   constexpr bool IsPhysical() const {
      return units.IsPhysical();
   }

   /// \brief Adjusts the SI prefix such that the magnitude of the quantity is readable.
   constexpr PhysicalQuantity& Normalize() {
      dip::sint oldThousands = units.Thousands();
      dip::sint zeros = 0;
      if (magnitude != 0) {
         zeros = floor_cast( std::log10( std::abs( magnitude ))) + 1; // the +1 gives a nicer range of magnitudes
      }
      // dip::sint newThousands = dip::sint( std::round(( zeros + 3 * oldThousands ) / 3 - 0.1 )); // this gives values [0.1,100) for ^1 and [0.01,10000) for ^2.
      dip::sint newThousands = div_floor< dip::sint >(( zeros + 3 * oldThousands ), 3 ) - oldThousands;
      dip::sint excessThousands = units.AdjustThousands( newThousands );
      magnitude *= pow10( 3 * ( excessThousands - newThousands ));
      return *this;
   }

   /// \brief Removes the SI prefix, such that the quantity is in base units (i.e. m rather than nm).
   constexpr PhysicalQuantity& RemovePrefix() {
      dip::sint thousands = units.Thousands();
      units.AdjustThousands( -thousands ); // sets thousands to 0, guaranteed to return 0
      magnitude *= pow10( 3 * thousands );
      return *this;
   }

   /// Retrieve the magnitude, discarding units.
   constexpr explicit operator dip::dfloat() const { return magnitude; };

   /// A physical quantity tests true if it is different from 0.
   constexpr explicit operator bool() const { return magnitude != 0; };

   /// Swaps the values of `this` and `other`.
   void swap( PhysicalQuantity& other ) {
      using std::swap;
      swap( magnitude, other.magnitude );
      swap( units, other.units );
   }

   dip::dfloat magnitude = 0; ///< The magnitude
   Units units;               ///< The units

   private:

      // Adds stuff to `this`. Call only when units.HasSameDimensions( otherUnits ).
      constexpr PhysicalQuantity& Add( Units const& otherUnits, double otherMagnitude ) {
         dip::sint this1000 = units.Thousands();
         dip::sint other1000 = otherUnits.Thousands();
         if( this1000 > other1000 ) {
            // bring magnitude of other in sync with this
            otherMagnitude *= pow10( 3 * ( other1000 - this1000 ));
            magnitude += otherMagnitude;
         } else if( this1000 < other1000 ) {
            // bring magnitude of this in sync with other
            magnitude *= pow10( 3 * ( this1000 - other1000 ));
            magnitude += otherMagnitude;
            units = otherUnits;
         } else {
            // just add
            magnitude += otherMagnitude;
         }
         return *this;
      }

      // Throws an exception if the physical quantities cannot be added together.
      // A workaround for GCC 5.4 bug that only allows a `throw` in a constexpr function in this way.
      constexpr void CheckHasSameDimensions( PhysicalQuantity const& other ) {
         ( void ) ( units.HasSameDimensions( other.units ) ? 0 : throw dip::ParameterError( "Units don't match" ));
      }
};

/// Multiplies two physical quantities.
constexpr inline PhysicalQuantity operator*( PhysicalQuantity lhs, PhysicalQuantity const& rhs ) {
   lhs *= rhs;
   return lhs;
}
/// Scaling of a physical quantity.
constexpr inline PhysicalQuantity operator*( PhysicalQuantity lhs, dip::dfloat rhs ) {
   lhs *= rhs;
   return lhs;
}
/// Scaling of a physical quantity.
constexpr inline PhysicalQuantity operator*( dip::dfloat lhs, PhysicalQuantity rhs ) {
   rhs *= lhs;
   return rhs;
}

/// Divides two physical quantities.
constexpr inline PhysicalQuantity operator/( PhysicalQuantity lhs, PhysicalQuantity const& rhs ) {
   lhs /= rhs;
   return lhs;
}
/// Scaling of a physical quantity.
constexpr inline PhysicalQuantity operator/( PhysicalQuantity lhs, dip::dfloat rhs ) {
   lhs /= rhs;
   return lhs;
}
/// Scaling of a physical quantity.
constexpr inline PhysicalQuantity operator/( dip::dfloat lhs, PhysicalQuantity rhs ) {
   return rhs.Invert() * lhs;
}

/// Addition of two physical quantities.
constexpr inline PhysicalQuantity operator+( PhysicalQuantity lhs, PhysicalQuantity const& rhs ) {
   lhs += rhs;
   return lhs;
}
/// Subtraction of two physical quantities.
constexpr inline PhysicalQuantity operator-( PhysicalQuantity lhs, PhysicalQuantity const& rhs ) {
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
constexpr inline PhysicalQuantity operator*( dip::dfloat magnitude, Units const& units ) {
   return { magnitude, units };
}
/// Create an arbitrary physical quantity by multiplying a magnitude with units.
constexpr inline PhysicalQuantity operator*( Units const& units, dip::dfloat magnitude ) {
   return { magnitude, units };
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
      PixelSize() = default;

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
            return PhysicalQuantity::Pixel(); // because this is a temporary, Get cannot return a reference.
         }
         if( d >= size_.size() ) {
            return size_.back();
         }
         return size_[ d ];
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
         if( !size_.empty() && ( Get( d1 ) != Get( d2 ))) {
            // we add a dimension past `d` here so that, if they were meaningful, dimensions d+1 and further don't change value.
            EnsureDimensionality( std::max( d1, d2 ) + 2 );
            swap( size_[ d1 ], size_[ d2 ] );
         }
      }

      /// \brief Permutes dimensions according to `order`, such that, after the call, `Get(ii)` returns the value
      /// that `Get(order[ii])` returned before the call.
      ///
      /// The values for any dimension not indexed by `order` will be lost.
      void Permute( UnsignedArray const& order ) {
         if( IsDefined() ) {
            PhysicalQuantityArray newSize( order.size() );
            for( dip::uint ii = 0; ii < order.size(); ++ii ) {
               newSize[ ii ] = Get( order[ ii ] );
            }
            size_ = std::move( newSize );
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

      /// \brief Returns the aspect ratio of the first `d` dimensions, with respect to the first dimension. That
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
         return IsDefined();
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

      /// Compares two pixel sizes, magnitudes are compared with a 1e-6 relative tolerance
      bool operator==( PixelSize const& rhs ) const {
         dip::uint d = std::max( size_.size(), rhs.size_.size() );
         return ApproximatelyEquals( rhs, d, 0.0 );
      }

      /// Compares two pixel sizes, magnitudes are compared with a 1e-6 relative tolerance
      bool operator!=( PixelSize const& rhs ) const {
         return !( *this == rhs );
      }

      /// Compares two pixels for the first `nDims` dimensions, magnitudes are compared with a relative tolerance of `tolerance`
      bool ApproximatelyEquals( PixelSize const& rhs, dip::uint nDims, double tolerance = 1e-6 ) const {
         nDims = std::min( nDims, std::max( size_.size(), rhs.size_.size() ));
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            if( !Get( ii ).ApproximatelyEquals( rhs.Get( ii ), tolerance )) {
               return false;
            }
         }
         return true;
      }

      /// Converts physical units to pixels.
      FloatArray ToPixels( PhysicalQuantityArray const& in ) const {
         FloatArray out( in.size() );
         for( dip::uint ii = 0; ii < in.size(); ++ii ) {
            PhysicalQuantity denom = Get( ii ).RemovePrefix();
            PhysicalQuantity value = in[ ii ];
            value.RemovePrefix();
            DIP_THROW_IF( value.units != denom.units, "Units don't match" );
            out[ ii ] = value.magnitude / denom.magnitude;
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
