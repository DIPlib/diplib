/*
 * DIPlib 3.0
 * This file contains definitions of bilateral filtering functions.
 *
 * (c)2018, Erik Schuitema.
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

#include "diplib/nonlinear.h"
#include "diplib/lookup_table.h"
#include "diplib/pixel_table.h"
#include "diplib/histogram.h"
#include "diplib/framework.h"
#include "diplib/math.h"
#include "diplib/linear.h"
#include "diplib/statistics.h"
#include "diplib/generation.h"
#include "diplib/overload.h"

namespace dip {

namespace {

// `estimate` receives a flex-type version of either optionalEstimate (if not empty) or in
Image PrepareEstimate(
      Image const& in,
      Image const& optionalEstimate
) {
   // Use `in` as estimate if no estimate is given
   Image estimate;
   if( optionalEstimate.IsForged() ) {
      DIP_STACK_TRACE_THIS( optionalEstimate.CompareProperties(
            in, Option::CmpPropEnumerator::Sizes + Option::CmpPropEnumerator::TensorElements,
            Option::ThrowException::DO_THROW ));
      estimate = optionalEstimate;
   } else {
      estimate = in;
   }
   estimate.Convert( DataType::SuggestFlex( in.DataType() )); // No-op if already of correct type
   // Do tensor-to-spatial, because the Separable/Full frameworks do this too with the 'AsScalarImage' option
   if( !estimate.IsScalar() ) {
      estimate.TensorToSpatial();
   }
   return estimate;
}

// Create simple half Gauss without normalization.
// The first element holds the value 1.0, the last elements holds the value std::numeric_limits<FloatType>::min();
template< typename FloatType >
void CreateUnnormalizedHalfGauss(
      Image& out,
      dfloat sigma,
      dfloat truncation
) {
   dip::uint size = clamp_cast< dip::uint >( std::ceil( truncation * sigma )) + 1; // One extra element is added for the (near) zero value
   out.ReForge( { size }, 1, DataType( static_cast< FloatType >( 0 )), Option::AcceptDataTypeChange::DONT_ALLOW );
   FloatType *val = static_cast< FloatType* >( out.Origin() );
   const FloatType denom = static_cast< FloatType >( -1.0 / ( 2.0 * sigma * sigma ));
   for( dip::uint i = 0; i < size - 1; ++i ) {
      FloatType r = static_cast< FloatType >( i );
      *val++ = std::exp( r * r * denom );
   }
   // Set last value to near-zero for division safe cutoff
   *val = std::numeric_limits<FloatType>::min();
}

// Create tonal LookupTable from a Gaussian
dfloat CreateTonalGauss(
      Image& tonalGauss,
      dfloat tonalSigma,
      DataType dataType
) {
   const dfloat gaussLUTSigma = 51.1;
   const dfloat gaussLUTTonalTrunc = 10.0; // We'll have 51.1 * 10.0 + 1 = 512 bins in the LUT

   if( dataType == DT_SFLOAT ) {
      CreateUnnormalizedHalfGauss< sfloat >( tonalGauss, gaussLUTSigma, gaussLUTTonalTrunc );
   } else if( dataType == DT_DFLOAT ) {
      CreateUnnormalizedHalfGauss< dfloat >( tonalGauss, gaussLUTSigma, gaussLUTTonalTrunc );
   } else {
      DIP_THROW( E::DATA_TYPE_NOT_SUPPORTED );
   }

   // Return scaling factor between the sigmas
   return ( tonalSigma <= 0 ) ? 0 : gaussLUTSigma / tonalSigma;
}

template< typename TPI >
class FullBilateralLineFilter : public Framework::FullLineFilter {
      using TPF = FloatType< TPI >;
   public:
      FullBilateralLineFilter( Image const& estimate, dfloat tonalSigma )
            : estimate_( estimate ) {
         // Create tonal Gauss and store scaling
         tonalGaussScaling_ = CreateTonalGauss( tonalGauss_, tonalSigma, DataType( TPF( 0 ) ) );
      }

      virtual void Filter( Framework::FullLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::sint inStride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         DIP_ASSERT( params.inBuffer.tensorLength == 1 );
         dip::uint length = params.bufferLength;
         PixelTableOffsets const& pixelTableOffsets = params.pixelTable;
         dip::sint estStride = estimate_.Stride( params.dimension );

         // Index tonalGauss_ image as simple array
         dip::uint const tonalGaussSize = tonalGauss_.Size( 0 );
         DIP_ASSERT( tonalGauss_.Stride( 0 ) == 1 );
         TPF* tonalGauss = static_cast< TPF* >( tonalGauss_.Origin() );

         // Get tonal center from estimate_
         TPI const* est = static_cast< TPI const* >( estimate_.Pointer( params.position ));

         TPF const tonalGaussScaling = static_cast< TPF >( tonalGaussScaling_ );

         // Iterate over all pixels in the line buffer
         for( dip::uint ii = 0; ii < length; ++ii ) {
            TPI sum = 0;
            TPI norm = 0;
            TPI const tonalCenter = *est;
            dfloat const* w = pixelTableOffsets.Weights().data();
            // Loop over the kernel
            for( auto pto = pixelTableOffsets.begin(); pto != pixelTableOffsets.end(); ++pto ) {
               TPI inValue = in[ pto.Offset() ];
               dip::uint luIndex = static_cast< dip::uint >( std::min( static_cast< dip::uint >( std::abs( inValue - tonalCenter ) * tonalGaussScaling ), tonalGaussSize - 1 ));
               TPF weight = static_cast< TPF >( *w ) * tonalGauss[ luIndex ];
               sum += weight * inValue;
               norm += weight;
               ++w;
            }
            *out = sum / norm;
            in += inStride;
            out += outStride;
            est += estStride;
         }
      }

   protected:
      Image const& estimate_;
      Image tonalGauss_;
      dfloat tonalGaussScaling_;
};

} // End anonymous namespace

void FullBilateralFilter(
      Image const& in,
      Image const& optionalEstimate,
      Image& out,
      FloatArray spatialSigmas,
      dfloat tonalSigma,
      dfloat truncation,
      StringArray const& boundaryCondition
) {
   // Check input
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   Image estimate;
   DIP_STACK_TRACE_THIS( estimate = PrepareEstimate( in, optionalEstimate ));
   DIP_STACK_TRACE_THIS( ArrayUseParameter( spatialSigmas, in.Dimensionality(), 2.0 ));
   BoundaryConditionArray bc;
   DIP_STACK_TRACE_THIS( bc = StringArrayToBoundaryConditionArray( boundaryCondition ));

   // Create kernel
   Image kernelImg = CreateGauss( spatialSigmas, { 0 }, truncation, { 0 } );
   UnsignedArray center = kernelImg.Sizes();
   center /= 2;
   center[ 0 ] = 0;
   kernelImg.At( kernelImg < kernelImg.At( center )) = nan;
   Kernel kernel( kernelImg );
   
   DataType dataType = DataType::SuggestFlex( in.DataType() );
   DIP_START_STACK_TRACE
      std::unique_ptr< Framework::FullLineFilter > lineFilter;
      DIP_OVL_NEW_FLEX( lineFilter, FullBilateralLineFilter, ( estimate, tonalSigma ), dataType );
      Framework::Full( in, out, dataType, dataType, dataType, in.TensorElements(), bc, kernel, *lineFilter, Framework::FullOption::AsScalarImage );
   DIP_END_STACK_TRACE
}


namespace {

// Recursive function to select bins
void SelectBins( Histogram const& hist, std::vector< dip::uint >& binIndices, dip::uint first, dip::uint last, dip::uint minDist, dip::uint minSample ) {
   if( first > last ) {
      return;
   }
   Image subHist = hist.GetImage();
   subHist.SetSizesUnsafe( { last - first + 1 } );
   subHist.ShiftOriginUnsafe( static_cast< dip::sint >( first ));
   if( Sum( subHist ).As< dip::uint >() < minSample ) {
      return;
   }
   dip::uint index = MaximumPixel( subHist )[ 0 ] + first;
   SelectBins( hist, binIndices, first, index - minDist, minDist, minSample );
   binIndices.push_back( index );
   SelectBins( hist, binIndices, index + minDist, last, minDist, minSample );
}

// Auto-select channels for piecewise linear bilateral filtering
FloatArray SelectChannels(
      Image const& in
) {
   Histogram::Configuration histConfig( 0.0, 100.0, 256 );
   histConfig.lowerIsPercentile = true;
   histConfig.upperIsPercentile = true;
   Image inFlat = in.QuickCopy().TensorToSpatial();
   Histogram hist( inFlat, {}, histConfig );
   constexpr dip::uint minDistPC = 10;
   constexpr dip::uint minSamplePC = 1;
   dip::uint minDist = std::max( dip::uint( 1 ), minDistPC * hist.Bins() / 100u );
   dip::uint minSample = div_ceil( minSamplePC * hist.Count(), dip::uint( 100 ));

   // Select bins
   std::vector< dip::uint > binIndices;
   binIndices.push_back( 0 );
   SelectBins( hist, binIndices, minDist, hist.Bins() - 1 - minDist, minDist, minSample );
   binIndices.push_back( hist.Bins() - 1 );

   // Obtain bin centers and pass them as tonalBins to QuantizedBilateralFilter
   FloatArray tonalBins( binIndices.size() );
   for( dip::uint ii = 0; ii < binIndices.size(); ++ii ) {
      tonalBins[ ii ] = hist.BinCenter( binIndices[ ii ] );
   }
   return tonalBins;
}

} // End anonymous namespace


void QuantizedBilateralFilter(
      Image const& in,
      Image const& optionalEstimate,
      Image& out,
      FloatArray spatialSigmas,
      dfloat tonalSigma,
      FloatArray tonalBins,
      dfloat truncation,
      StringArray const& boundaryCondition
) {
   // Check input
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR ); // TODO: Extend with tensor-to-spatial expansion
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   Image estimate;
   DIP_STACK_TRACE_THIS( estimate = PrepareEstimate( in, optionalEstimate ));
   DIP_STACK_TRACE_THIS( ArrayUseParameter( spatialSigmas, in.Dimensionality(), 2.0 ));

   // Determine best floating point type for the computations
   DataType compDataType = dip::DataType::SuggestFlex( in.DataType() );
   
   // Create tonal LookupTable
   Image tonalGauss;
   dfloat luScaling = CreateTonalGauss( tonalGauss, tonalSigma, compDataType );
   LookupTable tonalLUT( tonalGauss );

   // Fill in default bins
   if( tonalBins.empty() ) {
      tonalBins = SelectChannels( in );
   }

   DIP_START_STACK_TRACE
      // Create images for the tonal bins
      // TODO (optional): This might give a performance increase:
      //       Create single, composed image with an extra dimension at the end for the bin index.
      //       The LUT images can be addressed as Image::View of the composed image.
      //       In this way, the LUT values per pixel have better memory locality for the final arrayLUT.
      ImageArray lutImages( tonalBins.size() );
      for( dip::uint ii = 0; ii < tonalBins.size(); ++ii ) {
         dfloat tonalBin = tonalBins[ ii ];
         Image luIndex = Subtract( in, tonalBin, compDataType );
         Abs( luIndex, luIndex );
         luIndex *= luScaling;
         // Compute tonalWeight image using the tonalLUT
         Image tonalWeight;
         tonalLUT.Apply( luIndex, tonalWeight, LookupTable::InterpolationMode::ZERO_ORDER_HOLD );
         // Compute lutImages[ ii ]
         NormalizedConvolution( in, tonalWeight, lutImages[ ii ], spatialSigmas, S::BEST, boundaryCondition, truncation );
      }

      // Create lookup table to generate the output image
      LookupTable arrayLUT( lutImages, tonalBins );
      arrayLUT.Apply( estimate, out, LookupTable::InterpolationMode::LINEAR );
   DIP_END_STACK_TRACE
}


namespace {

template< typename TPI >
class SeparableBilateralLineFilter : public Framework::SeparableLineFilter {
      using TPF = FloatType< TPI >;
   public:
      SeparableBilateralLineFilter( Image const& estimate, std::vector< std::vector< dfloat >> const& spatialFilters, dfloat tonalSigma ) :
            estimate_( estimate ), spatialFilters_( spatialFilters ) {
         // Create tonal Gauss and store scaling
         tonalGaussScaling_ = CreateTonalGauss( tonalGauss_, tonalSigma, DataType( TPF( 0 )));
      }

      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::uint length = params.inBuffer.length;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint inStride = params.inBuffer.stride;
         dip::sint outStride = params.outBuffer.stride;
         dip::sint estStride = estimate_.Stride( params.dimension );
         dip::uint const tonalGaussSize = tonalGauss_.Size( 0 );

         // Index tonalGauss_ image as simple array
         DIP_ASSERT( tonalGauss_.Stride( 0 ) == 1 );
         TPF* tonalGauss = static_cast< TPF* >( tonalGauss_.Origin() );

         // Select filter for this dimension
         std::vector< dfloat > const& filter = spatialFilters_[ params.dimension ];
         dfloat const* filterSrc = filter.data();
         dip::uint filterSize = filter.size();
         dfloat const* filterEnd = filterSrc + filterSize;
         dip::uint filterCenter = ( filter.size() - 1 ) / 2;
         in -= static_cast< dip::sint >( filterCenter ) * inStride;

         // Get tonal center from estimate_
         TPI const* est = static_cast< TPI const* >( estimate_.Pointer( params.position ));
         TPF const tonalGaussScaling = static_cast< TPF >( tonalGaussScaling_ );

         // Iterate over input buffer
         for( dip::uint ii = 0; ii < length; ++ii ) {
            TPI sum = 0;
            TPI norm = 0;
            TPI* in_t = in;
            TPI const tonalCenter = *est;
            // Iterate over filter
            for( dfloat const* f = filterSrc; f != filterEnd; ++f, in_t += inStride ) {
               dip::uint luIndex = static_cast< dip::uint >( std::min( static_cast< dip::uint >( std::abs( *in_t - tonalCenter ) * tonalGaussScaling ), tonalGaussSize - 1 ));
               TPF weight = static_cast< TPF >( *f ) * tonalGauss[ luIndex ];
               sum += weight * ( *in_t );
               norm += weight;
            }
            *out = sum / norm;
            in += inStride;
            out += outStride;
            est += estStride;
         }
      }
   private:
      Image const& estimate_; // Estimate of tonal center
      std::vector< std::vector< dfloat >> const& spatialFilters_;
      Image tonalGauss_;
      dfloat tonalGaussScaling_;
};

} // End anonymous namespace

void SeparableBilateralFilter(
      Image const& in,
      Image const& optionalEstimate,   // Estimate of tonal center
      Image& out,
      BooleanArray const& process,
      FloatArray spatialSigmas,
      dfloat tonalSigma,
      dfloat truncation,
      StringArray const& boundaryCondition
) {
   // Check input
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   Image estimate;
   DIP_STACK_TRACE_THIS( estimate = PrepareEstimate( in, optionalEstimate ));
   DIP_STACK_TRACE_THIS( ArrayUseParameter( spatialSigmas, in.Dimensionality(), 2.0 ));
   BoundaryConditionArray bc;
   DIP_STACK_TRACE_THIS( bc = StringArrayToBoundaryConditionArray( boundaryCondition ));

   // Create 1D gaussian for each dimension
   std::vector< std::vector< dfloat >> gaussians;
   UnsignedArray borders;
   gaussians.reserve( in.Dimensionality() );
   for( dip::uint ii = 0; ii < in.Dimensionality(); ++ii ) {
      DIP_STACK_TRACE_THIS( gaussians.emplace_back( MakeGaussian( spatialSigmas[ ii ], 0, truncation ) ) );
      dip::uint gaussianLength = gaussians.back().size();
      borders.push_back(( gaussianLength - 1 ) / 2 );
   }

   DataType dataType = DataType::SuggestFlex( in.DataType() );
   DIP_START_STACK_TRACE
      std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
      DIP_OVL_NEW_FLEX( lineFilter, SeparableBilateralLineFilter, ( estimate, gaussians, tonalSigma ), dataType );
      Framework::Separable( in, out, dataType, dataType, process, borders,
                            bc, *lineFilter, Framework::SeparableOption::AsScalarImage );
   DIP_END_STACK_TRACE
}


void BilateralFilter(
      Image const& in,
      Image const& estimate,
      Image& out,
      FloatArray const& spatialSigmas,
      dfloat tonalSigma,
      dfloat truncation,
      String const& method,
      StringArray const& boundaryCondition
) {
   if( method == "full" ) {
      DIP_STACK_TRACE_THIS( FullBilateralFilter( in, estimate, out, spatialSigmas, tonalSigma, truncation, boundaryCondition ));
   } else if( method == "pwlinear" ) {
      DIP_STACK_TRACE_THIS( QuantizedBilateralFilter( in, estimate, out, spatialSigmas, tonalSigma, {}, truncation, boundaryCondition ));
   } else if( method == "xysep" ) {
      DIP_STACK_TRACE_THIS( SeparableBilateralFilter( in, estimate, out, {}, spatialSigmas, tonalSigma, truncation, boundaryCondition ));
   } else {
      DIP_THROW_INVALID_FLAG( method );
   }
}

} // namespace dip
