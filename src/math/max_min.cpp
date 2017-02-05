/*
 * DIPlib 3.0
 * This file contains definitions for image statistics functions.
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"
#include "diplib/math.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {

//
template< typename TPI >
class dip__GetMaximumAndMinimum : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         MaximumAndMinimum& vars = varsArray[ params.thread ];
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
      dip__GetMaximumAndMinimum( std::vector< MaximumAndMinimum >& varsArray ) : varsArray( varsArray ) {}
   private:
      std::vector< MaximumAndMinimum >& varsArray;
};

MaximumAndMinimum GetMaximumAndMinimum(
      Image const& in,
      Image const& mask
) {
   ImageConstRefArray inar;
   inar.reserve( 2 );
   if( in.DataType().IsComplex() ) {
      inar.push_back( in.QuickCopy().SplitComplex() );
   } else {
      inar.push_back( in );
   }
   DataTypeArray inBufT;
   inBufT.reserve( 2 );
   inBufT.push_back( in.DataType() );
   if( mask.IsForged() ) {
      // If we have a mask, add it to the input array.
      DIP_START_STACK_TRACE
         mask.CheckIsMask( in.Sizes(), Option::AllowSingletonExpansion::DO_ALLOW, Option::ThrowException::DO_THROW );
      DIP_END_STACK_TRACE
      inar.push_back( mask );
      inBufT.push_back( DT_BIN );
   }
   // Create an array for the values computed by each thread, and initialize them.
   std::vector< MaximumAndMinimum > varsArray;
   // Find the right overload for our data type
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_NONCOMPLEX( scanLineFilter, dip__GetMaximumAndMinimum, ( varsArray ), in.DataType() );
   // Call the framework function
   ImageRefArray outar{};
   Framework::Scan(
         inar, outar,
         inBufT, DataTypeArray{}, DataTypeArray{}, UnsignedArray{},
         scanLineFilter.get(), Framework::Scan_TensorAsSpatialDim );
   // Reduce
   MaximumAndMinimum out = varsArray[ 0 ];
   for( dip::uint ii = 1; ii < varsArray.size(); ++ii ) {
      out.max = std::max( out.max, varsArray[ ii ].max );
      out.min = std::min( out.min, varsArray[ ii ].min );
   }
   return out;
}


} // namespace dip
