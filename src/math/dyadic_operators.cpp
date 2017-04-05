/*
 * DIPlib 3.0
 * This file contains implements dyadic operators.
 *
 * (c)2017, Cris Luengo.
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

#include <array>

#include "diplib.h"
#include "diplib/math.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {

void Atan2( Image const& y, Image const& x, Image& out ) {
   DIP_THROW_IF( !y.IsScalar() || !x.IsScalar(), E::IMAGE_NOT_SCALAR );
   DataType dt = DataType::SuggestArithmetic( y.DataType(), x.DataType() );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_FLOAT( scanLineFilter, Framework::NewDyadicScanLineFilter, (
         []( auto its ) { return std::atan2( *its[ 0 ], *its[ 1 ] ); }
   ), dt );
   ImageRefArray outar{ out };
   Framework::Scan( { y, x }, outar, { dt, dt }, { dt }, { dt }, { 1 }, *scanLineFilter, Framework::Scan_TensorAsSpatialDim );
}

void Hypot( Image const& a, Image const& b, Image& out ) {
   DIP_THROW_IF( !a.IsScalar() || !b.IsScalar(), E::IMAGE_NOT_SCALAR );
   DataType dt = DataType::SuggestArithmetic( a.DataType(), b.DataType() );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_FLOAT( scanLineFilter, Framework::NewDyadicScanLineFilter, (
         []( auto its ) { return std::hypot( *its[ 0 ], *its[ 1 ] ); }
   ), dt );
   ImageRefArray outar{ out };
   Framework::Scan( { a, b }, outar, { dt, dt }, { dt }, { dt }, { 1 }, *scanLineFilter, Framework::Scan_TensorAsSpatialDim );
}

} // namespace dip
