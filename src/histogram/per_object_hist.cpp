/*
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

#include "diplib/histogram.h"

#include "diplib.h"
#include "diplib/distribution.h"
#include "diplib/framework.h"
#include "diplib/statistics.h"

namespace dip {

namespace {

class PerObjectHistogramLineFilter : public Framework::ScanLineFilter {
   public:
      //void SetNumberOfThreads( dip::uint threads ) override {
      //   for( dip::uint ii = 1; ii < threads; ++ii ) {
      //      imageArray_.emplace_back( image_ );       // makes a copy; image_ is not yet forged, so data is not shared.
      //   }
      //   // We don't forge the images here, the Filter() function should do that so each thread allocates its own
      //   // data segment. This ensures there's no false sharing.
      //}
      //dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint tensorElements ) override {
      //   return ( tensorInput_ ? tensorElements : 2 ) * 6;
      //}
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         // 0: Grey image
         dfloat const* grey = static_cast< dfloat const* >( params.inBuffer[ 0 ].buffer );
         dip::sint gStride = params.inBuffer[ 0 ].stride;
         dip::uint tLength = params.inBuffer[ 0 ].tensorLength;
         dip::sint tStride = params.inBuffer[ 0 ].tensorStride;
         // 1: Label image
         LabelType const* label = static_cast< LabelType const* >( params.inBuffer[ 1 ].buffer );
         dip::sint lStride = params.inBuffer[ 1 ].stride;
         // 2: Mask image
         bool hasMask = params.inBuffer.size() > 2;
         bin const* mask = hasMask ? static_cast< bin const* >( params.inBuffer[ 2 ].buffer ) : nullptr;
         dip::sint mStride = hasMask ? params.inBuffer[ 2 ].stride : 0;
         // Process
         for( dip::uint ii = 0; ii < params.bufferLength; ++ii ) {
            if( !hasMask || *mask ) {
               dip::uint lab = *label;
               if( !exclude0_ || lab != 0 ) {
                  if( exclude0_ ) {
                     --lab; // index 0 corresponds to label 1.
                  }
                  dfloat const* tgrey = grey;
                  for( dip::uint jj = 0; jj < tLength; ++jj ) {
                     if( !configuration_.IsOutOfRange( *tgrey )) {
                        auto index = static_cast< dip::uint >( configuration_.FindBin( *tgrey ));
                        ++( distribution_[ index ].Y( lab, jj ));
                     }
                     tgrey += tStride;
                  }
               }
            }
            grey += gStride;
            label += lStride;
            mask += mStride;
         }
      }
      PerObjectHistogramLineFilter( Distribution& distribution, Histogram::Configuration const& configuration, bool include0 ) :
            distribution_( distribution ), configuration_( configuration ), exclude0_( !include0 ) {}
   private:
      Distribution& distribution_;
      Histogram::Configuration const& configuration_;
      bool exclude0_;
};


} // namespace

Distribution PerObjectHistogram(
      Image const& grey,
      Image const& label,
      Image const& c_mask,
      Histogram::Configuration configuration, // taken by copy so we can modify it
      String const& mode,
      String const& background
) {
   DIP_THROW_IF( !label.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !label.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !label.DataType().IsUInt(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( !grey.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !grey.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );

   // Check mask, expand mask singleton dimensions if necessary
   Image mask;
   bool hasMask = false;
   if( c_mask.IsForged() ) {
      mask = c_mask.QuickCopy();
      DIP_START_STACK_TRACE
         mask.CheckIsMask( grey.Sizes(), Option::AllowSingletonExpansion::DO_ALLOW, Option::ThrowException::DO_THROW );
         mask.ExpandSingletonDimensions( grey.Sizes() );
      DIP_END_STACK_TRACE
      hasMask = true;
   }

   // Check and fix other parameters
   {
      Image scalarGrey = grey.QuickCopy();
      if( !scalarGrey.IsScalar() ) {
         scalarGrey.TensorToSpatial();
      }
      DIP_STACK_TRACE_THIS( configuration.Complete( scalarGrey, mask ));
   }
   bool fraction{};
   DIP_STACK_TRACE_THIS( fraction = BooleanFromString( mode, S::FRACTION, S::COUNT ));
   bool include0{};
   DIP_STACK_TRACE_THIS( include0 = BooleanFromString( background, S::INCLUDE, S::EXCLUDE ));

   // Count labels
   // TODO: should we use a label look-up table (and return it to the caller too) to link label to index?
   dip::uint nLabs = Maximum( label, mask ).As< LabelType >(); // we use labels 1..nLabs
   if( include0 ) {
      ++nLabs;                                              // no, we use labels 0..nLabs-1
   }

   // Create output
   //std::cout << "Creating distribution with " << configuration.nBins << " samples, of size " << nLabs << "x" << grey.TensorElements() << '\n';
   Distribution distribution( configuration.nBins, nLabs, grey.TensorElements() );
   auto it = distribution.Xbegin();
   dfloat offset = configuration.lowerBound + configuration.binSize / 2;
   for( dip::uint ii = 0; ii < configuration.nBins; ++ii, ++it ) {
      *it = offset + static_cast< dfloat >( ii ) * configuration.binSize;
   }

   // Fill output
   PerObjectHistogramLineFilter scanLineFilter( distribution, configuration, include0 );
   ImageConstRefArray inputs{ grey, label };
   DataTypeArray inBufferTypes{ DT_DFLOAT, DT_LABEL };
   if( hasMask ) {
      inputs.push_back( mask );
      inBufferTypes.push_back( DT_BIN );
   }
   ImageRefArray outputs{};
   DIP_STACK_TRACE_THIS( Framework::Scan( inputs, outputs, inBufferTypes, {}, {}, {}, scanLineFilter, Framework::ScanOption::NoMultiThreading ));

   // Normalize
   if( fraction ) {
      distribution.NormalizeSum();
   }

   return distribution;
}

} // namespace dip
