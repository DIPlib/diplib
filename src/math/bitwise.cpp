/*
 * DIPlib 3.0
 * This file contains the definition the bit-wise operators.
 *
 * (c)2016-2017, Cris Luengo.
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
   Framework::ScanDyadic( lhs, rhs, out, dt, dt, *scanLineFilter );
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
   Framework::ScanDyadic( lhs, rhs, out, dt, dt, *scanLineFilter );
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
   Framework::ScanDyadic( lhs, rhs, out, dt, dt, *scanLineFilter );
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
   Framework::ScanMonadic( in, out, dt, dt, 1, *scanLineFilter, Framework::Scan_TensorAsSpatialDim );
}


} // namespace dip
