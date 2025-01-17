/*
 * This example program shows how to compute per-pixel statistics across a series of images. It computes the
 * p-th percentile of the pixels at corresponding positions in a series of images, producing a new image of
 * the same size as the input images.
 *
 * If the image series can be loaded as a single multi-dimensional image, it is much easier to just use
 * the appropriate projection function from diplib/statistics.h, dip::Percentile in this case.
 */

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/generation.h"
#include "diplib/statistics.h"
#include "diplib/generic_iterators.h"

template< typename TPI >
class AcrossImagePercentile : public dip::Framework::ScanLineFilter {
   public:
      AcrossImagePercentile( dip::sint rank ) : rank_( rank ) {}
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         buffer_.resize( threads );
      }
      virtual dip::uint GetNumberOfOperations( dip::uint nInput, dip::uint, dip::uint ) override {
         return static_cast< dip::uint >( std::round( std::log2( nInput ) * nInput )); // sorting is about n log n operations
      }
      virtual void Filter( dip::Framework::ScanLineFilterParameters const& params ) override {
         dip::uint nInputs = params.inBuffer.size();
         std::vector< TPI const* > in( nInputs );
         std::vector< dip::sint > inStride( nInputs );
         for( dip::uint ii = 0; ii < nInputs; ++ii ) {
            in[ ii ] = static_cast< TPI const* >( params.inBuffer[ ii ].buffer );
            inStride[ ii ] = params.inBuffer[ ii ].stride;
         }
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint outStride = params.outBuffer[ 0 ].stride;
         dip::uint nSamples = params.bufferLength;
         std::vector< TPI >& buffer = buffer_[ params.thread ];
         buffer.resize( nInputs );
         for( dip::uint jj = 0; jj < nSamples; ++jj ) {
            for( dip::uint ii = 0; ii < nInputs; ++ii ) {
               buffer[ ii ] = *in[ ii ];
               in[ ii ] += inStride[ ii ];
            }
            auto ourGuy = buffer.begin() + rank_;
            std::nth_element( buffer.begin(), ourGuy, buffer.end() );
            *out = *ourGuy;
            out += outStride;
         }
      }
   private:
      std::vector< std::vector< TPI >> buffer_;
      dip::sint rank_;
};

int main() {

   // Create a series of images
   dip::ImageArray imar( 10 );
   dip::UnsignedArray sz{ 256, 382 };
   dip::Random random;
   for( auto& img : imar ) {
      img.ReForge( sz, 1, dip::DT_UINT16 );
      img.Fill( 0 );
      UniformNoise( img, img, random, 500, 30000 );
   }

   // Output
   dip::Image out;

   // Parameter
   dip::dfloat percentile = 30;
   dip::sint rank = dip::RankFromPercentile( percentile, imar.size() );

   // Data type we'll use for processing
   dip::DataType dt = imar[ 0 ].DataType();
   for( dip::uint ii = 1; ii < imar.size(); ++ii ) {
      dt = dip::DataType::SuggestDyadicOperation( dt, imar[ ii ].DataType() );
   }

   // Overloaded line filter
   std::unique_ptr< dip::Framework::ScanLineFilter > lineFilter;
   DIP_OVL_NEW_REAL( lineFilter, AcrossImagePercentile, ( rank ), dt );

   // Run the function
   dip::ImageRefArray outar{ out };
   dip::Framework::Scan( dip::CreateImageConstRefArray( imar ), outar, dip::DataTypeArray( imar.size(), dt ),
         { dt }, { dt }, { 1 }, *lineFilter, { } );

   // Repeat the same computations, but concatenating the images into a 3D volume, and using the
   // percentile projection.
   sz.push_back( imar.size() );
   dip::Image vol( sz, 1, dt );
   dip::ImageSliceIterator vol_it( vol, 2 );
   auto imar_it = imar.begin();
   do {
      vol_it->Copy( *imar_it );
      ++vol_it;
      ++imar_it;
   } while( imar_it != imar.end() );
   dip::Image out2 = dip::Percentile( vol, {}, percentile, { false, false, true } );

   // Compare the two outputs
   std::cout << "Number of pixels difference: " << dip::Count( out != out2 ) << '\n';
}
