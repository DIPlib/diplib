/*
 * (c)2017, Wouter Caarls
 * (c)2024-2026, Cris Luengo
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

#include <array>

#include "diplib.h"
#include "diplib/private/robin_map.h"
#include "diplib/viewer/viewer.h"

namespace dip {
namespace viewer {

namespace {

template< dip::uint n >
String toString( dip::uint idx, std::array< char const*, n > const& options ) {
   DIP_THROW_IF( idx >= n, dip::E::INDEX_OUT_OF_RANGE );
   return options[ idx ];
}

template< dip::uint n >
dip::uint toIndex( String const& str, std::array< char const*, n > const& options ) {
   for( dip::uint idx = 0; idx < n; ++idx ) {
      if( options[ idx ] == str ) {
         return idx;
      }
   }
   DIP_THROW_INVALID_FLAG( str );
}

using StringMap = tsl::robin_map< String, String >;

String lookupAlias( String const& str, StringMap const& map ) {
   auto it = map.find( str );
   if( it == map.end() ) {
      return str;
   }
   return it->second;
}

//
// NOTE!!!
// These string arrays below must match 1:1 with the corresponding enums in include/diplib/viewer/viewer.h
//

constexpr std::array< char const*, 4 > complexToRealStrings{{ "real", "imag", "magnitude", "phase" }};
constexpr std::array< char const*, 4 > complexToRealDescriptions{{ "real part", "imaginary part", "magnitude (abs)", "phase" }};

constexpr std::array< char const*, 7 > mappingStrings{{ "unit", "angle", "8bit", "mod", "lin", "base", "log" }};
constexpr std::array< char const*, 7 > mappingDescriptions{{ "unit", "angle", "normal", "modulo", "linear", "symmetric around 0", "logarithmic" }};

const StringMap mappingAliases{
      { "normal", "8bit" },
      { "linear", "lin" },
      { "all", "lin" },
      { "based", "base" },
      { "modulo", "mod" },
      { "logarithmic", "log" },
};

constexpr std::array< char const*, 4 > projectionStrings{{ "none", "min", "mean", "max" }};
constexpr std::array< char const*, 4 > projectionDescriptions{{ "none (slice)", "minimum", "mean", "maximum" }};

constexpr std::array< char const*, 7 > lookupTableStrings{{ "original", "ternary", "grey", "sequential", "divergent", "periodic", "labels" }};
constexpr std::array< char const*, 7 > lookupTableDescriptions{{ "image colorspace (mapping inactive)", "ternary (RGB)", "grey-value", "perceptually linear", "divergent blue-yellow", "cyclic", "labels" }};

const StringMap lutAliases{
      { "linear", "sequential" },
      { "diverging", "divergent" },
      { "cyclic", "periodic" },
      { "label", "labels" },
      { "gray", "grey" },
   };

} // namespace

ViewingOptions::ComplexToReal ViewingOptions::TranslateComplexToRealString( String const& string ) {
   return static_cast< ViewingOptions::ComplexToReal >( toIndex( string, complexToRealStrings ));
}
String ViewingOptions::ComplexToRealAsString( ComplexToReal value ) {
   return toString( static_cast< dip::uint >( value ), complexToRealStrings );
}
String ViewingOptions::ComplexToRealDescription( ComplexToReal value ) {
   return toString( static_cast< dip::uint >( value ), complexToRealDescriptions );
}

ViewingOptions::Mapping ViewingOptions::TranslateMappingString( String const& string ) {
   auto str = lookupAlias( string , mappingAliases );
   return static_cast< ViewingOptions::Mapping >( toIndex( str, mappingStrings ));
}
String ViewingOptions::MappingAsString( Mapping value ) {
   return toString( static_cast< dip::uint >( value ), mappingStrings );
}
String ViewingOptions::MappingDescription( Mapping value ) {
   return toString( static_cast< dip::uint >( value ), mappingDescriptions );
}

ViewingOptions::Projection ViewingOptions::TranslateProjectionString( String const& string ) {
   if( string == "slice" ) {
      return ViewingOptions::Projection::None;
   }
   return static_cast< ViewingOptions::Projection >( toIndex( string, projectionStrings ));
}
String ViewingOptions::ProjectionAsString( Projection value ) {
   return toString( static_cast< dip::uint >( value ), projectionStrings );
}
String ViewingOptions::ProjectionDescription( Projection value ) {
   return toString( static_cast< dip::uint >( value ), projectionDescriptions );
}

ViewingOptions::LookupTable ViewingOptions::TranslateLookupTableString( String const& string ) {
   auto str = lookupAlias( string , lutAliases );
   return static_cast< ViewingOptions::LookupTable >( toIndex( str, lookupTableStrings ));
}
String ViewingOptions::LookupTableAsString( LookupTable value ) {
   return toString( static_cast< dip::uint >( value ), lookupTableStrings );
}
String ViewingOptions::LookupTableDescription( LookupTable value ) {
   return toString( static_cast< dip::uint >( value ), lookupTableDescriptions );
}

} // namespace viewer
} // namespace dip
