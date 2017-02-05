/*
 * DIPlib 3.0
 * This file contains the definition the bit-wise operators.
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {

//
void And(
      Image const& lhs,
      Image const& rhs,
      Image& out
) {
   DataType dt = lhs.DataType();
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_INT_OR_BIN( scanLineFilter, Framework::NewDyadicScanLineFilter, (
         []( auto its ) { return *its[ 0 ] & *its[ 1 ]; }
   ), dt );
   Framework::ScanDyadic( lhs, rhs, out, dt, dt, scanLineFilter.get() );
}

//
void Or(
      Image const& lhs,
      Image const& rhs,
      Image& out
) {
   DataType dt = lhs.DataType();
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_INT_OR_BIN( scanLineFilter, Framework::NewDyadicScanLineFilter, (
         []( auto its ) { return *its[ 0 ] | *its[ 1 ]; }
   ), dt );
   Framework::ScanDyadic( lhs, rhs, out, dt, dt, scanLineFilter.get() );
}

//
void Xor(
      Image const& lhs,
      Image const& rhs,
      Image& out
) {
   DataType dt = lhs.DataType();
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_INT_OR_BIN( scanLineFilter, Framework::NewDyadicScanLineFilter, (
         []( auto its ) { return *its[ 0 ] ^ *its[ 1 ]; }
   ), dt );
   Framework::ScanDyadic( lhs, rhs, out, dt, dt, scanLineFilter.get() );
}

//
void Not(
      Image const& in,
      Image& out
) {
   DataType dt = in.DataType();
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_INT_OR_BIN( scanLineFilter, Framework::NewMonadicScanLineFilter, (
         []( auto its ) { return ~*its[ 0 ]; }
   ), dt );
   Framework::ScanMonadic( in, out, dt, dt, 1, scanLineFilter.get(), Framework::Scan_TensorAsSpatialDim );
}


} // namespace dip
