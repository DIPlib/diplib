/*
 * DIPlib 3.0
 * This file contains the definition for the GetMaximumAndMinimum function.
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"
#include "diplib/math.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {

namespace {

template< typename TPI >
class dip__GetMaximumAndMinimum : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         MinMaxAccumulator& vars = varsArray[ params.thread ];
         auto bufferLength = params.bufferLength;
         auto inStride = params.inBuffer[ 0 ].stride;
         if( params.inBuffer.size() > 1 ) {
            // If there's two input buffers, we have a mask image.
            auto maskStride = params.inBuffer[ 1 ].stride;
            bin const* mask = static_cast< bin const* >( params.inBuffer[ 1 ].buffer );
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               if( *mask ) {
                  vars.Push( *in );
               }
               in += inStride;
               mask += maskStride;
            }
         } else {
            // Otherwise we don't.
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               vars.Push( *in );
               in += inStride;
            }
         }
      }
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         varsArray.resize( threads );
      }
      dip__GetMaximumAndMinimum( std::vector< MinMaxAccumulator >& varsArray ) : varsArray( varsArray ) {}
   private:
      std::vector< MinMaxAccumulator >& varsArray;
};

} // namespace

MinMaxAccumulator GetMaximumAndMinimum(
      Image const& in,
      Image const& mask
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   // Create an array for the values computed by each thread, and initialize them.
   std::vector< MinMaxAccumulator > varsArray;
   // Find the right overload for our data type
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_NONCOMPLEX( scanLineFilter, dip__GetMaximumAndMinimum, ( varsArray ), in.DataType() );
   // Call the framework function
   Framework::ScanSingleInput( in, mask, in.DataType(), *scanLineFilter, Framework::Scan_TensorAsSpatialDim );
   // Reduce
   MinMaxAccumulator out = varsArray[ 0 ];
   for( dip::uint ii = 1; ii < varsArray.size(); ++ii ) {
      out += varsArray[ ii ];
   }
   return out;
}


} // namespace dip
