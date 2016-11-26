/*
 * DIPlib 3.0
 * This file contains definitions for image math and statistics functions.
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
static void dip__GetMaximumAndMinimum(
      std::vector< Framework::ScanBuffer > const& inBuffer,
      std::vector< Framework::ScanBuffer >& outBuffer,
      dip::uint bufferLength,
      dip::uint dimension,
      UnsignedArray const& position,
      const void* functionParameters,
      void* functionVariables
) {
   TPI const* in = static_cast< TPI const* >( inBuffer[ 0 ].buffer );
   MaximumAndMinimum* vars = ( MaximumAndMinimum* )functionVariables;
   if( inBuffer.size() > 1 ) {
      // If there's two input buffers, we have a mask image.
      bin const* mask = static_cast< bin const* >( inBuffer[ 1 ].buffer );
      for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
         if( * mask ) {
            double v = * in;
            if( v < vars->min ) {
               vars->min = v;
            }
            if( v > vars->max ) {
               vars->max = v;
            }
         }
         in += inBuffer[ 0 ].stride;
         mask += inBuffer[ 1 ].stride;
      }
   } else {
      // Otherwise we don't.
      for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
         double v = * in;
         if( v < vars->min ) {
            vars->min = v;
         }
         if( v > vars->max ) {
            vars->max = v;
         }
         in += inBuffer[ 0 ].stride;
      }
   }
}

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
      DIP_THROW_IF( !mask.DataType().IsBinary(), dip::E::MASK_NOT_BINARY );
      DIP_THROW_IF( mask.Dimensionality() > in.Dimensionality(), dip::E::MASK_TOO_MANY_DIMENSIONS );
      // It's OK for the mask to have fewer dimensions, singleton expansion will take care of it.
      // Note that the scan function will do the other checks.
      inar.push_back( mask );
      inBufT.push_back( DT_BIN );
   }
   Framework::ScanFilter filter;
   DIP_OVL_ASSIGN_NONCOMPLEX( filter, dip__GetMaximumAndMinimum, in.DataType() );
   // Create an array for the values computed by each thread, and initialize them.
   MaximumAndMinimum init{
         std::numeric_limits< double >::lowest(),
         std::numeric_limits< double >::max()
   };
   std::vector< MaximumAndMinimum > vars( 1, init ); // TODO: fill in the max number of threads we expect to create.
   // Call the framework function
   ImageRefArray outar{};
   Framework::Scan(
         inar, outar,
         inBufT, DataTypeArray{}, DataTypeArray{}, UnsignedArray{},
         filter, nullptr, Framework::CastToVoidpVector( vars ),
         Framework::Scan_TensorAsSpatialDim );
   // Reduce
   MaximumAndMinimum out = vars[ 0 ];
   for( dip::uint ii = 1; ii < vars.size(); ++ii ) {
      out.max = std::max( out.max, vars[ ii ].max );
      out.min = std::min( out.min, vars[ ii ].min );
   }
   return out;
}

} // namespace dip
