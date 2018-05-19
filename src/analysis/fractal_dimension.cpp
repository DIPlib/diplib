/*
 * DIPlib 3.0
 * This file contains definitions for fractal dimension estimation
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
#include "diplib/accumulators.h"
#include "diplib/analysis.h"
#include "diplib/statistics.h"
#include "diplib/morphology.h"

namespace dip {

namespace {

std::vector< dip::uint > ComputeBoxSizes(
      UnsignedArray const& sizes,
      dfloat eta
) {
   dfloat L = static_cast< dfloat >( *std::min_element( sizes.begin(), sizes.end() )) / 2.0;
   std::vector< dip::uint > out( 2, 1 );
   out[ 1 ] = 2;  // first two scales: 1, 2
   eta += 1.0;
   dfloat size = eta * eta;
   while( size <= L ) {
      dip::uint sz = static_cast< dip::uint >( round_cast( size ));
      if( sz > out.back() ) { // don't put two identical sizes on there
         out.push_back( sz );
      }
      size *= eta;
   }
   return out;
}

} // namespace

dfloat FractalDimension(
      Image const& in,
      dfloat eta
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsBinary(), E::IMAGE_NOT_BINARY );
   dip::uint nDims = in.Dimensionality();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF(( eta <= 0.0 ) || ( eta > 1.0 ), E::PARAMETER_OUT_OF_RANGE );

   // Set up box sizes
   auto sizes = ComputeBoxSizes( in.Sizes(), eta );
   dip::uint n = sizes.size();
   DIP_THROW_IF( n < 3, "Image is too small, too few levels generated" );
   DIP_ASSERT( sizes[ 0 ] == 1 );
   DIP_ASSERT( sizes[ 1 ] == 2 );

   // Compute box counts for boxes of sizes Delta
   CovarianceAccumulator acc;
   // size = 1
   double count = Sum( in ).As< double >();
   if( count == 0 ) {
      return 0; // If this count is not 0, then others won't either.
   }
   //std::cout << "size = 1, count = " << count << std::endl;
   acc.Push( std::log( 1.0 ), std::log( count ));
   // size = 2
   dip::Image boxes = Dilation( in, { 2, S::RECTANGULAR } );
   count = Sum( boxes ).As< double >();
   //std::cout << "size = 2, count = " << count << std::endl;
   acc.Push( std::log( 2.0 ), std::log( count ));
   bool mirror = true; // We need to mirror every other even size kernel, to keep the composition centered.
   dfloat numPixels = static_cast< dfloat >( boxes.NumberOfPixels() );
   // other sizes
   for( dip::uint ii = 2; ii < n; ++ii ) {
      dip::uint size = sizes[ ii ];
      if( count < numPixels ) {
         // else: We're saturated. Larger scales will yield the same count, so there's no need to compute them.
         dip::uint delta = size - sizes[ ii - 1 ] + 1;
         StructuringElement se( static_cast< dfloat >( delta ), S::RECTANGULAR );
         if(( delta & 1 ) == 0 ) {
            if( mirror ) {
               se.Mirror();
            }
            mirror = !mirror;
         }
         Dilation( boxes, boxes, se );
         count = Sum( boxes ).As< double >();
      }
      //std::cout << "size = " << size << ", count = " << count << std::endl;
      acc.Push( std::log( static_cast< dfloat >( size )), std::log( count ));
   }

   // Compute least squares fit (linear regression)
   dfloat T = acc.Slope();
   dfloat D = static_cast< dfloat >( nDims );
   // number of pixels grow as sz^D. Fractal dimension is given by D-T.
   return clamp( D - T, 0.0, D ); // The clamp should really not be necessary, but you never know...
}

} // namespace dip
