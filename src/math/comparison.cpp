/*
 * DIPlib 3.0
 * This file contains the definition the bit-wise operators.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"
#include "dip_framework.h"
#include "dip_overload.h"

namespace dip {

//
template< typename TPI >
static void dip__Equal(
      const std::vector<Framework::ScanBuffer>&   inBuffer,
      std::vector<Framework::ScanBuffer>&         outBuffer,
      dip::uint            bufferLength,
      dip::uint            dimension,
      UnsignedArray        position,
      const void*          functionParameters,
      void*                functionVariables
) {
   const TPI* lhs = (const TPI*)inBuffer[0].buffer;
   const TPI* rhs = (const TPI*)inBuffer[1].buffer;
   bin* out = (bin*)outBuffer[0].buffer;
   for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
      for( dip::uint jj = 0; jj < outBuffer[0].tensorLength; ++jj ) { // all 3 buffers have same number of tensor elements
         out[jj * outBuffer[0].tensorStride] = lhs[jj * inBuffer[0].tensorStride] ==
                                               rhs[jj * inBuffer[1].tensorStride];
      }
      lhs += inBuffer[0].stride;
      rhs += inBuffer[1].stride;
      out += outBuffer[0].stride;
   }
}

void Equal(
      const Image& lhs,
      const Image& rhs,
      Image& out
) {
   DataType dt = DataType::SuggestDiadicOperation( lhs.DataType(), rhs.DataType() );
   Framework::ScanFilter filter;
   DIP_OVL_ASSIGN_ALL( filter, dip__Equal, dt );
   std::vector<void*> vars;
   Framework::ScanOptions opts;
   Framework::ScanDyadic( lhs, rhs, out, dt, DT_BIN, filter, nullptr, vars, opts );
}


//
template< typename TPI >
static void dip__NotEqual(
      const std::vector<Framework::ScanBuffer>&   inBuffer,
      std::vector<Framework::ScanBuffer>&         outBuffer,
      dip::uint            bufferLength,
      dip::uint            dimension,
      UnsignedArray        position,
      const void*          functionParameters,
      void*                functionVariables
) {
   const TPI* lhs = (const TPI*)inBuffer[0].buffer;
   const TPI* rhs = (const TPI*)inBuffer[1].buffer;
   bin* out = (bin*)outBuffer[0].buffer;
   for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
      for( dip::uint jj = 0; jj < outBuffer[0].tensorLength; ++jj ) { // all 3 buffers have same number of tensor elements
         out[jj * outBuffer[0].tensorStride] = lhs[jj * inBuffer[0].tensorStride] !=
                                               rhs[jj * inBuffer[1].tensorStride];
      }
      lhs += inBuffer[0].stride;
      rhs += inBuffer[1].stride;
      out += outBuffer[0].stride;
   }
}

void NotEqual(
      const Image& lhs,
      const Image& rhs,
      Image& out
) {
   DataType dt = DataType::SuggestDiadicOperation( lhs.DataType(), rhs.DataType() );
   Framework::ScanFilter filter;
   DIP_OVL_ASSIGN_ALL( filter, dip__NotEqual, dt );
   std::vector<void*> vars;
   Framework::ScanOptions opts;
   Framework::ScanDyadic( lhs, rhs, out, dt, DT_BIN, filter, nullptr, vars, opts );
}


//
template< typename TPI >
static void dip__Lesser(
      const std::vector<Framework::ScanBuffer>&   inBuffer,
      std::vector<Framework::ScanBuffer>&         outBuffer,
      dip::uint            bufferLength,
      dip::uint            dimension,
      UnsignedArray        position,
      const void*          functionParameters,
      void*                functionVariables
) {
   const TPI* lhs = (const TPI*)inBuffer[0].buffer;
   const TPI* rhs = (const TPI*)inBuffer[1].buffer;
   bin* out = (bin*)outBuffer[0].buffer;
   for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
      for( dip::uint jj = 0; jj < outBuffer[0].tensorLength; ++jj ) { // all 3 buffers have same number of tensor elements
         out[jj * outBuffer[0].tensorStride] = lhs[jj * inBuffer[0].tensorStride] <
                                               rhs[jj * inBuffer[1].tensorStride];
      }
      lhs += inBuffer[0].stride;
      rhs += inBuffer[1].stride;
      out += outBuffer[0].stride;
   }
}

void Lesser(
      const Image& lhs,
      const Image& rhs,
      Image& out
) {
   DataType dt = DataType::SuggestDiadicOperation( lhs.DataType(), rhs.DataType() );
   Framework::ScanFilter filter;
   DIP_OVL_ASSIGN_NONCOMPLEX( filter, dip__Lesser, dt );
   std::vector<void*> vars;
   Framework::ScanOptions opts;
   Framework::ScanDyadic( lhs, rhs, out, dt, DT_BIN, filter, nullptr, vars, opts );
}


//
template< typename TPI >
static void dip__Greater(
      const std::vector<Framework::ScanBuffer>&   inBuffer,
      std::vector<Framework::ScanBuffer>&         outBuffer,
      dip::uint            bufferLength,
      dip::uint            dimension,
      UnsignedArray        position,
      const void*          functionParameters,
      void*                functionVariables
) {
   const TPI* lhs = (const TPI*)inBuffer[0].buffer;
   const TPI* rhs = (const TPI*)inBuffer[1].buffer;
   bin* out = (bin*)outBuffer[0].buffer;
   for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
      for( dip::uint jj = 0; jj < outBuffer[0].tensorLength; ++jj ) { // all 3 buffers have same number of tensor elements
         out[jj * outBuffer[0].tensorStride] = lhs[jj * inBuffer[0].tensorStride] >
                                               rhs[jj * inBuffer[1].tensorStride];
      }
      lhs += inBuffer[0].stride;
      rhs += inBuffer[1].stride;
      out += outBuffer[0].stride;
   }
}

void Greater(
      const Image& lhs,
      const Image& rhs,
      Image& out
) {
   DataType dt = DataType::SuggestDiadicOperation( lhs.DataType(), rhs.DataType() );
   Framework::ScanFilter filter;
   DIP_OVL_ASSIGN_NONCOMPLEX( filter, dip__Greater, dt );
   std::vector<void*> vars;
   Framework::ScanOptions opts;
   Framework::ScanDyadic( lhs, rhs, out, dt, DT_BIN, filter, nullptr, vars, opts );
}


//
template< typename TPI >
static void dip__NotGreater(
      const std::vector<Framework::ScanBuffer>&   inBuffer,
      std::vector<Framework::ScanBuffer>&         outBuffer,
      dip::uint            bufferLength,
      dip::uint            dimension,
      UnsignedArray        position,
      const void*          functionParameters,
      void*                functionVariables
) {
   const TPI* lhs = (const TPI*)inBuffer[0].buffer;
   const TPI* rhs = (const TPI*)inBuffer[1].buffer;
   bin* out = (bin*)outBuffer[0].buffer;
   for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
      for( dip::uint jj = 0; jj < outBuffer[0].tensorLength; ++jj ) { // all 3 buffers have same number of tensor elements
         out[jj * outBuffer[0].tensorStride] = lhs[jj * inBuffer[0].tensorStride] <=
                                               rhs[jj * inBuffer[1].tensorStride];
      }
      lhs += inBuffer[0].stride;
      rhs += inBuffer[1].stride;
      out += outBuffer[0].stride;
   }
}

void NotGreater(
      const Image& lhs,
      const Image& rhs,
      Image& out
) {
   DataType dt = DataType::SuggestDiadicOperation( lhs.DataType(), rhs.DataType() );
   Framework::ScanFilter filter;
   DIP_OVL_ASSIGN_NONCOMPLEX( filter, dip__NotGreater, dt );
   std::vector<void*> vars;
   Framework::ScanOptions opts;
   Framework::ScanDyadic( lhs, rhs, out, dt, DT_BIN, filter, nullptr, vars, opts );
}


//
template< typename TPI >
static void dip__NotLesser(
      const std::vector<Framework::ScanBuffer>&   inBuffer,
      std::vector<Framework::ScanBuffer>&         outBuffer,
      dip::uint            bufferLength,
      dip::uint            dimension,
      UnsignedArray        position,
      const void*          functionParameters,
      void*                functionVariables
) {
   const TPI* lhs = (const TPI*)inBuffer[0].buffer;
   const TPI* rhs = (const TPI*)inBuffer[1].buffer;
   bin* out = (bin*)outBuffer[0].buffer;
   for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
      for( dip::uint jj = 0; jj < outBuffer[0].tensorLength; ++jj ) { // all 3 buffers have same number of tensor elements
         out[jj * outBuffer[0].tensorStride] = lhs[jj * inBuffer[0].tensorStride] >=
                                               rhs[jj * inBuffer[1].tensorStride];
      }
      lhs += inBuffer[0].stride;
      rhs += inBuffer[1].stride;
      out += outBuffer[0].stride;
   }
}

void NotLesser(
      const Image& lhs,
      const Image& rhs,
      Image& out
) {
   DataType dt = DataType::SuggestDiadicOperation( lhs.DataType(), rhs.DataType() );
   Framework::ScanFilter filter;
   DIP_OVL_ASSIGN_NONCOMPLEX( filter, dip__NotLesser, dt );
   std::vector<void*> vars;
   Framework::ScanOptions opts;
   Framework::ScanDyadic( lhs, rhs, out, dt, DT_BIN, filter, nullptr, vars, opts );
}


} // namespace dip
