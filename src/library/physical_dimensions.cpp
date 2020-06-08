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

#ifdef DIP_CONFIG_ENABLE_UNICODE

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
      power = std::atoi( &( string[ ii ] ));
      ii += n;
   }
#ifdef DIP_CONFIG_ENABLE_UNICODE
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
#ifdef DIP_CONFIG_ENABLE_UNICODE
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
#ifdef DIP_CONFIG_ENABLE_UNICODE
      case micron[ 0 ]:
         if( string[ ii + 1 ] == micron[ 1 ] ) { thousands = -2; ii += 2; } break; // the micron character takes 2 bytes
#endif
      case 'm':
         // If the next character is a letter, then this is "milli" prefix, otherwise it's "meter" units.
         // Windows uses signed characters, so we cast to `unsigned char` first, then to `int` as expected by `isalpha`.
         if( std::isalpha( static_cast< int >( static_cast< unsigned char >( string[ ii + 1 ] )))) { thousands = -1; ++ii; } break;
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

void Units::FromString( dip::String const& string ) {

   // First, make sure the string is not empty
   if( string.empty() ) {
      *this = Units();
      return;
   }

   // Next, check for some generic names
   dip::String modifiedString = string;
   ToLowerCase( modifiedString );
   if( modifiedString.back() == 's' ) {
      // Remove trailing s, so that both "meter" and "meters" yields the same thing
      modifiedString.pop_back();
   }
   if( modifiedString == "meter" ) {
      *this = Meter();
      return;
   }
   if( modifiedString == "squaremeter" ) {
      *this = SquareMeter();
      return;
   }
   if( modifiedString == "cubicmeter" ) {
      *this = CubicMeter();
      return;
   }
   if( modifiedString == "nanometer" ) {
      *this = Nanometer();
      return;
   }
   if( modifiedString == "micrometer" ) {
      *this = Micrometer();
      return;
   }
   if( modifiedString == "millimeter" ) {
      *this = Millimeter();
      return;
   }
   if( modifiedString == "kilometer" ) {
      *this = Kilometer();
      return;
   }
   if( modifiedString == "squaremicrometer" ) {
      *this = SquareMicrometer();
      return;
   }
   if( modifiedString == "squaremillimeter" ) {
      *this = SquareMillimeter();
      return;
   }
   if( modifiedString == "cubicmillimeter" ) {
      *this = CubicMillimeter();
      return;
   }

   if( modifiedString == "second" ) {
      *this = Second();
      return;
   }
   if( modifiedString == "millisecond" ) {
      *this = Millisecond();
      return;
   }
   if( modifiedString == "hertz" ) {
      *this = Hertz();
      return;
   }
   if( modifiedString == "kilohertz" ) {
      *this = Kilohertz();
      return;
   }
   if( modifiedString == "megahertz" ) {
      *this = Megahertz();
      return;
   }
   if( modifiedString == "gigahertz" ) {
      *this = Gigahertz();
      return;
   }

   if( modifiedString == "radian" ) {
      *this = Radian();
      return;
   }

   if( modifiedString == "pixel" ) {
      *this = Pixel();
      return;
   }
   if( modifiedString == "squarepixel" ) {
      *this = SquarePixel();
      return;
   }
   if( modifiedString == "cubicpixel" ) {
      *this = CubicPixel();
      return;
   }

   // Finally, assume it's a string in our own format, and call the constructor
   DIP_STACK_TRACE_THIS( *this = Units( string ));
}

// --- WRITING UNITS ---

namespace {

constexpr char const* CDot() {
   return ".";
}

constexpr char const* CDotUnicode() {
#ifdef DIP_CONFIG_ENABLE_UNICODE
   return cdot;
#else
   return CDot();
#endif
}

std::string PowerAsString( dip::sint p ) {
   return "^" + std::to_string( p );
}

std::string PowerAsStringUnicode( dip::sint p ) {
#ifdef DIP_CONFIG_ENABLE_UNICODE
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
   return PowerAsString( p );
#endif
}

// Appends an SI prefix to the string `out`.
void WritePrefix( dip::String& out, dip::sint n ) {
   constexpr char const* prefixes = "fpnum kMGTPE";
   out += prefixes[ n + 5 ];
}

void WritePrefixUnicode( dip::String& out, dip::sint n ) {
#ifdef DIP_CONFIG_ENABLE_UNICODE
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
   WritePrefix( out, n );
#endif
}

// Appends a unit with a positive power to the string `out`.
bool WritePositivePower( dip::String& out, const char* s, dip::sint p, bool prefix, bool unicode ) {
   if( p > 0 ) {
      if( prefix ) {
         out += unicode ? CDotUnicode() : CDot();
      }
      out += s;
      if( p != 1 ) {
         out += unicode ? PowerAsStringUnicode( p ) : PowerAsString( p );
      }
      prefix = true;
   }
   return prefix;
}

// Appends a unit with a negative power to the string `out`.
bool WriteNegativePower( dip::String& out, const char* s, dip::sint p, bool prefix, bool unicode ) {
   if( p < 0 ) {
      if( prefix ) {
         out += '/';
         p = -p;
      }
      out += s;
      if( p != 1 ) {
         out += unicode ? PowerAsStringUnicode( p ) : PowerAsString( p );
      }
      prefix = true;
   }
   return prefix;
}

} // namespace

// Creates a string representation of the units.
dip::String Units::StringRepresentation( bool unicode ) const {
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
         out += "10" + ( unicode ? PowerAsStringUnicode( p ) : PowerAsString( p ));
         prefix = true;
      }
      if( n != 0 ) {
         if( prefix ) {
            out += unicode ? CDotUnicode() : CDot();
         }
         unicode ? WritePrefixUnicode( out, n ) : WritePrefix( out, n );
         prefix = false; // next thing should not output a cdot first.
      }
   }
   // We write out positive powers first
   prefix = WritePositivePower( out, "m",   power_[ unsigned( BaseUnits::LENGTH            ) ], prefix, unicode );
   prefix = WritePositivePower( out, "g",   power_[ unsigned( BaseUnits::MASS              ) ], prefix, unicode );
   prefix = WritePositivePower( out, "s",   power_[ unsigned( BaseUnits::TIME              ) ], prefix, unicode );
   prefix = WritePositivePower( out, "A",   power_[ unsigned( BaseUnits::CURRENT           ) ], prefix, unicode );
   prefix = WritePositivePower( out, "K",   power_[ unsigned( BaseUnits::TEMPERATURE       ) ], prefix, unicode );
   prefix = WritePositivePower( out, "cd",  power_[ unsigned( BaseUnits::LUMINOUSINTENSITY ) ], prefix, unicode );
   prefix = WritePositivePower( out, "rad", power_[ unsigned( BaseUnits::ANGLE             ) ], prefix, unicode );
   prefix = WritePositivePower( out, "px",  power_[ unsigned( BaseUnits::PIXEL             ) ], prefix, unicode );
   // and negative powers at the end
   prefix = WriteNegativePower( out, "m",   power_[ unsigned( BaseUnits::LENGTH            ) ], prefix, unicode );
   prefix = WriteNegativePower( out, "g",   power_[ unsigned( BaseUnits::MASS              ) ], prefix, unicode );
   prefix = WriteNegativePower( out, "s",   power_[ unsigned( BaseUnits::TIME              ) ], prefix, unicode );
   prefix = WriteNegativePower( out, "A",   power_[ unsigned( BaseUnits::CURRENT           ) ], prefix, unicode );
   prefix = WriteNegativePower( out, "K",   power_[ unsigned( BaseUnits::TEMPERATURE       ) ], prefix, unicode );
   prefix = WriteNegativePower( out, "cd",  power_[ unsigned( BaseUnits::LUMINOUSINTENSITY ) ], prefix, unicode );
   prefix = WriteNegativePower( out, "rad", power_[ unsigned( BaseUnits::ANGLE             ) ], prefix, unicode );
   prefix = WriteNegativePower( out, "px",  power_[ unsigned( BaseUnits::PIXEL             ) ], prefix, unicode );
   return out;
}


} // namespace dip


#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"

DOCTEST_TEST_CASE("[DIPlib] testing the dip::Units class") {
   // Note: further tested at the same time as dip::PhysicalQuantity below
   DOCTEST_SUBCASE("Constexpr arithmetic") {
      // We add static_assert tests here to make sure it's all really compile-time computations.
      constexpr dip::Units a = dip::Units::SquareMillimeter();
      constexpr dip::Units b = dip::Units::Gigahertz();
      static_assert( a != b, "A bug in dip::Units!" );
      constexpr dip::Units c = a * b;
      constexpr dip::sint t1 = c.Thousands();
      static_assert( t1 == 1, "A bug in dip::Units!" );
      DOCTEST_CHECK( t1 == 1 );
      constexpr dip::Units d = dip::Units::SquareMillimeter().Power( -1 );
      constexpr dip::sint t2 = d.Thousands();
      static_assert( t2 == 2, "A bug in dip::Units!" );
      DOCTEST_CHECK( t2 == 2 );
      static_assert( c / b == a, "A bug in dip::Units!" );
      DOCTEST_CHECK( c / b == a );
      static_assert( !a.IsDimensionless(), "A bug in dip::Units!" );
      DOCTEST_CHECK( !a.IsDimensionless() );
      static_assert( dip::Units().IsDimensionless(), "A bug in dip::Units!" );
      DOCTEST_CHECK( dip::Units().IsDimensionless() );
      static_assert( a.IsPhysical(), "A bug in dip::Units!" );
      DOCTEST_CHECK( a.IsPhysical() );
      static_assert( !dip::Units().IsPhysical(), "A bug in dip::Units!" );
      DOCTEST_CHECK( !dip::Units().IsPhysical() );
      static_assert( a.HasSameDimensions( dip::Units::SquareMeter() ), "A bug in dip::Units!" );
      DOCTEST_CHECK( a.HasSameDimensions( dip::Units::SquareMeter() ));
   }
   DOCTEST_SUBCASE("String conversion") {

#ifdef DIP_CONFIG_ENABLE_UNICODE

      dip::Units f = dip::Units::Meter();
      DOCTEST_CHECK(( f ).String() == "m" );
      DOCTEST_CHECK(( f ).StringUnicode() == "m" );
      DOCTEST_CHECK(( f * f ).String() == "m^2" );
      DOCTEST_CHECK(( f * f ).StringUnicode() == u8"m\u00B2" );
      DOCTEST_CHECK(( f * f * f ).String() == "m^3" );
      DOCTEST_CHECK(( f * f * f ).StringUnicode() == u8"m\u00B3" );
      DOCTEST_CHECK(( f * f * f * f ).String() == "m^4" );
      DOCTEST_CHECK(( f * f * f * f ).StringUnicode() == u8"m\u2074" );
      DOCTEST_CHECK(( dip::Units() / f ).String() == "m^-1" );
      DOCTEST_CHECK(( dip::Units() / f ).StringUnicode() == u8"m\u207B\u00B9" );
      DOCTEST_CHECK(( dip::Units() / f / f ).String() == "m^-2" );
      DOCTEST_CHECK(( dip::Units() / f / f ).StringUnicode() == u8"m\u207B\u00B2" );
      DOCTEST_CHECK(( dip::Units() / f / f / f ).String() == "m^-3" );
      DOCTEST_CHECK(( dip::Units() / f / f / f ).StringUnicode() == u8"m\u207B\u00B3" );
      DOCTEST_CHECK(( dip::Units() / f / f / f / f ).String() == "m^-4" );
      DOCTEST_CHECK(( dip::Units() / f / f / f / f ).StringUnicode() == u8"m\u207B\u2074" );
      DOCTEST_CHECK( f == dip::Units( "m" ));
      DOCTEST_CHECK( f * f == dip::Units( "m^2" ));
      DOCTEST_CHECK( f * f == dip::Units( u8"m\u00B2" ));
      DOCTEST_CHECK( f * f * f == dip::Units( "m^3" ));
      DOCTEST_CHECK( f * f * f == dip::Units( u8"m\u00B3" ));
      DOCTEST_CHECK( f * f * f * f == dip::Units( "m^4" ));
      DOCTEST_CHECK( f * f * f * f == dip::Units( u8"m\u2074" ));
      DOCTEST_CHECK( dip::Units() / f == dip::Units( "m^-1" ));
      DOCTEST_CHECK( dip::Units() / f == dip::Units( u8"m\u207B\u00B9" ));
      DOCTEST_CHECK( dip::Units() / f / f == dip::Units( "m^-2" ));
      DOCTEST_CHECK( dip::Units() / f / f == dip::Units( u8"m\u207B\u00B2" ));
      DOCTEST_CHECK( dip::Units() / f / f / f == dip::Units( "m^-3" ));
      DOCTEST_CHECK( dip::Units() / f / f / f == dip::Units( u8"m\u207B\u00B3" ));
      DOCTEST_CHECK( dip::Units() / f / f / f / f == dip::Units( "m^-4" ));
      DOCTEST_CHECK( dip::Units() / f / f / f / f == dip::Units( u8"m\u207B\u2074" ));

      dip::Units g = dip::Units::Second();
      DOCTEST_CHECK(( f / g ).String() == "m/s" );
      DOCTEST_CHECK(( f / g ).StringUnicode() == "m/s" );
      DOCTEST_CHECK(( f / g / g ).String() == "m/s^2" );
      DOCTEST_CHECK(( f / g / g ).StringUnicode() == u8"m/s\u00B2" );
      DOCTEST_CHECK(( f / g / g / g ).String() == "m/s^3" );
      DOCTEST_CHECK(( f / g / g / g ).StringUnicode() == u8"m/s\u00B3" );
      DOCTEST_CHECK(( f / g / g / g / g ).String() == "m/s^4" );
      DOCTEST_CHECK(( f / g / g / g / g ).StringUnicode() == u8"m/s\u2074" );
      DOCTEST_CHECK(( g / f ).String() == "s/m" );
      DOCTEST_CHECK(( g / f ).StringUnicode() == "s/m" );
      DOCTEST_CHECK(( g / f / f ).String() == "s/m^2" );
      DOCTEST_CHECK(( g / f / f ).StringUnicode() == u8"s/m\u00B2" );
      DOCTEST_CHECK(( g * g / f ).String() == "s^2/m" );
      DOCTEST_CHECK(( g * g / f ).StringUnicode() == u8"s\u00B2/m" );
      DOCTEST_CHECK(( g * f ).String() == "m.s" );
      DOCTEST_CHECK(( g * f ).StringUnicode() == u8"m\u00B7s" );
      DOCTEST_CHECK( f / g == dip::Units( "m/s" ));
      DOCTEST_CHECK( f / g / g == dip::Units( "m/s^2" ));
      DOCTEST_CHECK( f / g / g == dip::Units( u8"m/s\u00B2" ));
      DOCTEST_CHECK( f / g / g / g == dip::Units( "m/s^3" ));
      DOCTEST_CHECK( f / g / g / g == dip::Units( u8"m/s\u00B3" ));
      DOCTEST_CHECK( f / g / g / g / g == dip::Units( "m/s^4" ));
      DOCTEST_CHECK( f / g / g / g / g == dip::Units( u8"m/s\u2074" ));
      DOCTEST_CHECK( g / f == dip::Units( "s/m" ));
      DOCTEST_CHECK( g / f / f == dip::Units( "s/m^2" ));
      DOCTEST_CHECK( g / f / f == dip::Units( u8"s/m\u00B2" ));
      DOCTEST_CHECK( g * g / f == dip::Units( "s^2/m" ));
      DOCTEST_CHECK( g * g / f == dip::Units( u8"s\u00B2/m" ));
      DOCTEST_CHECK( g * f == dip::Units( "m.s" ));
      DOCTEST_CHECK( g * f == dip::Units( u8"m\u00B7s" ));

      DOCTEST_CHECK(( dip::Units::Millimeter() ).String() == "mm" );
      DOCTEST_CHECK(( dip::Units::Millimeter() ).StringUnicode() == "mm" );
      DOCTEST_CHECK(( dip::Units::Millimeter() * dip::Units::Millimeter() ).String() == "mm^2" );
      DOCTEST_CHECK(( dip::Units::Millimeter() * dip::Units::Millimeter() ).StringUnicode() == u8"mm\u00B2" );
      DOCTEST_CHECK(( dip::Units::Millimeter() * dip::Units::Meter() ).String() == "10^3.mm^2" );
      DOCTEST_CHECK(( dip::Units::Millimeter() * dip::Units::Meter() ).StringUnicode() == u8"10\u00B3\u00B7mm\u00B2" );
      DOCTEST_CHECK(( dip::Units::Kilometer() * dip::Units::Meter() ).String() == "10^3.m^2" );
      DOCTEST_CHECK(( dip::Units::Kilometer() * dip::Units::Meter() ).StringUnicode() == u8"10\u00B3\u00B7m\u00B2" );
      DOCTEST_CHECK( dip::Units::Millimeter() == dip::Units( "mm" ));
      DOCTEST_CHECK( dip::Units::Millimeter() * dip::Units::Millimeter() == dip::Units( "mm^2" ));
      DOCTEST_CHECK( dip::Units::Millimeter() * dip::Units::Millimeter() == dip::Units( u8"mm\u00B2" ));
      DOCTEST_CHECK( dip::Units::Millimeter() * dip::Units::Meter() == dip::Units( "10^3.mm^2" ));
      DOCTEST_CHECK( dip::Units::Millimeter() * dip::Units::Meter() == dip::Units( u8"10\u00B3\u00B7mm\u00B2" ));
      DOCTEST_CHECK( dip::Units::Kilometer() * dip::Units::Meter() == dip::Units( "10^3.m^2" ));
      DOCTEST_CHECK( dip::Units::Kilometer() * dip::Units::Meter() == dip::Units( u8"10\u00B3\u00B7m\u00B2" ));

      DOCTEST_CHECK(( dip::Units( "10^6.mm^2" )).String() == "m^2" );
      DOCTEST_CHECK(( dip::Units( "10^6.mm^2" )).StringUnicode() == u8"m\u00B2" );
      DOCTEST_CHECK(( dip::Units( u8"10\u2076\u00B7mm\u00B2" )).StringUnicode() == u8"m\u00B2" );
      DOCTEST_CHECK(( dip::Units( "km/s" )).String() == "km/s" );
      DOCTEST_CHECK(( dip::Units( "km/s" )).StringUnicode() == "km/s" );
      DOCTEST_CHECK(( dip::Units( "km.cd.rad.px" )).String() == "km.cd.rad.px" );
      DOCTEST_CHECK(( dip::Units( "km.cd.rad.px" )).StringUnicode() == u8"km\u00B7cd\u00B7rad\u00B7px" );
      DOCTEST_CHECK(( dip::Units( u8"km\u00B7cd\u00B7rad\u00B7px" )).StringUnicode() == u8"km\u00B7cd\u00B7rad\u00B7px" );
      DOCTEST_CHECK(( dip::Units( "km.cd/rad.px" )).String() == "km.cd.px/rad" );
      DOCTEST_CHECK(( dip::Units( "km.cd/rad.px" )).StringUnicode() == u8"km\u00B7cd\u00B7px/rad" );
      DOCTEST_CHECK(( dip::Units( u8"km\u00B7cd/rad\u00B7px" )).StringUnicode() == u8"km\u00B7cd\u00B7px/rad" );
      DOCTEST_CHECK(( dip::Units( "10^3.km^-1.cd^-2/K" )).String() == "m^-1/K/cd^2" );
      DOCTEST_CHECK(( dip::Units( "10^3.km^-1.cd^-2/K" )).StringUnicode() == u8"m\u207B\u00B9/K/cd\u00B2" );
      DOCTEST_CHECK(( dip::Units( u8"10\u00B3\u00B7km\u207B\u00B9\u00B7cd\u207B\u00B2/K" )).StringUnicode() == u8"m\u207B\u00B9/K/cd\u00B2" );

#else

      dip::Units f = dip::Units::Meter();
      DOCTEST_CHECK(( f ).String() == "m" );
      DOCTEST_CHECK(( f ).StringUnicode() == "m" );
      DOCTEST_CHECK(( f * f ).String() == "m^2" );
      DOCTEST_CHECK(( f * f ).StringUnicode() == "m^2" );
      DOCTEST_CHECK(( f * f * f ).String() == "m^3" );
      DOCTEST_CHECK(( f * f * f ).StringUnicode() == "m^3" );
      DOCTEST_CHECK(( f * f * f * f ).String() == "m^4" );
      DOCTEST_CHECK(( f * f * f * f ).StringUnicode() == "m^4" );
      DOCTEST_CHECK(( dip::Units() / f ).String() == "m^-1" );
      DOCTEST_CHECK(( dip::Units() / f ).StringUnicode() == "m^-1" );
      DOCTEST_CHECK(( dip::Units() / f / f ).String() == "m^-2" );
      DOCTEST_CHECK(( dip::Units() / f / f ).StringUnicode() == "m^-2" );
      DOCTEST_CHECK(( dip::Units() / f / f / f ).String() == "m^-3" );
      DOCTEST_CHECK(( dip::Units() / f / f / f ).StringUnicode() == "m^-3" );
      DOCTEST_CHECK(( dip::Units() / f / f / f / f ).String() == "m^-4" );
      DOCTEST_CHECK(( dip::Units() / f / f / f / f ).StringUnicode() == "m^-4" );
      DOCTEST_CHECK( f == dip::Units( "m" ));
      DOCTEST_CHECK( f * f == dip::Units( "m^2" ));
      DOCTEST_CHECK( f * f * f == dip::Units( "m^3" ));
      DOCTEST_CHECK( f * f * f * f == dip::Units( "m^4" ));
      DOCTEST_CHECK( dip::Units() / f == dip::Units( "m^-1" ));
      DOCTEST_CHECK( dip::Units() / f / f == dip::Units( "m^-2" ));
      DOCTEST_CHECK( dip::Units() / f / f / f == dip::Units( "m^-3" ));
      DOCTEST_CHECK( dip::Units() / f / f / f / f == dip::Units( "m^-4" ));

      dip::Units g = dip::Units::Second();
      DOCTEST_CHECK(( f / g ).String() == "m/s" );
      DOCTEST_CHECK(( f / g ).StringUnicode() == "m/s" );
      DOCTEST_CHECK(( f / g / g ).String() == "m/s^2" );
      DOCTEST_CHECK(( f / g / g ).StringUnicode() == "m/s^2" );
      DOCTEST_CHECK(( f / g / g / g ).String() == "m/s^3" );
      DOCTEST_CHECK(( f / g / g / g ).StringUnicode() == "m/s^3" );
      DOCTEST_CHECK(( f / g / g / g / g ).String() == "m/s^4" );
      DOCTEST_CHECK(( f / g / g / g / g ).StringUnicode() == "m/s^4" );
      DOCTEST_CHECK(( g / f ).String() == "s/m" );
      DOCTEST_CHECK(( g / f ).StringUnicode() == "s/m" );
      DOCTEST_CHECK(( g / f / f ).String() == "s/m^2" );
      DOCTEST_CHECK(( g / f / f ).StringUnicode() == "s/m^2" );
      DOCTEST_CHECK(( g * g / f ).String() == "s^2/m" );
      DOCTEST_CHECK(( g * g / f ).StringUnicode() == "s^2/m" );
      DOCTEST_CHECK(( g * f ).String() == "m.s" );
      DOCTEST_CHECK(( g * f ).StringUnicode() == "m.s" );
      DOCTEST_CHECK( f / g == dip::Units( "m/s" ));
      DOCTEST_CHECK( f / g / g == dip::Units( "m/s^2" ));
      DOCTEST_CHECK( f / g / g / g == dip::Units( "m/s^3" ));
      DOCTEST_CHECK( f / g / g / g / g == dip::Units( "m/s^4" ));
      DOCTEST_CHECK( g / f == dip::Units( "s/m" ));
      DOCTEST_CHECK( g / f / f == dip::Units( "s/m^2" ));
      DOCTEST_CHECK( g * g / f == dip::Units( "s^2/m" ));
      DOCTEST_CHECK( g * f == dip::Units( "m.s" ));

      DOCTEST_CHECK(( dip::Units::Millimeter() ).String() == "mm" );
      DOCTEST_CHECK(( dip::Units::Millimeter() ).StringUnicode() == "mm" );
      DOCTEST_CHECK(( dip::Units::Millimeter() * dip::Units::Millimeter() ).String() == "mm^2" );
      DOCTEST_CHECK(( dip::Units::Millimeter() * dip::Units::Millimeter() ).StringUnicode() == "mm^2" );
      DOCTEST_CHECK(( dip::Units::Millimeter() * dip::Units::Meter() ).String() == "10^3.mm^2" );
      DOCTEST_CHECK(( dip::Units::Millimeter() * dip::Units::Meter() ).StringUnicode() == "10^3.mm^2" );
      DOCTEST_CHECK(( dip::Units::Kilometer() * dip::Units::Meter() ).String() == "10^3.m^2" );
      DOCTEST_CHECK(( dip::Units::Kilometer() * dip::Units::Meter() ).StringUnicode() == "10^3.m^2" );
      DOCTEST_CHECK( dip::Units::Millimeter() == dip::Units( "mm" ));
      DOCTEST_CHECK( dip::Units::Millimeter() * dip::Units::Millimeter() == dip::Units( "mm^2" ));
      DOCTEST_CHECK( dip::Units::Millimeter() * dip::Units::Meter() == dip::Units( "10^3.mm^2" ));
      DOCTEST_CHECK( dip::Units::Kilometer() * dip::Units::Meter() == dip::Units( "10^3.m^2" ));

      DOCTEST_CHECK(( dip::Units( "10^6.mm^2" )).String() == "m^2" );
      DOCTEST_CHECK(( dip::Units( "10^6.mm^2" )).StringUnicode() == "m^2" );
      DOCTEST_CHECK(( dip::Units( "km/s" )).String() == "km/s" );
      DOCTEST_CHECK(( dip::Units( "km/s" )).StringUnicode() == "km/s" );
      DOCTEST_CHECK(( dip::Units( "km.cd.rad.px" )).String() == "km.cd.rad.px" );
      DOCTEST_CHECK(( dip::Units( "km.cd.rad.px" )).StringUnicode() == "km.cd.rad.px" );
      DOCTEST_CHECK(( dip::Units( "km.cd/rad.px" )).String() == "km.cd.px/rad" );
      DOCTEST_CHECK(( dip::Units( "km.cd/rad.px" )).StringUnicode() == "km.cd.px/rad" );
      DOCTEST_CHECK(( dip::Units( "10^3.km^-1.cd^-2/K" )).String() == "m^-1/K/cd^2" );
      DOCTEST_CHECK(( dip::Units( "10^3.km^-1.cd^-2/K" )).StringUnicode() == "m^-1/K/cd^2" );

#endif

   }
}

DOCTEST_TEST_CASE("[DIPlib] testing the dip::PhysicalQuantity class") {
   DOCTEST_SUBCASE("Constexpr arithmetic") {
      constexpr dip::PhysicalQuantity a = 50.0 * dip::Units::Nanometer();
      constexpr dip::PhysicalQuantity b = 0.5 * dip::Units::Micrometer();
      constexpr dip::PhysicalQuantity c = a * b;
      static_assert( c / b == a, "A bug in dip::PhysicalQuantity!" );
      DOCTEST_CHECK( c / b == a );
      constexpr dip::PhysicalQuantity d = a * 2.0;
      static_assert( d / 2.0 == a, "A bug in dip::PhysicalQuantity!" );
      DOCTEST_CHECK( d / 2.0 == a );
      constexpr dip::PhysicalQuantity e = a.Invert();
      static_assert( 1.0 / e == a, "A bug in dip::PhysicalQuantity!" );
      DOCTEST_CHECK( 1.0 / e == a );
      static_assert( a.HasSameDimensions( b ), "A bug in dip::PhysicalQuantity!" );
      DOCTEST_CHECK( a.HasSameDimensions( b ));
      constexpr dip::PhysicalQuantity f = a + b;
      static_assert( f.HasSameDimensions( a ), "A bug in dip::PhysicalQuantity!" );
      DOCTEST_CHECK( f.HasSameDimensions( b ));
      static_assert( !a.IsDimensionless(), "A bug in dip::PhysicalQuantity!" );
      DOCTEST_CHECK( !a.IsDimensionless() );
      static_assert( a.IsPhysical(), "A bug in dip::PhysicalQuantity!" );
      DOCTEST_CHECK( a.IsPhysical() );
      constexpr dip::PhysicalQuantity g1 = 100.0 * dip::Units::Nanometer();
      constexpr dip::PhysicalQuantity g2 = 0.1 * dip::Units::Micrometer();
      static_assert( g1 != g2, "A bug in dip::PhysicalQuantity!" );
      DOCTEST_CHECK( g1 != g2 );
      constexpr dip::PhysicalQuantity h1 = ( 100.0 * dip::Units::Nanometer() ).RemovePrefix();
      constexpr dip::PhysicalQuantity h2 = ( 0.1 * dip::Units::Micrometer() ).RemovePrefix();
      static_assert( h1.ApproximatelyEquals( h2 ), "A bug in dip::PhysicalQuantity!" );
      DOCTEST_CHECK( h1.ApproximatelyEquals( h2 ));
   }
   DOCTEST_SUBCASE("Arithmetic") {
      dip::PhysicalQuantity a = 50 * dip::Units::Nanometer();
      dip::PhysicalQuantity b = .4 * dip::Units::Micrometer();
      DOCTEST_CHECK( a + b == b + a );
      DOCTEST_CHECK( a + a == 2 * a );
      DOCTEST_CHECK( a * a == a.Power( 2 ));
      DOCTEST_CHECK(( 1 / ( a * a )) == a.Power( -2 ));
      dip::PhysicalQuantity c( 100, dip::Units::Second() );
      std::cout << c << '\n'; // TODO: GCC 8.2 makes the next test fail, but not if I add this line here.
      DOCTEST_CHECK(( 1 / c ) == c.Power( -1 ));
      DOCTEST_CHECK(( b / c ) == b * c.Power( -1 ));
      dip::PhysicalQuantity d = 180 * dip::PhysicalQuantity::Degree();
      DOCTEST_CHECK( d.ApproximatelyEquals( dip::pi * dip::PhysicalQuantity::Radian() ));
      DOCTEST_CHECK_THROWS( c + d );
   }
   DOCTEST_SUBCASE("Normalization") {
      dip::PhysicalQuantity f = dip::PhysicalQuantity::Meter();
      std::cout << f << '\n'; // TODO: GCC 8.1 makes a lot of tests below fail, but not if I add this line here (fixed in GCC 8.2, back in GCC 8.3 !?!?!).
      DOCTEST_CHECK(( f * 0 ).Normalize().magnitude == 0 );
      DOCTEST_CHECK(( f * 1 ).Normalize().magnitude == 1 );
      DOCTEST_CHECK(( f * 0.1 ).Normalize().magnitude == 0.1 );
      DOCTEST_CHECK(( f * 0.01 ).Normalize().magnitude == 10 );
      DOCTEST_CHECK(( f * 0.001 ).Normalize().magnitude == 1 );
      DOCTEST_CHECK(( f * 0.0001 ).Normalize().magnitude == 0.1 );
      DOCTEST_CHECK(( f * 0.00001 ).Normalize().magnitude == 10 );
      DOCTEST_CHECK(( f * 0.000001 ).Normalize().magnitude == 1 );
      DOCTEST_CHECK(( f * 0.0000001 ).Normalize().magnitude == doctest::Approx( 0.1 ));
      DOCTEST_CHECK(( f * 0.00000001 ).Normalize().magnitude == 10 );
      DOCTEST_CHECK(( f * 0.000000001 ).Normalize().magnitude == 1 );
      DOCTEST_CHECK(( f * 0.0000000001 ).Normalize().magnitude == doctest::Approx( 0.1 ));
      DOCTEST_CHECK(( f * 10 ).Normalize().magnitude == 10 );
      DOCTEST_CHECK(( f * 100 ).Normalize().magnitude == 0.1 );
      DOCTEST_CHECK(( f * 1000 ).Normalize().magnitude == 1 );
      DOCTEST_CHECK(( f * 10000 ).Normalize().magnitude == 10 );
      DOCTEST_CHECK(( f * 100000 ).Normalize().magnitude == doctest::Approx( 0.1 ));
      DOCTEST_CHECK(( f * 1000000 ).Normalize().magnitude == 1 );
      DOCTEST_CHECK(( f * 10000000 ).Normalize().magnitude == 10 );
      DOCTEST_CHECK(( f * 100000000 ).Normalize().magnitude == doctest::Approx( 0.1 ));
      DOCTEST_CHECK(( f * 1000000000 ).Normalize().magnitude == 1 );
      DOCTEST_CHECK(( f * f * 1 ).Normalize().magnitude == 1 );
      DOCTEST_CHECK(( f * f * 10 ).Normalize().magnitude == 10 );
      DOCTEST_CHECK(( f * f * 100 ).Normalize().magnitude == 100 );
      DOCTEST_CHECK(( f * f * 1000 ).Normalize().magnitude == 1000 );
      DOCTEST_CHECK(( f * f * 10000 ).Normalize().magnitude == 10000 );
      DOCTEST_CHECK(( f * f * 100000 ).Normalize().magnitude == doctest::Approx( 0.1 ));
      DOCTEST_CHECK(( f * f * 1000000 ).Normalize().magnitude == 1 );
      DOCTEST_CHECK(( f * f * 10000000 ).Normalize().magnitude == 10 );
      DOCTEST_CHECK(( f * f * 100000000 ).Normalize().magnitude == 100 );
      DOCTEST_CHECK(( f * f * 1000000000 ).Normalize().magnitude == 1000 );
      DOCTEST_CHECK(( -f ).Normalize().magnitude == -1 );

      DOCTEST_CHECK(( f * 0 ).Normalize().units.Thousands() == 0 );
      DOCTEST_CHECK(( f * 1 ).Normalize().units.Thousands() == 0 );
      DOCTEST_CHECK(( f * 0.1 ).Normalize().units.Thousands() == 0 );
      DOCTEST_CHECK(( f * 0.01 ).Normalize().units.Thousands() == -1 );
      DOCTEST_CHECK(( f * 0.001 ).Normalize().units.Thousands() == -1 );
      DOCTEST_CHECK(( f * 0.0001 ).Normalize().units.Thousands() == -1 );
      DOCTEST_CHECK(( f * 0.00001 ).Normalize().units.Thousands() == -2 );
      DOCTEST_CHECK(( f * 0.000001 ).Normalize().units.Thousands() == -2 );
      DOCTEST_CHECK(( f * 0.0000001 ).Normalize().units.Thousands() == -2 );
      DOCTEST_CHECK(( f * 0.00000001 ).Normalize().units.Thousands() == -3 );
      DOCTEST_CHECK(( f * 0.000000001 ).Normalize().units.Thousands() == -3 );
      DOCTEST_CHECK(( f * 0.0000000001 ).Normalize().units.Thousands() == -3 );
      DOCTEST_CHECK(( f * 10 ).Normalize().units.Thousands() == 0 );
      DOCTEST_CHECK(( f * 100 ).Normalize().units.Thousands() == 1 );
      DOCTEST_CHECK(( f * 1000 ).Normalize().units.Thousands() == 1 );
      DOCTEST_CHECK(( f * 10000 ).Normalize().units.Thousands() == 1 );
      DOCTEST_CHECK(( f * 100000 ).Normalize().units.Thousands() == 2 );
      DOCTEST_CHECK(( f * 1000000 ).Normalize().units.Thousands() == 2 );
      DOCTEST_CHECK(( f * 10000000 ).Normalize().units.Thousands() == 2 );
      DOCTEST_CHECK(( f * 100000000 ).Normalize().units.Thousands() == 3 );
      DOCTEST_CHECK(( f * 1000000000 ).Normalize().units.Thousands() == 3 );
      DOCTEST_CHECK(( f * f * 1 ).Normalize().units.Thousands() == 0 );
      DOCTEST_CHECK(( f * f * 10 ).Normalize().units.Thousands() == 0 );
      DOCTEST_CHECK(( f * f * 100 ).Normalize().units.Thousands() == 0 );
      DOCTEST_CHECK(( f * f * 1000 ).Normalize().units.Thousands() == 0 );
      DOCTEST_CHECK(( f * f * 10000 ).Normalize().units.Thousands() == 0 );
      DOCTEST_CHECK(( f * f * 100000 ).Normalize().units.Thousands() == 2 );
      DOCTEST_CHECK(( f * f * 1000000 ).Normalize().units.Thousands() == 2 );
      DOCTEST_CHECK(( f * f * 10000000 ).Normalize().units.Thousands() == 2 );
      DOCTEST_CHECK(( f * f * 100000000 ).Normalize().units.Thousands() == 2 );
      DOCTEST_CHECK(( f * f * 1000000000 ).Normalize().units.Thousands() == 2 );
      DOCTEST_CHECK(( -f ).Normalize().units.Thousands() == 0 );
   }
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
