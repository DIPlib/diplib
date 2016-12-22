/*
 * DIPlib 3.0
 * This file contains definitions for image statistics functions.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"
#include "diplib/math.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {


//
template< typename TPI >
static void dip__Count(
      std::vector< Framework::ScanBuffer > const& inBuffer,
      std::vector< Framework::ScanBuffer >&,
      dip::uint bufferLength,
      dip::uint,
      UnsignedArray const&,
      const void*,
      void* functionVariables
) {
   TPI const* in = static_cast< TPI const* >( inBuffer[ 0 ].buffer );
   dip::uint* sum = ( dip::uint* )functionVariables;
   for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
      if( *in != TPI( 0 )) {
         ++( *sum );
      }
      in += inBuffer[ 0 ].stride;
   }
}

dip::uint Count(
      Image const& in
) {
   DIP_THROW_IF( !in.IsScalar(), E::NOT_SCALAR );
   Framework::ScanFilter lineFilter;
   DIP_OVL_ASSIGN_ALL( lineFilter, dip__Count, in.DataType() );
   // Create an array for the values computed by each thread, and initialize them.
   std::vector< dip::uint > vars( 1, 0 ); // TODO: fill in the max number of threads we expect to create.
   // Call the framework function
   ImageRefArray outar{};
   Framework::ScanSingleInput(
         in,
         in.DataType(),
         lineFilter,
         nullptr,
         Framework::CastToVoidpVector( vars ),
         Framework::Scan_NoSingletonExpansion );
   // Reduce
   dip::uint out = vars[ 0 ];
   for( dip::uint ii = 1; ii < vars.size(); ++ii ) {
      out += vars[ ii ];
   }
   return out;
}

} // namespace dip
