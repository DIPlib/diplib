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

// `estimate` receives a DFLOAT version of either optionalEstimate (if not empty) or in
void CheckBilateralInput(
   Image const& in,
   Image const& optionalEstimate,
   Image& estimate,
   FloatArray& spatialSigmas
) {
   // Use `in` as estimate if no estimate is given
   if( optionalEstimate.IsForged() ) {
      DIP_STACK_TRACE_THIS( optionalEstimate.CompareProperties(
            in, Option::CmpPropEnumerator::Sizes + Option::CmpPropEnumerator::TensorElements,
            Option::ThrowException::DO_THROW ));
      estimate = Convert( optionalEstimate, DT_DFLOAT );
   } else {
      estimate = Convert( in, DT_DFLOAT );
   }
   // Do tensor-to-spatial, because the Separable/Full frameworks do this too with the 'AsScalarImage' option
   if( !estimate.IsScalar() ) {
      estimate.TensorToSpatial();
   }

   // Check spatial sigmas. If it has only 1 element, use (copy) it for all dimensions
   DIP_THROW_IF( spatialSigmas.empty(), "No spatial sigmas provided" );
   if( spatialSigmas.size() == 1 ) {
      spatialSigmas.resize( in.Dimensionality(), spatialSigmas[ 0 ] );
   } else if( spatialSigmas.size() != in.Dimensionality() ) {
      DIP_THROW( "spatialSigmas has " + std::to_string( spatialSigmas.size() ) + " elements instead of " + std::to_string( in.Dimensionality() ) + " (or 1)" );
   }
}

// Create simple half Gauss without normalization.
// The first element holds the value 1.0, the last elements holds the value std::numeric_limits<FloatType>::min();
template< typename FloatType >
void CreateUnnormalizedHalfGauss(
   Image& out,
   dfloat sigma,
   dfloat truncation
) {
   dip::uint size = clamp_cast< dip::uint >(std::ceil( truncation * sigma )) + 1; // One extra element is added for the (near) zero value
   out.ReForge( { size }, 1, DataType( static_cast<FloatType>(0) ), Option::AcceptDataTypeChange::DO_ALLOW );
   FloatType *val = static_cast<FloatType*>( out.Origin() );
   const FloatType denom = static_cast<FloatType>( -1.0 / (2.0 * sigma * sigma) );
   for( dip::uint i = 0; i < size - 1; ++i ) {
      *val++ = std::exp( static_cast< FloatType >( i * i ) * denom );
   }
   // Set last value to near-zero for division safe cutoff
   *val = std::numeric_limits< FloatType >::min();
}

// Create tonal LookupTable from a Gaussian
void CreateTonalGauss(
   Image& tonalGauss,
   dfloat tonalSigma,
   DataType dataType,
   dfloat& scaling
) {
   const dfloat gaussLUTSigma = 51.1;
   const dfloat gaussLUTTonalTrunc = 10.0;

   if( dataType == DT_SFLOAT ) {
      CreateUnnormalizedHalfGauss<sfloat>( tonalGauss, gaussLUTSigma, gaussLUTTonalTrunc );
   } else if( dataType == DT_DFLOAT ) {
      CreateUnnormalizedHalfGauss<dfloat>( tonalGauss, gaussLUTSigma, gaussLUTTonalTrunc );
   } else {
      DIP_THROW( E::DATA_TYPE_NOT_SUPPORTED );
   }

   // Return scaling factor between the sigmas
   if( tonalSigma <= 0 )
      scaling = 0;
   else
      scaling = gaussLUTSigma / tonalSigma;
}

template< typename TPI, typename TPO = FlexType< TPI > > // For TPO: see BilateralFilter(): DataType outputDataType = DataType::SuggestFlex( in.DataType() );
class FullBilateralLineFilter : public Framework::FullLineFilter
{
public:
   FullBilateralLineFilter( Image const& estimate, Kernel const& kernel, Image const& kernelImg, dfloat tonalSigma )
      : estimate_(estimate), kernel_(kernel), kernelImg_(kernelImg) {
      // Create tonal Gauss and store scaling
      CreateTonalGauss( tonalGauss_, tonalSigma, DT_DFLOAT, tonalGaussScaling_ );   // Create dfloats because buffer data type is SuggestDouble (see SeparableBilateralFilter)
   }

   // TODO: virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint nTensorElements, dip::uint nKernelPixels, dip::uint nRuns ) override {}

   virtual void SetNumberOfThreads( dip::uint, PixelTableOffsets const& pixelTable ) override {
      // Compute kernel weights for the elliptic kernel
      // The weights are stored in a local variable in the same order in which the pixel runs are being processed by the filter
      dip::uint procDim = pixelTable.ProcessingDimension();
      PixelTable pt = kernel_.PixelTable( kernelImg_.Dimensionality(), procDim );   // Obtain full PixelTable (function provides only PixelTableOffsets)
      kernelWeights_.resize( pt.NumberOfPixels() );
      dip::uint iKW = 0;
      for( auto run = pt.Runs().begin(); run != pt.Runs().end(); ++run ) {
         // Obtain coords of the start of the run, with respect to the image corner
         dip::IntegerArray runCoords( run->coordinates );
         runCoords -= pt.Origin();
         dip::UnsignedArray kernelCoords( runCoords );
         // Loop over all elements of this run and add the kernel weights in the same order
         for (dip::uint iRun=0; iRun<run->length; ++iRun, kernelCoords[procDim]++ ) {
            kernelWeights_[iKW++] = kernelImg_.At( kernelCoords ).As<dfloat>();
         }
      }
   }

   virtual void Filter( Framework::FullLineFilterParameters const& params ) override {
      TPI* in = static_cast< TPI* >(params.inBuffer.buffer);
      dip::sint inStride = params.inBuffer.stride;
      TPO* out = static_cast< TPO* >(params.outBuffer.buffer);
      dip::sint outStride = params.outBuffer.stride;
      DIP_ASSERT( params.inBuffer.tensorLength == 1 );
      dip::uint length = params.bufferLength;
      PixelTableOffsets const& pixelTableOffsets = params.pixelTable;
      dip::sint estStride = estimate_.Stride( params.dimension );

      // Index tonalGauss_ image as simple array
      dip::uint const tonalGaussSize = tonalGauss_.Size( 0 );
      DIP_ASSERT( tonalGauss_.Stride( 0 ) == 1 );
      TPI* tonalGauss = static_cast<TPI*>(tonalGauss_.Origin());

      // Get tonal center from estimate_
      dfloat const* est = static_cast< dfloat const* >( estimate_.Pointer( params.position ));

      // Iterate over all pixels in the line buffer
      for( dip::uint ii = 0; ii < length; ++ii ) {
         TPI sum = 0;
         TPI norm = 0;
         TPI const tonalCenter = static_cast< FloatType< TPI >>( *est );
         dfloat* w = kernelWeights_.data();
         // Loop over the kernel
         for( auto pto = pixelTableOffsets.begin(); pto != pixelTableOffsets.end(); ++pto ) {
            TPI inValue = in[ pto.Offset() ];
            dip::uint luIndex = static_cast<dip::uint>(std::min( static_cast<dip::uint>(abs<TPI>( inValue - tonalCenter )*tonalGaussScaling_), tonalGaussSize-1 ));
            TPI weight = static_cast<FloatType<TPI>>(*w) * tonalGauss[ luIndex ];
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
   Kernel const& kernel_;
   Image const& kernelImg_;
   FloatArray kernelWeights_;
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
   CheckBilateralInput( in, optionalEstimate, estimate, spatialSigmas );
   BoundaryConditionArray bc;
   DIP_STACK_TRACE_THIS( bc = StringArrayToBoundaryConditionArray( boundaryCondition ));

   // Create kernel
   Image kernelImg = CreateGauss( spatialSigmas, { 0 }, truncation, { 0 } );
   Kernel kernel( FloatArray(kernelImg.Sizes()), S::ELLIPTIC );
   
   // Determine best floating point type for the computations
   DataType bufferDataType = DataType::SuggestDouble( in.DataType() );
   DataType outputDataType = DataType::SuggestFlex( in.DataType() );

   DIP_START_STACK_TRACE
      std::unique_ptr< Framework::FullLineFilter > lineFilter;
      DIP_OVL_NEW_FLEX( lineFilter, FullBilateralLineFilter, ( estimate, kernel, kernelImg, tonalSigma ), bufferDataType );
      Framework::Full( in, out, bufferDataType, bufferDataType, outputDataType, in.TensorElements(), bc, kernel, *lineFilter, Framework::FullOption::AsScalarImage );
   DIP_END_STACK_TRACE

}


namespace {

// Recursive function to select bins
void SelectBins( Histogram const& hist, UnsignedArray& binIndices, dip::uint first, dip::uint last, dip::uint minDist, dfloat minSample ) {
   Image subHist = hist.GetImage().At( Range( static_cast< dip::sint >( first ), static_cast< dip::sint >( last )));
   if(( first >= last ) || ( Sum( subHist ).As< dfloat >() < minSample )) {
      return;
   } else {
      dip::uint index = MaximumPixel( subHist )[ 0 ] + first;
      SelectBins( hist, binIndices, first, index - minDist, minDist, minSample );
      binIndices.push_back( index );
      SelectBins( hist, binIndices, index + minDist, last, minDist, minSample );
   }
}

// Auto-select channels for piecewise linear bilateral filtering
FloatArray SelectChannels(
      Image const& in
) {
   Histogram::Configuration histConfig( 0.0, 100.0, 256 );
   histConfig.lowerIsPercentile = true;
   histConfig.upperIsPercentile = true;
   Histogram hist( in, {}, histConfig );  // TODO: translate histogram dimensions to input tensor elements
   dfloat minDistPC = 10.0;
   dfloat minSamplePC = 1.0;
   dip::uint minDist = static_cast<dip::uint>(floor( minDistPC * static_cast< dfloat >( hist.Bins() ) / 100.0 ));
   dfloat minSample = minSamplePC * static_cast< dfloat >( hist.Count() ) / 100.0;

   // Select bins
   UnsignedArray binIndices;
   binIndices.push_back( 0 );
   SelectBins( hist, binIndices, minDist, hist.Bins() - 1 - minDist, minDist, minSample);
   binIndices.push_back( hist.Bins() - 1 );

   // Obtain bin centers and pass them as tonalBins to QuantizedBilateralFilter
   FloatArray tonalBins;
   for( UnsignedArray::const_iterator itBinIndex = binIndices.begin(); itBinIndex != binIndices.end(); ++itBinIndex ) {
      tonalBins.push_back( hist.BinCenter( *itBinIndex ) );
   }
   return tonalBins;
}

} // End anonymous namespace

// TODO: Extend with tensor-to-spatial expansion:
// - expand in
// - ...
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
   Image estimate;
   CheckBilateralInput( in, optionalEstimate, estimate, spatialSigmas );

   // Determine best floating point type for the computations
   DataType compDataType = dip::DataType::SuggestFlex( in.DataType() );
   
   // Create tonal LookupTable
   dfloat luScaling;
   Image tonalGauss;
   CreateTonalGauss(tonalGauss, tonalSigma, compDataType, luScaling );
   LookupTable tonalLUT( tonalGauss );

   // Fill in default bins
   if( tonalBins.empty() ) {
      tonalBins = SelectChannels( in );
   }

   // Create images for the tonal bins
   // TODO (optional):This might give a performance increase:
   //       Create single, composed image with an extra dimension at the end for the bin index.
   //       The LUT images can be addressed as Image::View of the composed image.
   //       In this way, the LUT values per pixel have better memory locality for the final arrayLUT.
   ImageArray lutImages( tonalBins.size() );
   for( dip::uint i = 0; i < tonalBins.size(); ++i ) {
      dfloat tonalBin = tonalBins[ i ];
      Image luIndex;
      if( compDataType == DT_SFLOAT ) {
         // Compute floats
         luIndex = Abs( in - static_cast<float>( tonalBin ) ) * static_cast<float>( luScaling );
      }
      else {
         // Compute doubles
         luIndex = Abs( in - tonalBin ) * luScaling;
      }
      // Compute tonalWeight image using the tonalLUT
      Image tonalWeight;
      tonalLUT.Apply( luIndex, tonalWeight, LookupTable::InterpolationMode::ZERO_ORDER_HOLD );
      // Compute lutImages[ i ]
      NormalizedConvolution( in, tonalWeight, lutImages[ i ], spatialSigmas, S::BEST, boundaryCondition, truncation );
   }

   // Create lookup table to generate the output image
   LookupTable arrayLUT( lutImages, tonalBins );
   arrayLUT.Apply( estimate, out, LookupTable::InterpolationMode::LINEAR );
}


namespace {

template< typename TPI >
class SeparableBilateralLineFilter : public Framework::SeparableLineFilter
{
public:
   SeparableBilateralLineFilter( Image const& estimate, std::vector< std::vector< dfloat >> const& spatialFilters, dfloat tonalSigma ) :
      estimate_( estimate ), spatialFilters_( spatialFilters ) {
      // Create tonal Gauss and store scaling
      CreateTonalGauss( tonalGauss_, tonalSigma, DT_DFLOAT, tonalGaussScaling_ );   // Create dfloats because buffer data type is SuggestDouble (see SeparableBilateralFilter)
   }
   // TODO
   //virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint procDim ) override {
   //   return sizes_[ procDim ] + lineLength * 4;
   //}
   virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
      TPI* in = static_cast< TPI* >(params.inBuffer.buffer);
      dip::uint length = params.inBuffer.length;
      // TODO: // Estimate and in must have equal datatype? DIP_ASSERT( estimate_.DataType() == in.DataType() );
      TPI* out = static_cast< TPI* >(params.outBuffer.buffer);
      dip::sint inStride = params.inBuffer.stride;
      dip::sint outStride = params.outBuffer.stride;
      dip::sint estStride = estimate_.Stride( params.dimension );
      dip::uint const tonalGaussSize = tonalGauss_.Size(0);

      // Index tonalGauss_ image as simple array
      DIP_ASSERT( tonalGauss_.Stride( 0 ) == 1 );
      TPI* tonalGauss = static_cast<TPI*>(tonalGauss_.Origin());

      // Select filter for this dimension
      std::vector< dfloat > const& filter = spatialFilters_[ params.dimension ];

      dfloat const* filterSrc = static_cast< dfloat const* >(filter.data());
      dip::uint filterSize = filter.size();
      dfloat const* filterEnd = filterSrc + filterSize;
      dip::uint filterCenter = ( filter.size() - 1 ) / 2;
      in -= static_cast< dip::sint >( filterCenter ) * inStride;

      // Get tonal center from estimate_
      dfloat const* est = static_cast< dfloat const* >( estimate_.Pointer( params.position ) );
      // Iterate over input buffer
      for( dip::uint ii = 0; ii < length; ++ii ) {
         TPI sum = 0;
         TPI norm = 0;
         TPI* in_t = in;
         TPI const tonalCenter = static_cast< FloatType< TPI >>( *est );
         // Iterate over filter
         for( dfloat const* f = filterSrc; f != filterEnd; ++f, in_t+=inStride ) {
            dip::uint luIndex = static_cast<dip::uint>( std::min( static_cast<dip::uint>( abs<TPI>( (*in_t) - tonalCenter )*tonalGaussScaling_), tonalGaussSize-1 ) );
            TPI weight = static_cast<FloatType<TPI>>(*f) * tonalGauss[ luIndex ];
            sum += weight * (*in_t);
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
   CheckBilateralInput( in, optionalEstimate, estimate, spatialSigmas );
   BoundaryConditionArray bc;
   DIP_STACK_TRACE_THIS( bc = StringArrayToBoundaryConditionArray( boundaryCondition ));

   // Create tonal LookupTable
   DataType bufferDataType = DataType::SuggestDouble( in.DataType() );
   DataType outputDataType = DataType::SuggestFlex( in.DataType() );

   // Create 1D gaussian for each dimension
   std::vector< std::vector< dfloat >> gaussians;
   UnsignedArray borders;
   gaussians.reserve( in.Dimensionality() );
   for( dip::uint ii = 0; ii < in.Dimensionality(); ++ii ) {
      DIP_STACK_TRACE_THIS( gaussians.emplace_back( MakeGaussian( spatialSigmas[ ii ], 0, truncation ) ) );
      dip::uint gaussianLength = gaussians.back().size();
      borders.push_back( (gaussianLength - 1) / 2 );
   }
   // borderSize = half filter size

   DIP_START_STACK_TRACE
      std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
      DIP_OVL_NEW_FLEX( lineFilter, SeparableBilateralLineFilter, (estimate, gaussians, tonalSigma), bufferDataType );
      Framework::Separable( in, out, bufferDataType, outputDataType, process, borders, bc, *lineFilter, Framework::SeparableOption::AsScalarImage );
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
