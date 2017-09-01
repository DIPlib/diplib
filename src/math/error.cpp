/*
 * DIPlib 3.0
 * This file contains the definition of image error measures
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 *                                (c)2011, Cris Luengo.
 * Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
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
#include "diplib/statistics.h"
#include "diplib/math.h"
#include "diplib/linear.h"
#include "diplib/histogram.h"
#include "diplib/framework.h"

namespace dip {

dfloat MeanError( Image const& in1, Image const& in2, Image const& mask ) {
   Image error;
   DIP_START_STACK_TRACE
      error = Mean( in1 - in2, mask );
   DIP_END_STACK_TRACE
   DIP_THROW_IF( error.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED ); // means one of the inputs was complex
   if( !error.IsScalar() ) {
      error.TensorToSpatial( 0 );
      error = Mean( error ); // average across tensor elements also
   }
   return error.As< dfloat >();
}

dfloat MeanSquareError( Image const& in1, Image const& in2, Image const& mask ) {
   Image error;
   DIP_START_STACK_TRACE
      error = in1 - in2;
   DIP_END_STACK_TRACE
   if( error.DataType().IsComplex() ) {
      error = Modulus( error );
   }
   DIP_START_STACK_TRACE
      error = MeanSquare( error, mask );
   DIP_END_STACK_TRACE
   if( !error.IsScalar() ) {
      error.TensorToSpatial( 0 );
      error = Mean( error ); // average across tensor elements also
   }
   return error.As< dfloat >();
}

dfloat MeanAbsoluteError( Image const& in1, Image const& in2, Image const& mask ) {
   Image error;
   DIP_START_STACK_TRACE
      error = MeanAbs( in1 - in2, mask );
   DIP_END_STACK_TRACE
   if( !error.IsScalar() ) {
      error.TensorToSpatial( 0 );
      error = Mean( error ); // average across tensor elements also
   }
   return error.As< dfloat >();
}

dfloat MaximumAbsoluteError( Image const& in1, Image const& in2, Image const& mask ) {
   Image error;
   DIP_START_STACK_TRACE
      error = MaximumAbs( in1 - in2, mask );
   DIP_END_STACK_TRACE
   if( !error.IsScalar() ) {
      error.TensorToSpatial( 0 );
      error = Maximum( error ); // max across tensor elements also
   }
   return error.As< dfloat >();
}

namespace {

class IDivergenceLineFilter : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 23; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat const* in1 = static_cast< dfloat const* >( params.inBuffer[ 0 ].buffer );
         dfloat const* in2 = static_cast< dfloat const* >( params.inBuffer[ 1 ].buffer );
         dfloat& value = value_[ params.thread ];
         dip::uint& count = count_[ params.thread ];
         auto bufferLength = params.bufferLength;
         auto in1Stride = params.inBuffer[ 0 ].stride;
         auto in2Stride = params.inBuffer[ 1 ].stride;
         if( params.inBuffer.size() > 2 ) {
            // If there's three input buffers, we have a mask image.
            auto maskStride = params.inBuffer[ 2 ].stride;
            bin const* mask = static_cast< bin const* >( params.inBuffer[ 2 ].buffer );
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               if( *mask ) {
                  if(( *in1 > 0.0 ) && ( *in2 > 0.0 )) {
                     value += *in1 * std::log( *in1 / *in2 ) - *in1;
                     // Divide x/y before taking the log, better if x, y are very small
                  }
                  value += *in2;
                  ++count;
               }
               in1 += in1Stride;
               in2 += in2Stride;
               mask += maskStride;
            }
         } else {
            // Otherwise we don't.
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               if(( *in1 > 0.0 ) && ( *in2 > 0.0 )) {
                  value += *in1 * std::log( *in1 / *in2 ) - *in1;
                  // Divide x/y before taking the log, better if x, y are very small
               }
               value += *in2;
               in1 += in1Stride;
               in2 += in2Stride;
            }
            count += bufferLength;
         }
      }
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         value_.resize( threads, 0.0 );
         count_.resize( threads, 0 );
      }
      dfloat GetResult() {
         dfloat value = value_[ 0 ];
         dip::uint count = count_[ 0 ];
         for( dip::uint ii = 1; ii < value_.size(); ++ii ) {
            value += value_[ ii ];
            count += count_[ ii ];
         }
         return count > 0 ? value / static_cast< dfloat >( count ) : 0.0;
      }
   private:
      std::vector< dfloat > value_;
      std::vector< dip::uint > count_;
};

} // namespace

dfloat IDivergence( Image const& in1, Image const& in2, Image const& c_mask ) {
   ImageConstRefArray inar{ in1, in2 };
   DataTypeArray inBufT( 2, DT_DFLOAT );
   Image mask;
   if( c_mask.IsForged() ) {
      // If we have a mask, add it to the input array.
      mask = c_mask.QuickCopy();
      DIP_START_STACK_TRACE
         UnsignedArray sizes = Framework::SingletonExpandedSize( inar );
         mask.CheckIsMask( sizes, Option::AllowSingletonExpansion::DO_ALLOW, Option::ThrowException::DO_THROW );
         mask.ExpandSingletonDimensions( sizes );
      DIP_END_STACK_TRACE
      inar.push_back( mask );
      inBufT.push_back( mask.DataType() );
   }
   ImageRefArray outar{};
   IDivergenceLineFilter lineFilter;
   DIP_START_STACK_TRACE
      Scan( inar, outar, inBufT, {}, {}, {}, lineFilter, Framework::Scan_TensorAsSpatialDim );
   DIP_END_STACK_TRACE
   return lineFilter.GetResult();
}

dfloat InProduct( Image const& in1, Image const& in2, Image const& mask ) {
   Image error = Sum( MultiplySampleWise( in1, in2 ), mask );
   DIP_THROW_IF( error.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED ); // means one of the inputs was complex
   if( !error.IsScalar() ) {
      error.TensorToSpatial( 0 );
      error = Sum( error ); // average across tensor elements also
   }
   return error.As< dfloat >();
}

dfloat LnNormError( Image const& in1, Image const& in2, Image const& mask, dfloat order ) {
   Image error;
   DIP_START_STACK_TRACE
      error = in1 - in2;
   DIP_END_STACK_TRACE
   if( error.DataType().IsComplex() ) {
      error = SquareModulus( error );
      error = Power( error, order / 2.0 );
   } else {
      error = Power( error, order );
   }
   dip::uint N = mask.IsForged() ? Count( mask ) : error.NumberOfPixels();
   DIP_START_STACK_TRACE
      error = Sum( error, mask );
   DIP_END_STACK_TRACE
   if( !error.IsScalar() ) {
      N *= error.TensorElements();
      error.TensorToSpatial( 0 );
      error = Sum( error );
   }
   return N > 0
          ? std::pow( error.As< dfloat >(), 1.0 / order ) / static_cast< dfloat >( N )
          : 0.0;
}

dfloat PSNR( Image const& in, Image const& reference, Image const& mask, dfloat peakSignal ) {
   DIP_START_STACK_TRACE
      if( peakSignal <= 0.0 ) {
         auto m = MaximumAndMinimum( reference, mask );
         peakSignal = m.Maximum() - m.Minimum();
      }
      return 20.0 * std::log10( peakSignal / RootMeanSquareError( in, reference, mask ));
   DIP_END_STACK_TRACE
}

dfloat SSIM( Image const& in, Image const& reference, Image const& mask, dfloat sigma, dfloat K1, dfloat K2 ) {
   DIP_THROW_IF( !in.IsForged() || !reference.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.DataType().IsReal() || !reference.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( in.Sizes() != reference.Sizes(), E::SIZES_DONT_MATCH );
   if( K1 <= 0.0 ) {
      K1 = 1e-6;
   }
   if( K2 <= 0.0 ) {
      K2 = 1e-6;
   }
   auto m1 = MaximumAndMinimum( in, mask );
   auto m2 = MaximumAndMinimum( reference, mask );
   dfloat L = std::max( m1.Maximum() - m1.Minimum(), m2.Maximum() - m2.Minimum());
   dfloat C1 = ( K1 * K1 * L * L );
   dfloat C2 = ( K2 * K2 * L * L );
   Image inMean = Gauss( in, { sigma } );
   Image refMean = Gauss( reference, { sigma } );
   Image meanProduct = MultiplySampleWise( inMean, refMean );
   Square( inMean, inMean );
   Square( refMean, refMean );
   Image inVar = Gauss( Square( in ), { sigma } );
   inVar -= inMean;
   Image refVar = Gauss( Square( reference ), { sigma } );
   refVar -= refMean;
   inVar += refVar;
   refVar.Strip();
   inVar += C2;
   // Denominator
   inMean += refMean;
   refMean.Strip();
   inMean += C1;
   MultiplySampleWise( inMean, inVar, inMean );
   inVar.Strip();
   // Nominator
   Image varProduct = Gauss( MultiplySampleWise( in, reference ), { sigma } ) - meanProduct;
   meanProduct *= 2;
   meanProduct += C1;
   varProduct *= 2;
   varProduct += C2;
   MultiplySampleWise( meanProduct, varProduct, meanProduct );
   varProduct.Strip();
   // Total measure
   meanProduct /= inMean;
   inMean.Strip();
   Image error;
   DIP_START_STACK_TRACE
      error = Mean( meanProduct, mask );
   DIP_END_STACK_TRACE
   if( !error.IsScalar() ) {
      error.TensorToSpatial( 0 );
      error = Mean( error ); // average across tensor elements also
   }
   return error.As< dfloat >();
}

dfloat MutualInformation( Image const& in, Image const& reference, Image const& mask, dip::uint nBins ) {
   Histogram::ConfigurationArray configuration( 2 );
   configuration[ 0 ] = Histogram::Configuration( in.DataType() );
   configuration[ 1 ] = Histogram::Configuration( reference.DataType() );
   configuration[ 0 ].nBins = nBins;
   configuration[ 1 ].nBins = nBins; // TODO: This does nothing for 16 or 32-bit integer types.
   Histogram hist( in, reference, mask, configuration );
   return MutualInformation( hist );
}

dfloat Entropy( Image const& in, Image const& mask, dip::uint nBins ) {
   Histogram::Configuration configuration( in.DataType() );
   configuration.nBins = nBins; // TODO: This does nothing for 16 or 32-bit integer types.
   Histogram hist( in, mask, configuration );
   return Entropy( hist );
}

dfloat EstimateNoiseVariance( Image const& in, Image const& c_mask ) {
   Image mask;
   if( c_mask.IsForged() ) {
      mask = c_mask.QuickCopy();
   } else {
      DIP_START_STACK_TRACE
         GradientMagnitude( in, mask );
         Gauss( mask, mask, { 3 } );
         if( !mask.IsScalar() ) {
            // In case of a multi-channel input, take maximum over the gradient magnitudes for each channel
            dip::uint n = mask.Dimensionality();
            mask.TensorToSpatial( n );
            BooleanArray process( mask.Dimensionality(), false );
            process[ n ] = true;
            Maximum( mask, {}, mask, process );
            mask.Squeeze( n );
         }
         dfloat threshold = OtsuThreshold( Histogram( mask ));
         Lesser( mask, threshold, mask );
      DIP_END_STACK_TRACE
   }
   Image error;
   DIP_START_STACK_TRACE
      FiniteDifference( in, error, { 2 } ); // In 2D, this is the [1,-2,1;-2,4,-2;1,-2,1] matrix from the paper.
      // TODO: Not sure how this filter extends to higher dimensionalities.
      error = MeanSquare( error, mask );
   DIP_END_STACK_TRACE
   if( !error.IsScalar() ) {
      error.TensorToSpatial( 0 );
      error = Mean( error ); // average across tensor elements also
   }
   return error.As< dfloat >() / 36.0;
}

} // namespace dip
