/*
 * DIPlib 3.0
 * This file contains the definition for colocalization functions.
 *
 * (c)2019, Cris Luengo.
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
#include "diplib/statistics.h"
#include "diplib/math.h"
#include "diplib/histogram.h"
#include "diplib/framework.h"

namespace dip {

dfloat MandersOverlapCoefficient(
      Image const& channel1,
      Image const& channel2,
      Image const& mask
) {
   DIP_THROW_IF( !channel1.IsForged() || !channel2.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !channel1.IsScalar() || !channel2.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !channel1.DataType().IsReal() || !channel2.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_STACK_TRACE_THIS( channel1.CompareProperties( channel2, Option::CmpProp::Sizes ));
   if( mask.IsForged() ) {
      DIP_STACK_TRACE_THIS( mask.CheckIsMask( channel1.Sizes(), Option::AllowSingletonExpansion::DO_ALLOW ));
   }

   return InProduct( channel1, channel2, mask ) / std::sqrt( SumSquare( channel1, mask ).As< dfloat >() * SumSquare( channel2, mask ).As< dfloat >() );
}

ColocalizationCoefficients MandersColocalizationCoefficients(
      Image const& channel1,
      Image const& channel2,
      Image const& mask,
      dfloat threshold1,
      dfloat threshold2
) {
   DIP_THROW_IF( !channel1.IsForged() || !channel2.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !channel1.IsScalar() || !channel2.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !channel1.DataType().IsReal() || !channel2.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_STACK_TRACE_THIS( channel1.CompareProperties( channel2, Option::CmpProp::Sizes ));
   if( mask.IsForged() ) {
      DIP_STACK_TRACE_THIS( mask.CheckIsMask( channel1.Sizes(), Option::AllowSingletonExpansion::DO_ALLOW ));
   }

   Image tmp = channel2 > threshold2;
   if( mask.IsForged() ) {
      tmp &= mask;
   }
   dfloat M1 = dip::Sum( channel1, tmp ).As< dfloat >() / dip::Sum( channel1, mask ).As< dfloat >();

   tmp = channel1 > threshold1;
   if( mask.IsForged() ) {
      tmp &= mask;
   }
   dfloat  M2 = dip::Sum( channel2, tmp ).As< dfloat >() / dip::Sum( channel2, mask ).As< dfloat >();

   return { M1, M2 };
}

dfloat IntensityCorrelationQuotient(
      Image const& channel1,
      Image const& channel2,
      Image const& mask
) {
   DIP_THROW_IF( !channel1.IsForged() || !channel2.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !channel1.IsScalar() || !channel2.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !channel1.DataType().IsReal() || !channel2.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_STACK_TRACE_THIS( channel1.CompareProperties( channel2, Option::CmpProp::Sizes ));
   if( mask.IsForged() ) {
      DIP_STACK_TRACE_THIS( mask.CheckIsMask( channel1.Sizes(), Option::AllowSingletonExpansion::DO_ALLOW ));
   }

   Image c = channel1 - Mean( channel1, mask );
   c *= channel2 - Mean( channel2, mask );
   dip::uint n = Count( c > 0, mask );
   dip::uint m = c.NumberOfPixels();
   if( mask.IsForged() ) {
      dip::Image tmp = mask.QuickCopy();
      tmp.ExpandSingletonDimensions( c.Sizes() ); // we have tested that this is possible
      m = Count( tmp );
   }
   return static_cast< dfloat >( n ) / static_cast< dfloat >( m ) - 0.5;
}

ColocalizationCoefficients CostesColocalizationCoefficients(
      Image const& channel1,
      Image const& channel2,
      Image const& mask
) {
   DIP_THROW_IF( !channel1.IsForged() || !channel2.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !channel1.IsScalar() || !channel2.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !channel1.DataType().IsReal() || !channel2.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_STACK_TRACE_THIS( channel1.CompareProperties( channel2, Option::CmpProp::Sizes ));
   if( mask.IsForged() ) {
      DIP_STACK_TRACE_THIS( mask.CheckIsMask( channel1.Sizes(), Option::AllowSingletonExpansion::DO_ALLOW ));
   }

   // We ignore pixels that are zero in both images, as well as pixels outside `mask` if given
   Image nonZeroMask = channel1 > 0;
   nonZeroMask |= channel2 > 0;
   if( mask.IsForged() ) {
      nonZeroMask &= mask;
   }

   // Compute histogram
   dfloat maxValue1 = dip::Maximum( channel1, mask ).As< dfloat >();
   dfloat maxValue2 = dip::Maximum( channel2, mask ).As< dfloat >();
   Histogram hist( channel1, channel2, mask, Histogram::ConfigurationArray{{ 0.0, maxValue1 }, { 0.0, maxValue2 }} );
   Image histIm = hist.GetImage();
   auto bins1 = hist.BinCenters( 0 );
   auto bins2 = hist.BinCenters( 1 );

   // Find parameters to regression line in histogram
   RegressionParameters params = Regression( hist );
   auto f = [ &params ]( dfloat x ){ return params.intercept + x * params.slope; };
   if( params.slope <= 0.0 ) {
      // There is no positive correlation
      return { 0.0, 0.0 };
   }

   // Find initial threshold so it's meaningful for both directions
   dfloat T1 = maxValue1;
   dfloat T2 = f( T1 );
   if( T2 > maxValue2 ) {
      T1 = ( maxValue2 - params.intercept ) / params.slope;
      T2 = f( T1 );
   }
   dip::uint ind1 = 0;
   for( ; ( ind1 < bins1.size() ) && ( bins1[ ind1 ] <= T1 ); ++ind1 ) {}
   dip::uint ind2 = 0;
   for( ; ( ind2 < bins2.size() ) && ( bins2[ ind2 ] <= T2 ); ++ind2 ) {}
   // Erase stuff from histogram that is above the threshold
   if(( ind1 < bins1.size() ) && ( ind2 < bins2.size() )) {
      histIm.At( Range( static_cast< dip::sint >( ind1 ), -1 ), Range( static_cast< dip::sint >( ind2 ), -1 ) ) = 0;
   }

   // Find a delta such that each iteration we remove exactly one bin from the fastest moving threshold
   dfloat delta = bins1[ 1 ] - bins1[ 0 ];
   if( delta * params.slope > bins2[ 1 ] - bins2[ 0 ] ) {
      delta = ( bins2[ 1 ] - bins2[ 0 ] ) / params.slope;
   }

   // Iteratively reduce the threshold until we have non-positive correlation
   // TODO: We could speed this up further by not computing `PearsonCorrelation` every time,
   //       instead updating the moments when we delete a row/column of the histogram.
   while( true ) {
      dfloat corr = PearsonCorrelation( hist );
      if(( corr <= 0.0 ) || ( T1 - delta < 0.0 ) || ( f( T1 - delta ) < 0.0 )) {
         break;
      }
      T1 = T1 - delta;
      T2 = f( T1 );
      if(( ind1 > 0 ) && ( bins1[ ind1 - 1 ] > T1 )) {
         --ind1;
         histIm.At( Range( static_cast< dip::sint >( ind1 )), Range( static_cast< dip::sint >( ind2 ), -1 )) = 0;
      }
      if(( ind2 > 0 ) && ( bins2[ ind2 - 1 ] > T2 )) {
         --ind2;
         histIm.At( Range( static_cast< dip::sint >( ind1 ), -1 ), Range( static_cast< dip::sint >( ind2 ))) = 0;
      }
   }

   // Threshold and compute colocalization coefficients
   // The computation here looks like Manders', but is different.
   Image colocalizedMap = channel1 > T1;
   colocalizedMap &= channel2 > T2;
   colocalizedMap &= nonZeroMask;
   dfloat M1 = dip::Sum( channel1, colocalizedMap ).As< dfloat >() / dip::Sum( channel1, nonZeroMask ).As< dfloat >();
   dfloat M2 = dip::Sum( channel2, colocalizedMap ).As< dfloat >() / dip::Sum( channel2, nonZeroMask ).As< dfloat >();

   return { M1, M2 };
}

} // namespace dip
