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

namespace {

#ifdef DIP__ENABLE_UNICODE

constexpr char const micron[] = u8"\u00B5";
static_assert( sizeof( micron ) == 2+1, "UTF-8 encoded symbol is of different size than expected." );

constexpr char const cdot[] = u8"\u00B7";
static_assert( sizeof( cdot ) == 2+1, "UTF-8 encoded symbol is of different size than expected." );

constexpr char const superN[] = u8"\u207B";
static_assert( sizeof( superN ) == 3+1, "UTF-8 encoded symbol is of different size than expected." );
constexpr char const super0[] = u8"\u2070";
static_assert( sizeof( super0 ) == 3+1, "UTF-8 encoded symbol is of different size than expected." );
constexpr char const super1[] = u8"\u00B9";
static_assert( sizeof( super1 ) == 2+1, "UTF-8 encoded symbol is of different size than expected." );
constexpr char const super2[] = u8"\u00B2";
static_assert( sizeof( super2 ) == 2+1, "UTF-8 encoded symbol is of different size than expected." );
constexpr char const super3[] = u8"\u00B3";
static_assert( sizeof( super3 ) == 2+1, "UTF-8 encoded symbol is of different size than expected." );
constexpr char const super4[] = u8"\u2074";
static_assert( sizeof( super4 ) == 3+1, "UTF-8 encoded symbol is of different size than expected." );
constexpr char const super5[] = u8"\u2075";
static_assert( sizeof( super5 ) == 3+1, "UTF-8 encoded symbol is of different size than expected." );
constexpr char const super6[] = u8"\u2076";
static_assert( sizeof( super6 ) == 3+1, "UTF-8 encoded symbol is of different size than expected." );
constexpr char const super7[] = u8"\u2077";
static_assert( sizeof( super7 ) == 3+1, "UTF-8 encoded symbol is of different size than expected." );
constexpr char const super8[] = u8"\u2078";
static_assert( sizeof( super8 ) == 3+1, "UTF-8 encoded symbol is of different size than expected." );
constexpr char const super9[] = u8"\u2079";
static_assert( sizeof( super9 ) == 3+1, "UTF-8 encoded symbol is of different size than expected." );

#else

constexpr char const cdot[] = ".";

#endif

} // namespace

// --- READING UNITS ---

namespace {

bool ParsePower( dip::String const& string, dip::uint& ii, int& power ) {
   power = 0;
   if( string[ ii ] == '^' ) {
      ++ii;
      dip::uint n = 0;
      if( string[ ii + n ] == '-' ) { ++n; }
      while( isdigit( string[ ii + n ] )) { ++n; }
      if( n == 0 ) {
         return false;
      }
      power = std::atoi( &( string[ ii ] ) );
      ii += n;
   }
#ifdef DIP__ENABLE_UNICODE
   else {
      bool neg = false;
      if(( string[ ii ] == superN[ 0 ] ) && ( string[ ii + 1 ] == superN[ 1 ] ) && ( string[ ii + 2 ] == superN[ 2 ] )) {
         neg = true;
         ii += 3;
      }
      int p = 0;
      while( true ) {
         if(( string[ ii ] == super0[ 0 ] ) && ( string[ ii + 1 ] == super0[ 1 ] ) && ( string[ ii + 2 ] == super0[ 2 ] )) {
            p *= 10;
            ii += 3;
         } else if(( string[ ii ] == super1[ 0 ] ) && ( string[ ii + 1 ] == super1[ 1 ] )) {
            p = p * 10 + 1;
            ii += 2;
         } else if(( string[ ii ] == super2[ 0 ] ) && ( string[ ii + 1 ] == super2[ 1 ] )) {
            p = p * 10 + 2;
            ii += 2;
         } else if(( string[ ii ] == super3[ 0 ] ) && ( string[ ii + 1 ] == super3[ 1 ] )) {
            p = p * 10 + 3;
            ii += 2;
         } else if(( string[ ii ] == super4[ 0 ] ) && ( string[ ii + 1 ] == super4[ 1 ] ) && ( string[ ii + 2 ] == super4[ 2 ] )) {
            p = p * 10 + 4;
            ii += 3;
         } else if(( string[ ii ] == super5[ 0 ] ) && ( string[ ii + 1 ] == super5[ 1 ] ) && ( string[ ii + 2 ] == super5[ 2 ] )) {
            p = p * 10 + 5;
            ii += 3;
         } else if(( string[ ii ] == super6[ 0 ] ) && ( string[ ii + 1 ] == super6[ 1 ] ) && ( string[ ii + 2 ] == super6[ 2 ] )) {
            p = p * 10 + 6;
            ii += 3;
         } else if(( string[ ii ] == super7[ 0 ] ) && ( string[ ii + 1 ] == super7[ 1 ] ) && ( string[ ii + 2 ] == super7[ 2 ] )) {
            p = p * 10 + 7;
            ii += 3;
         } else if(( string[ ii ] == super8[ 0 ] ) && ( string[ ii + 1 ] == super8[ 1 ] ) && ( string[ ii + 2 ] == super8[ 2 ] )) {
            p = p * 10 + 8;
            ii += 3;
         } else if(( string[ ii ] == super9[ 0 ] ) && ( string[ ii + 1 ] == super9[ 1 ] ) && ( string[ ii + 2 ] == super9[ 2 ] )) {
            p = p * 10 + 9;
            ii += 3;
         } else {
            power = neg ? -p : p;
            return !( neg && ( p == 0 ));
         }
      }
   }
#endif
   return true;
}

// Parses a part of a string representation of units.
bool ParseComponent( dip::String const& string, dip::uint& ii, Units::BaseUnits& bu, int& power ) {
   // <component> = <units>[ { ^<N> | <n> } ]
   // <units> = {m|g|s|A|K|cd|rad|px}
   // <N> = small integer
   // <n> = small integer, written in UTF-8 superscript numbers
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
   if( !ParsePower( string, ii, power )) {
      return false;
   }
   if( power == 0 ) { // This means that there was no power, we take power == 1
      power = 1;
   }
   return true;
}

bool ExpectCDot( dip::String const& string, dip::uint& ii ) {
   if( string[ ii ] == '.' ) {
      ++ii;
      return true;
   }
#ifdef DIP__ENABLE_UNICODE
   if(( string[ ii ] == cdot[ 0 ] ) && ( string[ ii + 1 ] == cdot[ 1 ] )) {
      ii += 2;
      return true;
   }
#endif
   return false;
}

} // namespace

// Constructs a new Units using a string representation of those units.
Units::Units( dip::String const& string ) {
   power_.fill( 0 );
   if( string.empty() ) {
      return;
   }
   // Parse the string.
   // Format: [ '10' { '^' <3N> | <3n> } <cdot> ][ <prefix> ] <component> [ { <cdot> | '/' } <component> ]
   // <component> = <units>[^<N>]
   // <prefix> = {f|p|n|u|m|k|M|G|T|P|E}
   // <units> = {m|g|s|A|K|cd|rad|px}
   // <N> = small integer
   // <n> = small integer, written in Unicode superscript numbers
   // <3N> = small integer multiple of 3
   // <3n> = small integer multiple of 3, written in Unicode superscript numbers
   // <cdot> = '.' or Unicode MIDDLE DOT character
   // note that "m" is the only difficult component to distinguish: if it's followed by a letter, it's the prefix!
   // TODO: accept more general strings, to let users type something? e.g. allow prefixes on any component?
   // TODO: allow alternative units? e.g. inches, Hz, ...
   constexpr char const* errorMessage = "Ill-formed Units string";
   // Note that testing for string[ii] always works, as it's a null-terminated string, so we can always
   // test the next character.
   dip::uint ii = 0;
   if(( string[ ii ] == '1' ) && ( string[ ii + 1 ] == '0' )) {
      // Starts with 10^{<N>|<n>}.
      ii += 2;
      int power;
      DIP_THROW_IF( !ParsePower( string, ii, power ), errorMessage );
      DIP_THROW_IF(( power == 0 ) || (( power % 3 ) != 0 ), errorMessage );
      DIP_THROW_IF( !ExpectCDot( string, ii ), errorMessage );
      power_[ unsigned( BaseUnits::THOUSANDS ) ] = static_cast< sint8 >( power_[ unsigned( BaseUnits::THOUSANDS ) ] + power / 3 );
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
#ifdef DIP__ENABLE_UNICODE
      case micron[ 0 ]:
         if( string[ ii + 1 ] == micron[ 1 ] ) { thousands = -2; ii += 2; } break; // the micron character takes 2 bytes
#endif
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
   power_[ unsigned( BaseUnits::THOUSANDS ) ] = static_cast< sint8 >( power_[ unsigned( BaseUnits::THOUSANDS ) ]  + thousands * power );
   power_[ unsigned( bu ) ] = static_cast< sint8 >( power_[ unsigned( bu ) ] + power );
   while( string[ ii ] != '\0' ) {
      bool neg = string[ ii ] == '/';
      if( neg ) {
         ++ii;
      } else {
         DIP_THROW_IF( !ExpectCDot( string, ii ), errorMessage );
      }
      DIP_THROW_IF( !ParseComponent( string, ii, bu, power ), errorMessage );
      power_[ unsigned( bu ) ] = static_cast< sint8 >( power_[ unsigned( bu ) ] + ( neg ? -power : power ));
   }
}

// --- WRITING UNITS ---

namespace {

std::string PowerAsString( dip::sint p ) {
#ifdef DIP__ENABLE_UNICODE
   std::string str = std::to_string( p );
   std::string out;
   for( auto c : str ) {
      switch( c ) {
         case '-': out += superN; break;
         case '0': out += super0; break;
         case '1': out += super1; break;
         case '2': out += super2; break;
         case '3': out += super3; break;
         case '4': out += super4; break;
         case '5': out += super5; break;
         case '6': out += super6; break;
         case '7': out += super7; break;
         case '8': out += super8; break;
         case '9': out += super9; break;
         default: out += c; break; // This shouldn't happen
      }
   }
   return out;
#else
   return "^" + std::to_string( p );
#endif
}

// Appends a unit with a positive power to the string `out`.
bool WritePositivePower( dip::String& out, const char* s, dip::sint p, bool prefix ) {
   if( p > 0 ) {
      if( prefix ) {
         out += cdot;
      }
      out += s;
      if( p != 1 ) {
         out += PowerAsString( p );
      }
      prefix = true;
   }
   return prefix;
}

// Appends a unit with a negative power to the string `out`.
bool WriteNegativePower( dip::String& out, const char* s, dip::sint p, bool prefix ) {
   if( p < 0 ) {
      if( prefix ) {
         out += '/';
         p = -p;
      }
      out += s;
      if( p != 1 ) {
         out += PowerAsString( p );
      }
      prefix = true;
   }
   return prefix;
}

} // namespace

// Creates a string representation of the units.
dip::String Units::String() const {
   dip::String out;
   bool prefix = false;
   // The prefix
   if( power_[ 0 ] != 0 ) {
      dip::sint p = FirstPower();
      dip::sint n = div_floor< dip::sint >( power_[ 0 ], p );
      if(( n < -5 ) || ( n > 6 )) {
         // We cannot print an SI prefix, just print a 10^n instead.
         n = 0;
         p = power_[ 0 ] * 3;
      } else {
         p = ( power_[ 0 ] - n * p ) * 3;     // dip::PhysicalQuantity should make sure that p is 0 here, using AdjustThousands()
      }
      if( p != 0 ) {
         out += "10" + PowerAsString( p );
         prefix = true;
      }
      if( n != 0 ) {
         if( prefix ) {
            out += cdot;
         }
#ifdef DIP__ENABLE_UNICODE
         switch( n ) {
            case -5: out += "f"; break;
            case -4: out += "p"; break;
            case -3: out += "n"; break;
            case -2: out += micron; break; // This is two bytes, so we can't do the trick we do when plain ASCII is enabled.
            case -1: out += "m"; break;
            default: // Should not happen!
            case 0: break;
            case 1: out += "k"; break;
            case 2: out += "M"; break;
            case 3: out += "G"; break;
            case 4: out += "T"; break;
            case 5: out += "P"; break;
            case 6: out += "E"; break;
         }
#else
         constexpr char const* prefixes = "fpnum kMGTPE";
         out += prefixes[ n + 5 ];
#endif
         prefix = false; // next thing should not output a cdot first.
      }
   }
   // We write out positive powers first
   prefix = WritePositivePower( out, "m",   power_[ unsigned( BaseUnits::LENGTH            ) ], prefix );
   prefix = WritePositivePower( out, "g",   power_[ unsigned( BaseUnits::MASS              ) ], prefix );
   prefix = WritePositivePower( out, "s",   power_[ unsigned( BaseUnits::TIME              ) ], prefix );
   prefix = WritePositivePower( out, "A",   power_[ unsigned( BaseUnits::CURRENT           ) ], prefix );
   prefix = WritePositivePower( out, "K",   power_[ unsigned( BaseUnits::TEMPERATURE       ) ], prefix );
   prefix = WritePositivePower( out, "cd",  power_[ unsigned( BaseUnits::LUMINOUSINTENSITY ) ], prefix );
   prefix = WritePositivePower( out, "rad", power_[ unsigned( BaseUnits::ANGLE             ) ], prefix );
   prefix = WritePositivePower( out, "px",  power_[ unsigned( BaseUnits::PIXEL             ) ], prefix );
   // and negative powers at the end
   prefix = WriteNegativePower( out, "m",   power_[ unsigned( BaseUnits::LENGTH            ) ], prefix );
   prefix = WriteNegativePower( out, "g",   power_[ unsigned( BaseUnits::MASS              ) ], prefix );
   prefix = WriteNegativePower( out, "s",   power_[ unsigned( BaseUnits::TIME              ) ], prefix );
   prefix = WriteNegativePower( out, "A",   power_[ unsigned( BaseUnits::CURRENT           ) ], prefix );
   prefix = WriteNegativePower( out, "K",   power_[ unsigned( BaseUnits::TEMPERATURE       ) ], prefix );
   prefix = WriteNegativePower( out, "cd",  power_[ unsigned( BaseUnits::LUMINOUSINTENSITY ) ], prefix );
   prefix = WriteNegativePower( out, "rad", power_[ unsigned( BaseUnits::ANGLE             ) ], prefix );
   prefix = WriteNegativePower( out, "px",  power_[ unsigned( BaseUnits::PIXEL             ) ], prefix );
   return out;
}


} // namespace dip



#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"

DOCTEST_TEST_CASE("[DIPlib] testing the dip::Units class") {
   // We only test the string conversion functions here
   // Other workings are tested at the same time as dip::PhysicalQuantity below

#ifdef DIP__ENABLE_UNICODE

   dip::Units f = dip::Units::Meter();
   DOCTEST_CHECK( ( f ).String() == "m" );
   DOCTEST_CHECK( ( f * f ).String() == u8"m\u00B2" );
   DOCTEST_CHECK( ( f * f * f ).String() == u8"m\u00B3" );
   DOCTEST_CHECK( ( f * f * f * f ).String() == u8"m\u2074" );
   DOCTEST_CHECK( ( dip::Units() / f ).String() == u8"m\u207B\u00B9" );
   DOCTEST_CHECK( ( dip::Units() / f / f ).String() == u8"m\u207B\u00B2" );
   DOCTEST_CHECK( ( dip::Units() / f / f / f ).String() == u8"m\u207B\u00B3" );
   DOCTEST_CHECK( ( dip::Units() / f / f / f / f ).String() == u8"m\u207B\u2074" );
   DOCTEST_CHECK( f == dip::Units( "m" ));
   DOCTEST_CHECK( f * f == dip::Units( u8"m\u00B2" ));
   DOCTEST_CHECK( f * f * f == dip::Units( u8"m\u00B3" ));
   DOCTEST_CHECK( f * f * f * f == dip::Units( u8"m\u2074" ));
   DOCTEST_CHECK( dip::Units() / f == dip::Units( u8"m\u207B\u00B9" ));
   DOCTEST_CHECK( dip::Units() / f / f == dip::Units( u8"m\u207B\u00B2" ));
   DOCTEST_CHECK( dip::Units() / f / f / f == dip::Units( u8"m\u207B\u00B3" ));
   DOCTEST_CHECK( dip::Units() / f / f / f / f == dip::Units( u8"m\u207B\u2074" ));

   dip::Units g = dip::Units::Second();
   DOCTEST_CHECK( ( f / g ).String() == "m/s" );
   DOCTEST_CHECK( ( f / g / g ).String() == u8"m/s\u00B2" );
   DOCTEST_CHECK( ( f / g / g / g ).String() == u8"m/s\u00B3" );
   DOCTEST_CHECK( ( f / g / g / g / g ).String() == u8"m/s\u2074" );
   DOCTEST_CHECK( ( g / f ).String() == "s/m" );
   DOCTEST_CHECK( ( g / f / f ).String() == u8"s/m\u00B2" );
   DOCTEST_CHECK( ( g * g / f ).String() == u8"s\u00B2/m" );
   DOCTEST_CHECK( ( g * f ).String() == u8"m\u00B7s" );
   DOCTEST_CHECK( f / g == dip::Units( "m/s" ));
   DOCTEST_CHECK( f / g / g == dip::Units( u8"m/s\u00B2" ));
   DOCTEST_CHECK( f / g / g / g == dip::Units( u8"m/s\u00B3" ));
   DOCTEST_CHECK( f / g / g / g / g == dip::Units( u8"m/s\u2074" ));
   DOCTEST_CHECK( g / f == dip::Units( "s/m" ));
   DOCTEST_CHECK( g / f / f == dip::Units( u8"s/m\u00B2" ));
   DOCTEST_CHECK( g * g / f == dip::Units( u8"s\u00B2/m" ));
   DOCTEST_CHECK( g * f == dip::Units( u8"m\u00B7s" ));

   DOCTEST_CHECK( ( dip::Units::Millimeter() ).String() == "mm" );
   DOCTEST_CHECK( ( dip::Units::Millimeter() * dip::Units::Millimeter() ).String() == u8"mm\u00B2" );
   DOCTEST_CHECK( ( dip::Units::Millimeter() * dip::Units::Meter() ).String() == u8"10\u00B3\u00B7mm\u00B2" );
   DOCTEST_CHECK( ( dip::Units::Kilometer() * dip::Units::Meter() ).String() == u8"10\u00B3\u00B7m\u00B2" );
   DOCTEST_CHECK( dip::Units::Millimeter() == dip::Units( "mm" ));
   DOCTEST_CHECK( dip::Units::Millimeter() * dip::Units::Millimeter() == dip::Units( u8"mm\u00B2" ));
   DOCTEST_CHECK( dip::Units::Millimeter() * dip::Units::Meter() == dip::Units( u8"10\u00B3\u00B7mm\u00B2" ));
   DOCTEST_CHECK( dip::Units::Kilometer() * dip::Units::Meter() == dip::Units( u8"10\u00B3\u00B7m\u00B2" ));

   DOCTEST_CHECK( ( dip::Units( u8"10\u2076\u00B7mm\u00B2" )).String() == u8"m\u00B2" );
   DOCTEST_CHECK( ( dip::Units( "km/s" )).String() == "km/s" );
   DOCTEST_CHECK( ( dip::Units( u8"km\u00B7cd\u00B7rad\u00B7px" )).String() == u8"km\u00B7cd\u00B7rad\u00B7px" );
   DOCTEST_CHECK( ( dip::Units( u8"km\u00B7cd/rad\u00B7px" )).String() == u8"km\u00B7cd\u00B7px/rad" );
   DOCTEST_CHECK( ( dip::Units( u8"10\u00B3\u00B7km\u207B\u00B9\u00B7cd\u207B\u00B2/K" )).String() == u8"m\u207B\u00B9/K/cd\u00B2" );

   DOCTEST_CHECK( ( dip::Units( "10^6.mm^2" )).String() == u8"m\u00B2" );
   DOCTEST_CHECK( ( dip::Units( "km.cd.rad.px" )).String() == u8"km\u00B7cd\u00B7rad\u00B7px" );
   DOCTEST_CHECK( ( dip::Units( "km.cd/rad.px" )).String() == u8"km\u00B7cd\u00B7px/rad" );
   DOCTEST_CHECK( ( dip::Units( "10^3.km^-1.cd^-2/K" )).String() == u8"m\u207B\u00B9/K/cd\u00B2" );

#else

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

#endif

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
