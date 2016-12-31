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
class dip__Count : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         dip::uint& count = counts[ params.thread ];
         auto bufferLength = params.bufferLength;
         auto inStride = params.inBuffer[ 0 ].stride;
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            if( *in != TPI( 0 ) ) {
               ++count;
            }
            in += inStride;
         }
      }
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         counts.resize( threads );
      }
      dip__Count( std::vector< dip::uint >& counts ) : counts( counts ) {}
   private:
      std::vector< dip::uint >& counts;
};

dip::uint Count(
      Image const& in
) {
   DIP_THROW_IF( !in.IsScalar(), E::NOT_SCALAR );
   std::vector< dip::uint > counts;
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_NONCOMPLEX( scanLineFilter, dip__Count, ( counts ), in.DataType() );
   // Call the framework function
   ImageRefArray outar{};
   Framework::ScanSingleInput( in, in.DataType(), scanLineFilter.get(), Framework::Scan_NoSingletonExpansion );
   // Reduce
   dip::uint out = counts[ 0 ];
   for( dip::uint ii = 1; ii < counts.size(); ++ii ) {
      out += counts[ ii ];
   }
   return out;
}

} // namespace dip
