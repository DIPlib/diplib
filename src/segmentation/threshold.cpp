/*
 * DIPlib 3.0
 * This file contains definitions of functions that implement image thresholding.
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
#include "diplib/segmentation.h"
#include "diplib/histogram.h"
#include "diplib/math.h"
#include "diplib/statistics.h"
#include "diplib/binary.h"
#include "diplib/generation.h"
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/lookup_table.h"

namespace dip {

FloatArray IsodataThreshold(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint nThresholds
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   // TODO: if the input image is not scalar, we can apply KMeansClustering to the histogram, then do an inverse mapping.
   DIP_START_STACK_TRACE
      FloatArray thresholds = IsodataThreshold( Histogram( in, mask ), nThresholds );
      if( nThresholds == 1 ) {
         FixedThreshold( in, out, thresholds[ 0 ] );
      } else {
         MultipleThresholds( in, out, thresholds );
      }
      return thresholds;
   DIP_END_STACK_TRACE
}

dfloat OtsuThreshold(
      Image const& in,
      Image const& mask,
      Image& out
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   // TODO: if the input image is not scalar, we can apply MinimumVariancePartitioning to the histogram, then do an inverse mapping.
   DIP_START_STACK_TRACE
      dfloat threshold = OtsuThreshold( Histogram( in, mask ) );
      FixedThreshold( in, out, threshold );
      return threshold;
   DIP_END_STACK_TRACE
}

dfloat MinimumErrorThreshold(
      Image const& in,
      Image const& mask,
      Image& out
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_START_STACK_TRACE
      dfloat threshold = MinimumErrorThreshold( Histogram( in, mask ) );
      FixedThreshold( in, out, threshold );
      return threshold;
   DIP_END_STACK_TRACE
}

dfloat TriangleThreshold(
      Image const& in,
      Image const& mask,
      Image& out
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_START_STACK_TRACE
      dfloat threshold = TriangleThreshold( Histogram( in, mask ) );
      FixedThreshold( in, out, threshold );
      return threshold;
   DIP_END_STACK_TRACE
}

dfloat BackgroundThreshold(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat distance
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_START_STACK_TRACE
      dfloat threshold = BackgroundThreshold( Histogram( in, mask ), distance );
      FixedThreshold( in, out, threshold );
      return threshold;
   DIP_END_STACK_TRACE
}

dfloat VolumeThreshold(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat volumeFraction
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_START_STACK_TRACE
      dfloat threshold = Percentile( in, mask, ( 1 - volumeFraction ) * 100 ).As< dfloat >();
      FixedThreshold( in, out, threshold );
      return threshold;
   DIP_END_STACK_TRACE
}

void FixedThreshold(
      Image const& in,
      Image& out,
      dfloat threshold,
      dfloat foreground,
      dfloat background,
      String const& output
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_START_STACK_TRACE
      if( output == S::BINARY ) {
         if( foreground == 0.0 ) {
            // out = in <= threshold
            NotGreater( in, Image{ threshold, in.DataType() }, out );
         } else {
            // out = in >= threshold
            NotLesser( in, Image{ threshold, in.DataType() }, out );
         }
      } else {
         // out = in >= threshold ? foreground : background
         Select( in, Image{ threshold, in.DataType() },
                 Image{ foreground, in.DataType() }, Image{ background, in.DataType() },
                 out, ">=" );
      }
   DIP_END_STACK_TRACE
}


namespace {

template< typename TPI >
class RangeThresholdScanLineFilter : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 3; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint const inStride = params.inBuffer[ 0 ].stride;
         dip::sint const outStride = params.outBuffer[ 0 ].stride;
         dip::uint const bufferLength = params.bufferLength;
         for( dip::uint kk = 0; kk < bufferLength; ++kk ) {
            *out = (( *in >= lowerBound_ ) && ( *in <= upperBound_ )) ? foreground_ : background_;
            in += inStride;
            out += outStride;
         }
      }
      RangeThresholdScanLineFilter( dfloat lowerBound, dfloat upperBound, dfloat foreground, dfloat background ) :
            lowerBound_( clamp_cast< TPI >( lowerBound )),
            upperBound_( clamp_cast< TPI >( upperBound )),
            foreground_( clamp_cast< TPI >( foreground )),
            background_( clamp_cast< TPI >( background )) {}
   private:
      TPI lowerBound_, upperBound_, foreground_, background_;
};

} // namespace

void RangeThreshold(
      Image const& in,
      Image& out,
      dfloat lowerBound,
      dfloat upperBound,
      String const& output,
      dfloat foreground,
      dfloat background
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   if( output == S::BINARY ) {
      DIP_START_STACK_TRACE
         if( foreground == 0.0 ) {
            // out = in <= lowerBound || in >= upperBound
            OutOfRange( in, Image{ lowerBound, in.DataType() }, Image{ upperBound, in.DataType() }, out );
         } else {
            // out = in >= lowerBound && in <= upperBound
            InRange( in, Image{ lowerBound, in.DataType() }, Image{ upperBound, in.DataType() }, out );
         }
      DIP_END_STACK_TRACE
   } else {
      // out = in >= lowerBound && in <= upperBound ? foreground : background
      DataType dataType = in.DataType();
      DIP_THROW_IF( !dataType.IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
      std::unique_ptr< Framework::ScanLineFilter > lineFilter;
      DIP_OVL_NEW_REAL( lineFilter, RangeThresholdScanLineFilter, ( lowerBound, upperBound, foreground, background ), dataType );
      DIP_START_STACK_TRACE
         ImageRefArray outar{ out };
         Framework::Scan( { in }, outar, { dataType }, { dataType }, { dataType }, { 0 }, *lineFilter, Framework::ScanOption::TensorAsSpatialDim );
      DIP_END_STACK_TRACE
   }
}

void HysteresisThreshold(
      Image const& in,
      Image& out,
      dfloat lowThreshold,
      dfloat highThreshold
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_START_STACK_TRACE
      Image low = in >= lowThreshold;
      Image high = in >= highThreshold;
      BinaryPropagation( high, low, out, 0, 0, S::BACKGROUND );
   DIP_END_STACK_TRACE
}

void MultipleThresholds(
      Image const& in,
      Image& out,
      FloatArray const& thresholds
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   dip::uint nLabels = thresholds.size() + 1;
   DataType dataType = DT_UINT8;
   if( nLabels > std::numeric_limits< uint32 >::max() ) {
      dataType = DT_UINT64;
   } else if( nLabels > std::numeric_limits< uint16 >::max() ) {
      dataType = DT_UINT32;
   } else if( nLabels > std::numeric_limits< uint8 >::max() ) {
      dataType = DT_UINT16;
   }
   Image values( { nLabels }, 1, dataType );
   FillXCoordinate( values, { S::CORNER } );
   values = values.At( Range{ 1, -1 } ); // remove first label, use the out-of-bounds value for this label
   LookupTable lut( values, thresholds );
   lut.SetOutOfBoundsValue( 0, static_cast< dfloat >( nLabels - 1 ));
   DIP_STACK_TRACE_THIS( lut.Apply( in, out, LookupTable::InterpolationMode::ZERO_ORDER_HOLD ));
}

} // namespace dip
