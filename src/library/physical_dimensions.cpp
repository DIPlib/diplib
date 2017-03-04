/*
 * DIPlib 3.0
 * This file contains some dip::Units methods that create and parse string representations of units.
 *
 * (c)2015-2017, Cris Luengo.
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

#include "diplib/library/physical_dimensions.h"

namespace dip {

// Parses a part of a string respresentation of units.
static bool ParseComponent( dip::String const& string, dip::uint& ii, Units::BaseUnits& bu, int& power ) {
   // <component> = <units>[^<N>]
   // <units> = {m|g|s|A|K|cd|rad|px}
   // <N> = small integer
   switch( string[ ii ] ) {
      case 'm': bu = Units::BaseUnits::LENGTH; break;
      case 'g': bu = Units::BaseUnits::MASS; break;
      case 's': bu = Units::BaseUnits::TIME; break;
      case 'A': bu = Units::BaseUnits::CURRENT; break;
      case 'K': bu = Units::BaseUnits::TEMPERATURE; break;
      case 'c': bu = Units::BaseUnits::LUMINOUSINTENSITY;
         if( string[ ++ii ] != 'd' ) { return false; }
         break;
      case 'r': bu = Units::BaseUnits::ANGLE;
         if(( ii + 2 >= string.size() ) || ( string[ ii + 1 ] != 'a' ) || ( string [ ii + 2 ] != 'd' )) { return false; }
         ii += 2;
         break;
      case 'p':  bu = Units::BaseUnits::PIXEL;
         if( string[ ++ii ] != 'x' ) { return false; }
         break;
      default:
         return false;
   }
   ++ii;
   power = 1;
   if( string[ ii ] == '^' ) {
      ++ii;
      dip::uint n = 0;
      if( string[ ii + n ] == '-' ) { ++n; }
      while( isdigit( string[ ii + n ] )) { ++n; }
      if( n == 0 ) { return false; }
      power = std::atoi( &(string[ ii ] ));
      ii += n;
   }
   return true;
}

// Constructs a new Units using a string representation of those units.
Units::Units( dip::String const& string ) {
   power_.fill( 0 );
   // Parse the string.
   // Format: [10^<3N>.][<prefix>]<component>[{.|/}<component>]
   // <component> = <units>[^<N>]
   // <prefix> = {f|p|n|u|m|k|M|G|T|P|E}
   // <units> = {m|g|s|A|K|cd|rad|px}
   // <N> = [-] small integer
   // <3N> = [-] small integer multiple of 3
   // note that "m" is the only difficult component to distinguish: if it's followed by a letter, it's the prefix!
   // TODO: accept more general strings, to let users type something? e.g. allow prefixes on any component?
   // TODO: allow alternative units? e.g. inches, Hz, ...
   constexpr char const* errorMessage = "Ill-formed Units string";
   dip::uint ii = 0;
   dip::uint N = string.size();
   if( ( string[ ii ] == '1' ) && ( N > 5 ) ) {
      // Starts with 10^<N>.
      DIP_THROW_IF( string[ ii + 1 ] != '0' || string[ ii + 2 ] != '^', errorMessage );
      ii += 3;
      dip::uint n = 0;
      if( string[ ii + n ] == '-' ) { ++n; }
      while( isdigit( string[ ii + n ] ) ) { ++n; }
      DIP_THROW_IF( n == 0, errorMessage );
      int power = std::atoi( & ( string[ ii ] ) );
      DIP_THROW_IF( ( power / 3 ) * 3 != power, errorMessage );
      ii += n;
      DIP_THROW_IF( string[ ii ] != '.', errorMessage );
      ++ii;
      power_[ int( BaseUnits::THOUSANDS ) ] += power / 3;
   }
   int thousands = 0;
   switch( string[ ii ] ) {
      case 'f': thousands = -5; ++ii; break;
      case 'p':
         if( string[ ii + 1 ] != 'x' ) { thousands = -4; ++ii; } break; // could be pico or pixel
      case 'n':
         thousands = -3; ++ii; break;
      case 'u':
         thousands = -2; ++ii; break;
      case 'm':
         if( isalpha( string[ ii + 1 ] ) ) { thousands = -1; ++ii; } break; // could be mili or meter
      case 'k':
         thousands = 1; ++ii; break;
      case 'M':
         thousands = 2; ++ii; break;
      case 'G':
         thousands = 3; ++ii; break;
      case 'T':
         thousands = 4; ++ii; break;
      case 'P':
         thousands = 5; ++ii; break;
      case 'E':
         thousands = 6; ++ii; break;
      default:
         break;
   }
   int power;
   BaseUnits bu;
   DIP_THROW_IF( !ParseComponent( string, ii, bu, power ), errorMessage );
   power_[ int( BaseUnits::THOUSANDS ) ] += thousands * power;
   power_[ int( bu ) ] += power;
   while( string[ ii ] != '\0' ) {
      bool neg = string[ ii ] == '/';
      DIP_THROW_IF( !neg && ( string[ ii ] != '.' ), errorMessage );
      ++ii;
      DIP_THROW_IF( !ParseComponent( string, ii, bu, power ), errorMessage );
      power_[ int( bu ) ] += neg ? -power : power;
   }
}

// Appends a unit with a positive power to the string `out`.
static bool WritePositivePower( dip::String& out, const char* s, dip::sint8 p, bool prefix ) {
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
static bool WriteNegativePower( dip::String& out, const char* s, dip::sint8 p, bool prefix ) {
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

// Creates a string representation of the units.
dip::String Units::String() const {
   dip::String out;
   bool prefix = false;
   // The prefix
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
         out += dip::String( "10^" ) + std::to_string( p );
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
   // We write out positive powers first
   prefix = WritePositivePower( out, "m",   power_[ int( BaseUnits::LENGTH            ) ], prefix );
   prefix = WritePositivePower( out, "g",   power_[ int( BaseUnits::MASS              ) ], prefix );
   prefix = WritePositivePower( out, "s",   power_[ int( BaseUnits::TIME              ) ], prefix );
   prefix = WritePositivePower( out, "A",   power_[ int( BaseUnits::CURRENT           ) ], prefix );
   prefix = WritePositivePower( out, "K",   power_[ int( BaseUnits::TEMPERATURE       ) ], prefix );
   prefix = WritePositivePower( out, "cd",  power_[ int( BaseUnits::LUMINOUSINTENSITY ) ], prefix );
   prefix = WritePositivePower( out, "rad", power_[ int( BaseUnits::ANGLE             ) ], prefix );
   prefix = WritePositivePower( out, "px",  power_[ int( BaseUnits::PIXEL             ) ], prefix );
   // and negative powers at the end
   prefix = WriteNegativePower( out, "m",   power_[ int( BaseUnits::LENGTH            ) ], prefix );
   prefix = WriteNegativePower( out, "g",   power_[ int( BaseUnits::MASS              ) ], prefix );
   prefix = WriteNegativePower( out, "s",   power_[ int( BaseUnits::TIME              ) ], prefix );
   prefix = WriteNegativePower( out, "A",   power_[ int( BaseUnits::CURRENT           ) ], prefix );
   prefix = WriteNegativePower( out, "K",   power_[ int( BaseUnits::TEMPERATURE       ) ], prefix );
   prefix = WriteNegativePower( out, "cd",  power_[ int( BaseUnits::LUMINOUSINTENSITY ) ], prefix );
   prefix = WriteNegativePower( out, "rad", power_[ int( BaseUnits::ANGLE             ) ], prefix );
   prefix = WriteNegativePower( out, "px",  power_[ int( BaseUnits::PIXEL             ) ], prefix );
   return out;
}


} // namespace dip



#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"

DOCTEST_TEST_CASE("[DIPlib] testing the dip::Units class") {
   // We only test the string conversion functions here
   // Other workings are tested at the same time as dip::PhysicalQuantity below

   dip::Units f = dip::Units::Meter();
   DOCTEST_CHECK( ( f ).String() == "m" );
   DOCTEST_CHECK( ( f * f ).String() == "m^2" );
   DOCTEST_CHECK( ( f * f * f ).String() == "m^3" );
   DOCTEST_CHECK( ( f * f * f * f ).String() == "m^4" );
   DOCTEST_CHECK( ( dip::Units() / f ).String() == "m^-1" );
   DOCTEST_CHECK( ( dip::Units() / f / f ).String() == "m^-2" );
   DOCTEST_CHECK( ( dip::Units() / f / f / f ).String() == "m^-3" );
   DOCTEST_CHECK( ( dip::Units() / f / f / f / f ).String() == "m^-4" );
   DOCTEST_CHECK( f == dip::Units( "m" ));
   DOCTEST_CHECK( f * f == dip::Units( "m^2" ));
   DOCTEST_CHECK( f * f * f == dip::Units( "m^3" ));
   DOCTEST_CHECK( f * f * f * f == dip::Units( "m^4" ));
   DOCTEST_CHECK( dip::Units() / f == dip::Units( "m^-1" ));
   DOCTEST_CHECK( dip::Units() / f / f == dip::Units( "m^-2" ));
   DOCTEST_CHECK( dip::Units() / f / f / f == dip::Units( "m^-3" ));
   DOCTEST_CHECK( dip::Units() / f / f / f / f == dip::Units( "m^-4" ));

   dip::Units g = dip::Units::Second();
   DOCTEST_CHECK( ( f / g ).String() == "m/s" );
   DOCTEST_CHECK( ( f / g / g ).String() == "m/s^2" );
   DOCTEST_CHECK( ( f / g / g / g ).String() == "m/s^3" );
   DOCTEST_CHECK( ( f / g / g / g / g ).String() == "m/s^4" );
   DOCTEST_CHECK( ( g / f ).String() == "s/m" );
   DOCTEST_CHECK( ( g / f / f ).String() == "s/m^2" );
   DOCTEST_CHECK( ( g * g / f ).String() == "s^2/m" );
   DOCTEST_CHECK( ( g * f ).String() == "m.s" );
   DOCTEST_CHECK( f / g == dip::Units( "m/s" ));
   DOCTEST_CHECK( f / g / g == dip::Units( "m/s^2" ));
   DOCTEST_CHECK( f / g / g / g == dip::Units( "m/s^3" ));
   DOCTEST_CHECK( f / g / g / g / g == dip::Units( "m/s^4" ));
   DOCTEST_CHECK( g / f == dip::Units( "s/m" ));
   DOCTEST_CHECK( g / f / f == dip::Units( "s/m^2" ));
   DOCTEST_CHECK( g * g / f == dip::Units( "s^2/m" ));
   DOCTEST_CHECK( g * f == dip::Units( "m.s" ));

   DOCTEST_CHECK( ( dip::Units::Millimeter() ).String() == "mm" );
   DOCTEST_CHECK( ( dip::Units::Millimeter() * dip::Units::Millimeter() ).String() == "mm^2" );
   DOCTEST_CHECK( ( dip::Units::Millimeter() * dip::Units::Meter() ).String() == "10^3.mm^2" );
   DOCTEST_CHECK( ( dip::Units::Kilometer() * dip::Units::Meter() ).String() == "10^3.m^2" );
   DOCTEST_CHECK( dip::Units::Millimeter() == dip::Units( "mm" ));
   DOCTEST_CHECK( dip::Units::Millimeter() * dip::Units::Millimeter() == dip::Units( "mm^2" ));
   DOCTEST_CHECK( dip::Units::Millimeter() * dip::Units::Meter() == dip::Units( "10^3.mm^2" ));
   DOCTEST_CHECK( dip::Units::Kilometer() * dip::Units::Meter() == dip::Units( "10^3.m^2" ));

   DOCTEST_CHECK( ( dip::Units( "10^6.mm^2" )).String() == "m^2" );
   DOCTEST_CHECK( ( dip::Units( "km/s" )).String() == "km/s" );
   DOCTEST_CHECK( ( dip::Units( "km.cd.rad.px" )).String() == "km.cd.rad.px" );
   DOCTEST_CHECK( ( dip::Units( "km.cd/rad.px" )).String() == "km.cd.px/rad" );
   DOCTEST_CHECK( ( dip::Units( "10^3.km^-1.cd^-2/K" )).String() == "m^-1/K/cd^2" );
}

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

#endif // DIP__ENABLE_DOCTEST
