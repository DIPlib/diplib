/*
 * DIPlib 3.0
 * This file contains definitions of functions that implement the finite difference filter.
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

#include "diplib.h"
#include "diplib/linear.h"
#include "diplib/framework.h"

namespace dip {

void FiniteDifference(
      Image const& in,
      Image& out,
      UnsignedArray order,
      String const& smoothFlag,
      StringArray const& boundaryCondition,
      BooleanArray process
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = in.Dimensionality();
   DIP_START_STACK_TRACE
      ArrayUseParameter( order, nDims, dip::uint( 0 ));
      ArrayUseParameter( process, nDims, true );
   DIP_END_STACK_TRACE
   bool smooth = smoothFlag == "smooth";
   OneDimensionalFilterArray filter( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( process[ ii ] ) {
         switch( order[ ii ] ) {
            case 0:
               if( smooth ) {
                  filter[ ii ].symmetry = "even";
                  filter[ ii ].filter = { 0.25, 0.5 };
               } else {
                  process[ ii ] = false;
               }
               break;
            case 1:
               filter[ ii ].symmetry = "odd";
               filter[ ii ].filter = { 0.5, 0.0 };
               break;
            case 2:
               filter[ ii ].symmetry = "even";
               filter[ ii ].filter = { 1.0, -2.0 };
               break;
            default:
               DIP_THROW( "Finite difference filter not implemented for order > 3" );
         }
         // NOTE: origin defaults to the middle of the filter, so we don't need to set it explicitly here.
      }
   }
   SeparableConvolution( in, out, filter, boundaryCondition, process );
}

} // namespace dip

#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/statistics.h"
#include "diplib/iterators.h"

DOCTEST_TEST_CASE("[DIPlib] testing the finite difference filter") {
   dip::Image img{ dip::UnsignedArray{ 256 }, 1, dip::DT_DFLOAT };
   img.Fill( 0.0 );
   img.At( 128 ) = 1.0;
   dip::Image out = dip::FiniteDifference( img, { 0 } );
   DOCTEST_CHECK( out.At( 128 ) == 0.5 );
   DOCTEST_CHECK( dip::Sum( out ).As< dip::dfloat >() == doctest::Approx( 1.0 ));
   dip::ImageIterator< dip::dfloat > it( img );
   for( dip::dfloat x = -128; it; ++it, ++x ) {
      *it = x;
   }
   out = dip::FiniteDifference( img, { 1 } );
   DOCTEST_CHECK( out.At( 128 ).As< dip::dfloat >() == doctest::Approx( 1.0 ));
   img = img * img;
   out = dip::FiniteDifference( img, { 2 } );
   DOCTEST_CHECK( out.At( 128 ).As< dip::dfloat >() == doctest::Approx( 2.0 ));
}

#endif // DIP__ENABLE_DOCTEST
