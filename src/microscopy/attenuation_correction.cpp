/*
 * DIPlib 3.0
 * This file contains functions to apply attenuation correction
 *
 * (c)2018, Cris Luengo.
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
#include "diplib/microscopy.h"
#include "diplib/generation.h"
#include "diplib/math.h"
#include "diplib/statistics.h"
#include "diplib/framework.h"

namespace dip {

namespace {

struct ExponentialParams {
   dfloat a;
   dfloat b;
};

ExponentialParams ExponentialFit(
      dfloat* y,
      dfloat* var,
      dip::uint start,
      dip::uint stop
) {
   // Determine weights
   std::vector< dfloat > weights( stop, 1 );
   if( var ) {
      for( dip::uint ii = start; ii < stop; ii++ ) {
         dfloat v = std::max( var[ ii ], 1e-3 );
         weights[ ii ] = 1.0 / ( v * v );
      }
   }
   // Convert y to ln(y)
   for( dip::uint ii = start; ii < stop; ii++ ) {
      if( y[ ii ] > 0 ) {
         y[ ii ] = std::log( y[ ii ] );
      } else {
         y[ ii ] = 0;
         weights[ ii ] = 0; // Ignore this one
      }
   }
   dfloat sum_w = 0.0;
   dfloat sum_x = 0.0;
   dfloat sum_y = 0.0;
   for( dip::uint ii = start; ii < stop; ii++ ) {
      sum_w += weights[ ii ];
      sum_x += static_cast< dfloat >( ii ) * weights[ ii ];
      sum_y += y[ ii ] * weights[ ii ];
   }
   dfloat mean_x = sum_w == 0 ? 0 : sum_x / sum_w;
   dfloat sum_t2 = 0.0;
   ExponentialParams out{ 0.0, 0.0 };
   for( dip::uint ii = start; ii < stop; ii++ ) {
      dfloat w = std::sqrt( weights[ ii ] );
      dfloat t = ( static_cast< dfloat >( ii ) - mean_x ) * w;
      sum_t2 += t * t;
      out.b += t * y[ ii ] * w;
   }
   out.b = sum_t2 == 0 ? 0 : ( out.b / sum_t2 );
   out.a = sum_w == 0 ? 0 : (( sum_y - sum_x * ( out.b )) / sum_w );
   return out;
}

} // namespace

void ExponentialFitCorrection(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat percentile,
      String const& fromWhere,
      dfloat hysteresis,
      String const& weighting
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   if( in.Dimensionality() < 3 ) {
      Copy( in, out );
      return;
   }
   DIP_THROW_IF( in.Dimensionality() != 3, E::DIMENSIONALITY_NOT_SUPPORTED );

   // Compute mean or percentile projections
   Image fitData( { 1, 1, in.Size( 2 ) }, 1, DT_DFLOAT ); // Allocated to make sure `Mean` copies data, even if `in` has the same size and data type.
   fitData.Protect();
   Image fitVariance;
   if(( percentile < 0 ) || ( percentile > 100 )) {
      DIP_STACK_TRACE_THIS( Mean( in, mask, fitData, "", { true, true, false } ));
      if(( weighting == "variance" ) && ( in.Size( 0 ) * in.Size( 1 ) > 1 )) { // Need at least 2 values to compute variance
         fitVariance.SetDataType( DT_DFLOAT );
         fitVariance.Protect();
         DIP_STACK_TRACE_THIS( StandardDeviation( in, mask, fitVariance, S::FAST, { true, true, false } ));
      } else if( weighting != "none" ) {
         DIP_THROW_INVALID_FLAG( weighting );
      }
   } else {
      DIP_STACK_TRACE_THIS( Percentile( in, mask, fitData, percentile, { true, true, false } ));
   }

   DIP_ASSERT( fitData.DataType() == DT_DFLOAT );
   DIP_ASSERT( !fitVariance.IsForged() || ( fitVariance.DataType() == DT_DFLOAT ));
   DIP_ASSERT( fitData.Stride( 2 ) == 1 );
   DIP_ASSERT( !fitVariance.IsForged() || fitVariance.Stride( 2 ) == 1 );

   // Find start point
   dip::uint depth = in.Size( 2 );
   dip::uint corStart = 0;
   dfloat const* pFit = static_cast< dfloat const* >( fitData.Origin() );
   while(( corStart < depth ) && ( pFit[ corStart ] == 0 )) {
      ++corStart;
   }
   if( fromWhere == "first plane" ) {
      // OK.
   } else if( fromWhere == "global max" ) {
      dfloat max = pFit[ corStart ];
      for( dip::uint zz = corStart + 1; zz < depth; ++zz ) {
         if( pFit[ zz ] > max ) {
            max = pFit[ zz ];
            corStart = zz;
         }
      }
   } else if( fromWhere == "first max" ) {
      dip::uint zz = corStart;
      while(( pFit[ zz + 1 ] > ( hysteresis * pFit[ zz ] )) && ( zz < ( depth - 1 ))) {
         zz++;
      }
      if( zz != ( depth - 1 )) {
         corStart = zz;
      }
   } else {
      DIP_THROW_INVALID_FLAG( fromWhere );
   }

   // Do the fit
   auto fitParams = ExponentialFit(
         static_cast< dfloat* >( fitData.Origin() ),
         fitVariance.IsForged() ?  static_cast< dfloat* >( fitVariance.Origin() ) + corStart : nullptr,
         corStart,
         depth);

   // Calculate the correction values
   dfloat* fit = static_cast< dfloat* >( fitData.Origin() );
   for( dip::uint zz = 0; zz < corStart; ++zz ) {
      fit[ zz ] = 1.0;
   }
   dfloat first = std::exp( fitParams.b * static_cast< dfloat >( corStart ) + fitParams.a );
   for( dip::uint zz = corStart; zz < depth; ++zz ) {
      fit[ zz ] = first / std::exp( fitParams.b * static_cast< dfloat >( zz ) + fitParams.a );
   }

   // Correct the data
   MultiplySampleWise( in, fitData, out, in.DataType() );
}

} // namespace dip
