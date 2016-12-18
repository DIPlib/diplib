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

#include "diplib/library/error.h"
#include "diplib/library/types.h"
#include "diplib/library/numeric.h"

#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#endif


/// \file
/// \brief Defines support for units, physical quantities and pixel sizes.
/// This file is always included through `diplib.h`.


namespace dip {


/// \addtogroup infrastructure
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
/// in DIPlib...).
///
/// Prefixes are recorded with the `BaseUnits::THOUSANDS` value. It indicates how
/// often to multiply by 10^3. Thus, a value of 1 here corresponds to the 'k'
/// prefix, 3 with 'G', and -2 with 'u' (==micro). Note that for 'mm^2', the
/// value for length is 2, and that for thousands is -2. if thousands were -1,
/// the units would have to be formatted as '10^-3.m^2'. `dip::Units::AdjustThousands'
/// adjusts this power so that it can always be formatted with an SI prefix,
/// returning a magnitude that can be handled elsewhere (the `dip::PhysicalQuantity`
/// class uses this feature).
///
/// The `BaseUnits::PIXEL` value is not to be associated with a pixel size in
/// an image. `dip::MeasurementTool` uses it when an image has no pixel size.
/// `IsPhysical` tests whether there are pixel units present or not.
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
      Units( BaseUnits bu, dip::sint8 power = 1 ) {
         power_.fill( 0 );
         power_[ int( bu ) ] = power;
      }

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
         for( auto& p : power_ ) {
            p = p * power;
         }
         return *this;
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

      /// Test to see if the units are dimensionless.
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
            newpower = clamp( newpower, -5l, 6l ); // these are the SI prefixes that dip::Units knows.
            power_[ 0 ] = static_cast< dip::sint8 >( newpower );
            thousands -= newpower;
            return thousands;
         }
      }

      /// \brief Returns the power associated with `BaseUnits::THOUSANDS`, corresponding to a given SI prefix.
      dip::sint Thousands() const {
         return power_[ 0 ];
      }

      /// \brief Insert physical quantity to an output stream as a string of base units.
      /// No attempt is (yet?) made to produce derived SI units or to translate
      /// to different units.
      friend std::ostream& operator<<( std::ostream& os, Units const& units ) {
         String out;
         // The prefix
         bool prefix = units.WritePrefix( out );
         // We write out positive powers first
         prefix = WritePositivePower( out, "m",   units.power_[ int( BaseUnits::LENGTH            ) ], prefix );
         prefix = WritePositivePower( out, "g",   units.power_[ int( BaseUnits::MASS              ) ], prefix );
         prefix = WritePositivePower( out, "s",   units.power_[ int( BaseUnits::TIME              ) ], prefix );
         prefix = WritePositivePower( out, "A",   units.power_[ int( BaseUnits::CURRENT           ) ], prefix );
         prefix = WritePositivePower( out, "K",   units.power_[ int( BaseUnits::TEMPERATURE       ) ], prefix );
         prefix = WritePositivePower( out, "cd",  units.power_[ int( BaseUnits::LUMINOUSINTENSITY ) ], prefix );
         prefix = WritePositivePower( out, "rad", units.power_[ int( BaseUnits::ANGLE             ) ], prefix );
         prefix = WritePositivePower( out, "px",  units.power_[ int( BaseUnits::PIXEL             ) ], prefix );
         // and negative powers at the end
         prefix = WriteNegativePower( out, "m",   units.power_[ int( BaseUnits::LENGTH            ) ], prefix );
         prefix = WriteNegativePower( out, "g",   units.power_[ int( BaseUnits::MASS              ) ], prefix );
         prefix = WriteNegativePower( out, "s",   units.power_[ int( BaseUnits::TIME              ) ], prefix );
         prefix = WriteNegativePower( out, "A",   units.power_[ int( BaseUnits::CURRENT           ) ], prefix );
         prefix = WriteNegativePower( out, "K",   units.power_[ int( BaseUnits::TEMPERATURE       ) ], prefix );
         prefix = WriteNegativePower( out, "cd",  units.power_[ int( BaseUnits::LUMINOUSINTENSITY ) ], prefix );
         prefix = WriteNegativePower( out, "rad", units.power_[ int( BaseUnits::ANGLE             ) ], prefix );
         prefix = WriteNegativePower( out, "px",  units.power_[ int( BaseUnits::PIXEL             ) ], prefix );
         // send to os
         os << out;
         return os;
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

      // Appends the SI prefix to the string `out`.
      bool WritePrefix( String& out ) const {
         bool prefix = false;
         if( power_[ 0 ] != 0 ) {
            dip::sint p = FirstPower();
            dip::sint n = div_floor( power_[ 0 ], p );
            if(( n < -5 ) || ( n > 6 )) {
               // We cannot print an SI prefix, just print a 10^n instead.
               n = 0;
               p = power_[ 0 ] * 3;
            } else {
               p = ( power_[ 0 ] - n * p ) * 3;     // dip::PhysicalQuantity should make sure that p is 0 here, using AdjustThousands()
            }
            if( p != 0 ) {
               out += String( "10^" ) + std::to_string( p );
               prefix = true;
            }
            if( n != 0 ) {
               if( prefix ) {
                  out += '.'; // TODO: output 'cdot' character?
               }
               constexpr char const* prefixes = "fpnum kMGTPE"; // TODO: output 'mu' character instead of 'u'?
               out += prefixes[ n + 5 ];
               prefix = false; // next thing should not output a '.' first.
            }
         }
         return prefix;
      }

      // Appends a unit with a positive power to the string `out`.
      static bool WritePositivePower( String& out, const char* s, dip::sint8 p, bool prefix ) {
         if( p > 0 ) {
            if( prefix ) {
               out += '.'; // TODO: output 'cdot' character?
            }
            out += s;
            if( p != 1 ) {
               out += '^';
               out += std::to_string( p );
            }
            prefix = true;
         }
         return prefix;
      }

      // Appends a unit with a negative power to the string `out`.
      static bool WriteNegativePower( String& out, const char* s, dip::sint8 p, bool prefix ) {
         if( p < 0 ) {
            if( prefix ) {
               out += '/';
               p = -p;
            }
            out += s;
            if( p != 1 ) {
               out += '^';
               out += std::to_string( p );
            }
            prefix = true;
         }
         return prefix;
      }
};

inline void swap( Units& v1, Units& v2 ) {
   v1.swap( v2 );
}


#ifdef DIP__ENABLE_DOCTEST

#include <sstream>

DOCTEST_TEST_CASE("[DIPlib] testing the dip::Units class") {
   // We only test the printing function here
   // Other workings are tested at the same time as dip::PhysicalQuantity below
   std::stringstream ss;

   dip::Units f = dip::Units::Meter();
   ss << f;
   DOCTEST_CHECK( ss.str() == std::string( "m" ) );
   ss.str( "" );
   ss << f * f;
   DOCTEST_CHECK( ss.str() == std::string( "m^2" ) );
   ss.str( "" );
   ss << f * f * f;
   DOCTEST_CHECK( ss.str() == std::string( "m^3" ) );
   ss.str( "" );
   ss << f * f * f * f;
   DOCTEST_CHECK( ss.str() == std::string( "m^4" ) );
   ss.str( "" );
   ss << dip::Units() / f;
   DOCTEST_CHECK( ss.str() == std::string( "m^-1" ) );
   ss.str( "" );
   ss << dip::Units() / f / f;
   DOCTEST_CHECK( ss.str() == std::string( "m^-2" ) );
   ss.str( "" );
   ss << dip::Units() / f / f / f;
   DOCTEST_CHECK( ss.str() == std::string( "m^-3" ) );
   ss.str( "" );
   ss << dip::Units() / f / f / f / f;
   DOCTEST_CHECK( ss.str() == std::string( "m^-4" ) );

   dip::Units g = dip::Units::Second();
   ss.str( "" );
   ss << f / g;
   DOCTEST_CHECK( ss.str() == std::string( "m/s" ));
   ss.str( "" );
   ss << f / g / g;
   DOCTEST_CHECK( ss.str() == std::string( "m/s^2" ));
   ss.str( "" );
   ss << f / g / g / g;
   DOCTEST_CHECK( ss.str() == std::string( "m/s^3" ));
   ss.str( "" );
   ss << f / g / g / g / g;
   DOCTEST_CHECK( ss.str() == std::string( "m/s^4" ));
   ss.str( "" );
   ss << g / f;
   DOCTEST_CHECK( ss.str() == std::string( "s/m" ));
   ss.str( "" );
   ss << g / f / f;
   DOCTEST_CHECK( ss.str() == std::string( "s/m^2" ));
   ss.str( "" );
   ss << g * g / f;
   DOCTEST_CHECK( ss.str() == std::string( "s^2/m" ));
   ss.str( "" );
   ss << g * f;
   DOCTEST_CHECK( ss.str() == std::string( "m.s" ));

   ss.str( "" );
   ss << dip::Units::Millimeter();
   DOCTEST_CHECK( ss.str() == std::string( "mm" ));
   ss.str( "" );
   ss << dip::Units::Millimeter() * dip::Units::Millimeter();
   DOCTEST_CHECK( ss.str() == std::string( "mm^2" ));
   ss.str( "" );
   ss << dip::Units::Millimeter() * dip::Units::Meter();
   DOCTEST_CHECK( ss.str() == std::string( "10^3.mm^2" ));
   ss.str( "" );
   ss << dip::Units::Kilometer() * dip::Units::Meter();
   DOCTEST_CHECK( ss.str() == std::string( "10^3.m^2" ));
}

#endif


/// \brief Encapsulates a quantity with phyisical units.
///
/// Multiplying a double by a
/// `dip::Units` object yields a `%PhysicalQuantity` object. Numbers and units implicity
/// convert to a `%PhysicalQuantity`. It is possible to multiply and divide any physical
/// quantities, but adding and subtracting is only possible if the units match.
///
///     dip::PhysicalQuantity a = 50 * dip::Units( dip::Units::BaseUnits::LENGTH );
struct PhysicalQuantity {

   /// A default-constructed `%PhysicalQuantity` has magnitude 0 and is unitless.
   PhysicalQuantity() {};

   /// Create an arbitrary physical quantity.
   PhysicalQuantity( double m, Units const& u = {} ) : magnitude( m ), units( u ) {};

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
      rhs = rhs.Power( -1 );
      rhs *= lhs;
      return rhs;
   }

   /// Computes a physical quantity to the power of `p`.
   PhysicalQuantity Power( dip::sint8 p ) const {
      PhysicalQuantity out = *this;
      out.units.Power( p );
      out.magnitude = std::pow( magnitude, p );
      return out;
   }

   /// Unary negation.
   friend PhysicalQuantity operator-( PhysicalQuantity pq ) {
      pq.magnitude = -pq.magnitude;
      return pq;
   }

   /// Addition of two physical quantities.
   PhysicalQuantity& operator+=( PhysicalQuantity const& other ) {
      DIP_THROW_IF( !units.HasSameDimensions( other.units ), "Units don't match" );
      dip::sint this1000 = units.Thousands();
      dip::sint other1000 = other.units.Thousands();
      if( this1000 > other1000 ) {
         // bring magnitude of other in synch with this
         double otherMag = other.magnitude * pow10( 3 * ( other1000 - this1000 ));
         magnitude += otherMag;
      } else if( this1000 < other1000 ) {
         // bring magnitude of this in synch with other
         magnitude *= pow10( 3 * ( this1000 - other1000 ));
         magnitude += other.magnitude;
         units = other.units;
      } else {
         // just add
         magnitude += other.magnitude;
      }
      return *this;
   }
   /// Addition of two physical quantities.
   friend PhysicalQuantity operator+( PhysicalQuantity lhs, PhysicalQuantity const& rhs ) {
      lhs += rhs;
      return lhs;
   }

   /// Subtraction of two physical quantities.
   PhysicalQuantity& operator-=( PhysicalQuantity other ) {
      // Written in terms of `operator+=()` because it's not a trivial function to copy.
      other.magnitude = -other.magnitude;
      return operator+=( other );
   }
   /// Subtraction of two physical quantities.
   friend PhysicalQuantity operator-( PhysicalQuantity lhs, PhysicalQuantity const& rhs ) {
      lhs -= rhs;
      return lhs;
   }

   /// Comparison of two physical quantities.
   friend bool operator==( PhysicalQuantity const& lhs, PhysicalQuantity const& rhs ) {
      if( lhs.units.Thousands() != rhs.units.Thousands() ) {
         if( lhs.units.HasSameDimensions( rhs.units )) {
            double lhsmag = lhs.magnitude * pow10( 3 * lhs.units.Thousands() );
            double rhsmag = rhs.magnitude * pow10( 3 * rhs.units.Thousands() );
            return lhsmag == rhsmag;
         } else {
            return false;
         }
      } else {
         return ( lhs.magnitude == rhs.magnitude ) && ( lhs.units == rhs.units );
      }
   }

   /// Comparison of two physical quantities.
   friend bool operator!=( PhysicalQuantity const& lhs, PhysicalQuantity const& rhs ) {
      return !( lhs == rhs );
   }

   /// Test to see if the physical quantity is dimensionless.
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
      dip::sint newthousands = div_floor(( zeros + 3 * oldthousands ), 3 ) - oldthousands;
      dip::sint excessthousands = units.AdjustThousands( newthousands );
      magnitude *= std::pow( 10.0, 3 * ( excessthousands - newthousands ));
      return *this;
   }

   /// Insert physical quantity to an output stream.
   friend std::ostream& operator<<( std::ostream& os, PhysicalQuantity const& pq ) {
      os << pq.magnitude << " " << pq.units;
      return os;
   }

   /// Retrieve the magnitude, discaring units.
   explicit operator double() const { return magnitude; };

   /// A physical quantity tests true if it is different from 0.
   explicit operator bool() const { return magnitude != 0; };

   /// Swaps the values of `this` and `other`.
   void swap( PhysicalQuantity& other ) {
      using std::swap;
      swap( magnitude, other.magnitude );
      swap( units, other.units );
   }

   double magnitude = 0;   ///< The magnitude
   Units units;            ///< The units
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


#ifdef DIP__ENABLE_DOCTEST

DOCTEST_TEST_CASE("[DIPlib] testing the dip::PhysicalQuantity class") {
   DOCTEST_SUBCASE("Arithmetic") {
      dip::PhysicalQuantity a = 50 * dip::Units::Nanometer();
      dip::PhysicalQuantity b = .4 * dip::Units::Micrometer();
      DOCTEST_CHECK( a + b == b + a );
      DOCTEST_CHECK( a + a == 2 * a );
      DOCTEST_CHECK( a * a == a.Power( 2 ) );
      DOCTEST_CHECK(( 1 / ( a * a )) == a.Power( -2 ) );
      dip::PhysicalQuantity c( 100, dip::Units::Second() );
      DOCTEST_CHECK(( 1 / c ) == c.Power( -1 ) );
      DOCTEST_CHECK(( b / c ) == b * c.Power( -1 ) );
      dip::PhysicalQuantity d = 180 * dip::PhysicalQuantity::Degree();
      DOCTEST_CHECK( d.magnitude == doctest::Approx( dip::pi ) );
      DOCTEST_CHECK_THROWS( c + d );
   }
   DOCTEST_SUBCASE("Normalization") {
      dip::PhysicalQuantity f = dip::PhysicalQuantity::Meter();
      DOCTEST_CHECK( ( f * 1 ).Normalize().magnitude == 1 );
      DOCTEST_CHECK( ( f * 0.1 ).Normalize().magnitude == 0.1 );
      DOCTEST_CHECK( ( f * 0.01 ).Normalize().magnitude == 10 );
      DOCTEST_CHECK( ( f * 0.001 ).Normalize().magnitude == 1 );
      DOCTEST_CHECK( ( f * 0.0001 ).Normalize().magnitude == 0.1 );
      DOCTEST_CHECK( ( f * 0.00001 ).Normalize().magnitude == 10 );
      DOCTEST_CHECK( ( f * 0.000001 ).Normalize().magnitude == 1 );
      DOCTEST_CHECK( ( f * 0.0000001 ).Normalize().magnitude == doctest::Approx( 0.1 ));
      DOCTEST_CHECK( ( f * 0.00000001 ).Normalize().magnitude == 10 );
      DOCTEST_CHECK( ( f * 0.000000001 ).Normalize().magnitude == 1 );
      DOCTEST_CHECK( ( f * 0.0000000001 ).Normalize().magnitude == doctest::Approx( 0.1 ));
      DOCTEST_CHECK( ( f * 10 ).Normalize().magnitude == 10 );
      DOCTEST_CHECK( ( f * 100 ).Normalize().magnitude == 0.1 );
      DOCTEST_CHECK( ( f * 1000 ).Normalize().magnitude == 1 );
      DOCTEST_CHECK( ( f * 10000 ).Normalize().magnitude == 10 );
      DOCTEST_CHECK( ( f * 100000 ).Normalize().magnitude == doctest::Approx( 0.1 ));
      DOCTEST_CHECK( ( f * 1000000 ).Normalize().magnitude == 1 );
      DOCTEST_CHECK( ( f * 10000000 ).Normalize().magnitude == 10 );
      DOCTEST_CHECK( ( f * 100000000 ).Normalize().magnitude == doctest::Approx( 0.1 ));
      DOCTEST_CHECK( ( f * 1000000000 ).Normalize().magnitude == 1 );
      DOCTEST_CHECK( ( f * f * 1 ).Normalize().magnitude == 1 );
      DOCTEST_CHECK( ( f * f * 10 ).Normalize().magnitude == 10 );
      DOCTEST_CHECK( ( f * f * 100 ).Normalize().magnitude == 100 );
      DOCTEST_CHECK( ( f * f * 1000 ).Normalize().magnitude == 1000 );
      DOCTEST_CHECK( ( f * f * 10000 ).Normalize().magnitude == 10000 );
      DOCTEST_CHECK( ( f * f * 100000 ).Normalize().magnitude == doctest::Approx( 0.1 ));
      DOCTEST_CHECK( ( f * f * 1000000 ).Normalize().magnitude == 1 );
      DOCTEST_CHECK( ( f * f * 10000000 ).Normalize().magnitude == 10 );
      DOCTEST_CHECK( ( f * f * 100000000 ).Normalize().magnitude == 100 );
      DOCTEST_CHECK( ( f * f * 1000000000 ).Normalize().magnitude == 1000 );

      DOCTEST_CHECK( ( f * 1 ).Normalize().units.Thousands() == 0 );
      DOCTEST_CHECK( ( f * 0.1 ).Normalize().units.Thousands() == 0 );
      DOCTEST_CHECK( ( f * 0.01 ).Normalize().units.Thousands() == -1 );
      DOCTEST_CHECK( ( f * 0.001 ).Normalize().units.Thousands() == -1 );
      DOCTEST_CHECK( ( f * 0.0001 ).Normalize().units.Thousands() == -1 );
      DOCTEST_CHECK( ( f * 0.00001 ).Normalize().units.Thousands() == -2 );
      DOCTEST_CHECK( ( f * 0.000001 ).Normalize().units.Thousands() == -2 );
      DOCTEST_CHECK( ( f * 0.0000001 ).Normalize().units.Thousands() == -2 );
      DOCTEST_CHECK( ( f * 0.00000001 ).Normalize().units.Thousands() == -3 );
      DOCTEST_CHECK( ( f * 0.000000001 ).Normalize().units.Thousands() == -3 );
      DOCTEST_CHECK( ( f * 0.0000000001 ).Normalize().units.Thousands() == -3 );
      DOCTEST_CHECK( ( f * 10 ).Normalize().units.Thousands() == 0 );
      DOCTEST_CHECK( ( f * 100 ).Normalize().units.Thousands() == 1 );
      DOCTEST_CHECK( ( f * 1000 ).Normalize().units.Thousands() == 1 );
      DOCTEST_CHECK( ( f * 10000 ).Normalize().units.Thousands() == 1 );
      DOCTEST_CHECK( ( f * 100000 ).Normalize().units.Thousands() == 2 );
      DOCTEST_CHECK( ( f * 1000000 ).Normalize().units.Thousands() == 2 );
      DOCTEST_CHECK( ( f * 10000000 ).Normalize().units.Thousands() == 2 );
      DOCTEST_CHECK( ( f * 100000000 ).Normalize().units.Thousands() == 3 );
      DOCTEST_CHECK( ( f * 1000000000 ).Normalize().units.Thousands() == 3 );
      DOCTEST_CHECK( ( f * f * 1 ).Normalize().units.Thousands() == 0 );
      DOCTEST_CHECK( ( f * f * 10 ).Normalize().units.Thousands() == 0 );
      DOCTEST_CHECK( ( f * f * 100 ).Normalize().units.Thousands() == 0 );
      DOCTEST_CHECK( ( f * f * 1000 ).Normalize().units.Thousands() == 0 );
      DOCTEST_CHECK( ( f * f * 10000 ).Normalize().units.Thousands() == 0 );
      DOCTEST_CHECK( ( f * f * 100000 ).Normalize().units.Thousands() == 2 );
      DOCTEST_CHECK( ( f * f * 1000000 ).Normalize().units.Thousands() == 2 );
      DOCTEST_CHECK( ( f * f * 10000000 ).Normalize().units.Thousands() == 2 );
      DOCTEST_CHECK( ( f * f * 100000000 ).Normalize().units.Thousands() == 2 );
      DOCTEST_CHECK( ( f * f * 1000000000 ).Normalize().units.Thousands() == 2 );
   }
}

#endif


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
/// as 1, and considered as an "undefined" size. Angles, measured in radian, are
/// not considered dimensionless, though they actually are (see `dip::Units`). Pixels,
/// though not actually dimensionless, are considered so and treated as an "undefined"
/// size. Thus, any physical quantity represented in an object of this class must be
/// `dip::PhysicalQuantity::IsPhysical`.
class PixelSize {

   public:

      /// By default, an image has no physical dimensions. The pixel size is given
      /// as "1 pixel".
      PixelSize() {};

      /// Create an isotropic pixel size based on a physical quantity.
      PixelSize( PhysicalQuantity const& m ) {
         if( m.IsPhysical() ) {
            size_.resize( 1 );
            size_[ 0 ] = m;
         } else {
            size_.clear();
         }
      };

      /// Create a pixel size based on an array of physical quantities.
      PixelSize( PhysicalQuantityArray const& m ) {
         Set( m );
      };

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
      /// \brief Returns the pixel size for the given dimension.
      /// Cannot be used to write to the array, see `Set`.
      PhysicalQuantity operator[]( dip::uint d ) const {
         return Get( d );
      }

      /// \brief Sets the pixel size in the given dimension. Note that
      /// any subsequent dimension, if not explicitly set, will have the same
      /// size.
      void Set( dip::uint d, PhysicalQuantity m ) {
         if( !m.IsPhysical() ) {
            m = 1;
         }
         if( Get( d ) != m ) {
            EnsureDimensionality( d + 1 );
            size_[ d ] = m;
         }
      }

      /// Sets the isotropic pixel size in all dimensions.
      void Set( PhysicalQuantity const& m ) {
         if( m.IsPhysical() ) {
            size_.resize( 1 );
            size_[ 0 ] = m;
         } else {
            size_.clear();
         }
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
         size_.resize( m.size() );
         for( dip::uint ii = 0; ii < m.size(); ++ii ) {
            if( m[ ii ].IsPhysical() ) {
               size_[ ii ] = m[ ii ];
            } else {
               size_[ ii ] = 1;
            }
         }
      }

      /// Scales the pixel size in the given dimension, if it is defined.
      void Scale( dip::uint d, double s ) {
         if( ( !size_.empty() ) && !Get( d ).IsDimensionless() ) {
            // we add a dimension past `d` here so that, if they were meaningful, dimensions d+1 and further don't change value.
            EnsureDimensionality( d + 2 );
            size_[ d ] *= s;
         }
      }

      /// Scales the pixel size isotropically.
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
      void InsertDimension( dip::uint d, PhysicalQuantity m = 1 ) {
         if( !m.IsPhysical() ) {
            m = 1;
         }
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
            DIP_THROW_IF( in[ ii ].units != v.units, "Units don't match" );
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
            size_.resize( d, 1 );
         } else if( size_.size() < d ) {
            size_.resize( d, size_.back() );
         }
      }

};

inline void swap( PixelSize& v1, PixelSize& v2 ) {
   v1.swap( v2 );
}

/// \}

} // namespace dip

#endif // DIP_PHYSDIMS_H
