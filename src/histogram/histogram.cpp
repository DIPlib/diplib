/*
 * (c)2017-2024, Cris Luengo.
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

#include <algorithm>
#include <cmath>
#include <memory>
#include <utility>
#include <vector>

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/linear.h"
#include "diplib/measurement.h"
#include "diplib/multithreading.h"
#include "diplib/overload.h"
#include "diplib/statistics.h"

namespace dip {

using CountType = Histogram::CountType;

void Histogram::Configuration::Complete( bool isInteger ) {
   if( mode == Mode::IS_COMPLETE ) {
      return;
   }
   DIP_THROW_IF( lowerIsPercentile || upperIsPercentile, "Cannot complete configuration without image data, bounds are percentiles" );
   // Fixup wrong values silently
   if( mode != Mode::COMPUTE_BINS ) {
      if( nBins < 1 ) {
         nBins = 256;
      }
   }
   bool hasNoBinSize = false;
   if( mode != Mode::COMPUTE_BINSIZE ) {
      if( binSize <= 0.0 ) {
         hasNoBinSize = true;
         binSize = 1.0;
      }
   }
   if(( mode != Mode::COMPUTE_LOWER ) && ( mode != Mode::COMPUTE_UPPER )) {
      if( upperBound < lowerBound ) {
         std::swap( upperBound, lowerBound );
      } else if( upperBound == lowerBound ) {
         upperBound += 1.0;
      }
   }
   // For integer images, we need the bin size and bounds to be integer values
   if( isInteger ) {
      lowerBound = std::floor( lowerBound );
      upperBound = std::ceil( upperBound );
      if( mode == Mode::COMPUTE_BINSIZE ) {
         binSize = ( upperBound - lowerBound ) / static_cast< dfloat >( nBins );
      }
      binSize = std::ceil( binSize );
   }
   // Next, complete the configuration
   switch( mode ) {
      case Mode::COMPUTE_BINSIZE:
         if( isInteger ) {
            // We've aready computed the bin size earlier
            upperBound = lowerBound + static_cast< dfloat >( nBins ) * binSize;
         } else {
            binSize = ( upperBound - lowerBound ) / static_cast< dfloat >( nBins );
         }
         break;
      case Mode::COMPUTE_BINS:
         if( hasNoBinSize && isInteger ) {
            // Find a suitable bin size that is power of 2:
            dfloat range = upperBound - lowerBound; // an integer value...
            binSize = std::round( std::pow( 2.0, std::ceil( std::log2( range / 256.0 ))));
            binSize = std::max( binSize, 1.0 );
            // Shift lower bound to be a multiple of the binSize:
            lowerBound = std::floor( lowerBound / binSize ) * binSize;
            // Update range in case we shifted lower bound:
            range = upperBound - lowerBound;
            // Find number of bins we need to use:
            nBins = static_cast< dip::uint >( std::ceil( range / binSize ));
            // Update upper bound so that it matches what the histogram class would compute:
            upperBound = lowerBound + static_cast< dfloat >( nBins ) * binSize;
         } else {
            nBins = static_cast< dip::uint >( std::round(( upperBound - lowerBound ) / binSize ));
            if( isInteger ) {
               upperBound = lowerBound + static_cast< dfloat >( nBins ) * binSize;
            } else {
               binSize = ( upperBound - lowerBound ) / static_cast< dfloat >( nBins );
            }
         }
         break;
      case Mode::COMPUTE_LOWER:
         lowerBound = upperBound - static_cast< dfloat >( nBins ) * binSize;
         break;
      case Mode::COMPUTE_UPPER:
         upperBound = lowerBound + static_cast< dfloat >( nBins ) * binSize;
         break;
      default:
         // Mode::ESTIMATE_BINSIZE or Mode::ESTIMATE_BINSIZE_AND_LIMITS
         DIP_THROW( "Cannot complete configuration without image data, mode requests choosing bin size according to data" );
   }
   if( isInteger ) {
      DIP_ASSERT( binSize == std::round( binSize ));
      DIP_ASSERT( lowerBound == std::round( lowerBound ));
      DIP_ASSERT( upperBound == std::round( upperBound ));
      // Let's make sure the bin centers are integers
      if( static_cast< dip::uint >( binSize ) % 2 ) {
         // If the bin size is odd, then the center of a bin (at `(bound + binSize / 2)`) will be half-way between two integers
         lowerBound -= 0.5;
         upperBound -= 0.5;
      }
   }
   mode = Mode::IS_COMPLETE;
}

void Histogram::Configuration::Complete( Image const& input, Image const& mask ) {
   if( mode == Mode::IS_COMPLETE ) {
      return;
   }
   if( mode == Mode::ESTIMATE_BINSIZE || mode == Mode::ESTIMATE_BINSIZE_AND_LIMITS ) {
      DIP_START_STACK_TRACE
         auto quartiles = Quartiles( input, mask );
         dfloat iqr = quartiles.upperQuartile - quartiles.lowerQuartile;
         if( iqr == 0 ) {
            // Case where the middle 50% of samples have the same value...
            binSize = ( quartiles.maximum - quartiles.minimum ) / 256;
            // Ensure we don't use lower and upper quartile later!
            if( mode == Mode::ESTIMATE_BINSIZE_AND_LIMITS ) {
               mode = Mode::ESTIMATE_BINSIZE;
               lowerBound = quartiles.minimum;
               upperBound = quartiles.maximum * ( 1.0 + 1e-15 );
               lowerIsPercentile = false;
               upperIsPercentile = false;
            }
            // If this is zero also, then there's not a whole lot we can do here
         } else {
            dip::uint n = input.NumberOfSamples();
            if( mask.IsForged() ) {
               n = dip::Count( mask );
            }
            binSize = 2 * iqr / std::cbrt( n );
         }
         if( mode == Mode::ESTIMATE_BINSIZE_AND_LIMITS ) {
            // We use the min and max, but avoid the full range being more than 101 times the IQR.
            lowerBound = std::max( quartiles.minimum, quartiles.lowerQuartile - 50 * iqr );
            upperBound = std::min( quartiles.maximum, quartiles.upperQuartile + 50 * iqr ) * ( 1.0 + 1e-15 );
            lowerIsPercentile = false;
            upperIsPercentile = false;
         } else {
            // avoid computing the min and max again later, we already have these values computed.
            if( lowerIsPercentile && lowerBound <= 0 ) {
               lowerBound = quartiles.minimum;
               lowerIsPercentile = false;
            }
            if( upperIsPercentile && upperBound >= 100 ) {
               upperBound = quartiles.maximum * ( 1.0 + 1e-15 );
               upperIsPercentile = false;
            }
         }
         nBins = 0;
         mode = Mode::COMPUTE_BINS;
      DIP_END_STACK_TRACE
   } else {
      if( lowerIsPercentile && mode != Mode::COMPUTE_LOWER ) {
         lowerBound = Percentile( input, mask, lowerBound ).As< dfloat >();
         lowerIsPercentile = false;
      }
      if( upperIsPercentile && mode != Mode::COMPUTE_UPPER ) {
         // NOTE: we increase the upper bound when computed as a percentile, because we do lowerBound <= value < upperBound.
         upperBound = Percentile( input, mask, upperBound ).As< dfloat >() * ( 1.0 + 1e-15 );
         upperIsPercentile = false;
      }
   }
   Complete( input.DataType().IsInteger() );
}

namespace {

class HistogramBaseLineFilter : public Framework::ScanLineFilter {
   public:
      HistogramBaseLineFilter( Image& image ) : image_( image ) {}
      void SetNumberOfThreads( dip::uint threads ) override {
         for( dip::uint ii = 1; ii < threads; ++ii ) {
            imageArray_.emplace_back( image_ );       // makes a copy; image_ is not yet forged, so data is not shared.
         }
         // We don't forge the images here, the Filter() function should do that so each thread allocates its own
         // data segment. This ensures there's no false sharing.
      }
      void Reduce() {
         for( auto const& img : imageArray_ ) {
            image_ += img;
         }
      }
   protected:
      Image& image_;
      ImageArray imageArray_;
};

template< typename TPI >
class ScalarImageHistogramLineFilter : public HistogramBaseLineFilter {
   public:
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override { return 6; }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         auto bufferLength = params.bufferLength;
         auto inStride = params.inBuffer[ 0 ].stride;
         Image& image = params.thread == 0 ? image_ : imageArray_[ params.thread - 1 ];
         if( !image.IsForged() ) {
            image.Forge();
            image.Fill( 0 );
         }
         CountType* data = static_cast< CountType* >( image.Origin() );
         // Note: `image_` strides are always normal.
         if( params.inBuffer.size() > 1 ) {
            // If there's two input buffers, we have a mask image.
            bin const* mask = static_cast< bin const* >( params.inBuffer[ 1 ].buffer );
            auto maskStride = params.inBuffer[ 1 ].stride;
            if( configuration_.excludeOutOfBoundValues ) {
               for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
                  if( *mask && ( static_cast< dfloat >( *in ) >= configuration_.lowerBound ) && ( static_cast< dfloat >( *in ) < configuration_.upperBound )) {
                     ++data[ configuration_.FindBin( static_cast< dfloat >( *in )) ];
                  }
                  in += inStride;
                  mask += maskStride;
               }
            } else {
               for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
                  if( *mask ) {
                     ++data[ configuration_.FindBin( static_cast< dfloat >( *in )) ];
                  }
                  in += inStride;
                  mask += maskStride;
               }
            }
         } else {
            // Otherwise we don't.
            if( configuration_.excludeOutOfBoundValues ) {
               for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
                  if(( static_cast< dfloat >( *in ) >= configuration_.lowerBound ) && ( static_cast< dfloat >( *in ) < configuration_.upperBound )) {
                     ++data[ configuration_.FindBin( static_cast< dfloat >( *in )) ];
                  }
                  in += inStride;
               }
            } else {
               for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
                  ++data[ configuration_.FindBin( static_cast< dfloat >( *in )) ];
                  in += inStride;
               }
            }
         }
      }
      ScalarImageHistogramLineFilter( Image& image, Histogram::Configuration const& configuration ) :
            HistogramBaseLineFilter( image ), configuration_( configuration ) {}
   private:
      Histogram::Configuration const& configuration_;
};

template< typename TPI >
class JointImageHistogramLineFilter : public HistogramBaseLineFilter {
   public:
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint tensorElements ) override {
         return ( tensorInput_ ? tensorElements : 2 ) * 6;
      }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         std::vector< TPI const* > in;
         std::vector< dip::sint > stride;
         dip::uint nDims{};
         dip::uint maskBuffer{};
         if( tensorInput_ ) {
            nDims = params.inBuffer[ 0 ].tensorLength;
            in.resize( nDims );
            auto tensorStride = params.inBuffer[ 0 ].tensorStride;
            in[ 0 ] = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
            for( dip::uint ii = 1; ii < nDims; ++ii ) {
               in[ ii ] = in[ ii - 1 ] + tensorStride;
            }
            stride.resize( nDims, params.inBuffer[ 0 ].stride );
            maskBuffer = 1;
         } else {
            nDims = 2;
            DIP_ASSERT( params.inBuffer.size() >= 2 );
            in = { static_cast< TPI const* >( params.inBuffer[ 0 ].buffer ),
                   static_cast< TPI const* >( params.inBuffer[ 1 ].buffer ) };
            stride = { params.inBuffer[ 0 ].stride, params.inBuffer[ 1 ].stride };
            maskBuffer = 2;
         }
         auto bufferLength = params.bufferLength;
         Image& image = params.thread == 0 ? image_ : imageArray_[ params.thread - 1 ];
         if( !image.IsForged() ) {
            image.Forge();
            image.Fill( 0 );
//#if defined(_OPENMP)
            // For some reason, MATLAB crashes the second time that `mdhistogram` is called,
            // when using multi-threading. This tiny sleep prevented the crash in the past.
            // A `std::cout <<` call also prevented the crash. However, MATLAB is crashing again.
            // Now we are simply never calling this function multi-threaded in DIPimage. Keeping
            // this hack here in comments for future reference.
            // Some people say that these crashes are an issue of compatibility between OpenMP
            // libraries (MATLAB links against Intel's they say).
            //using namespace std::chrono_literals;
            //std::this_thread::sleep_for(10ns);
//#endif
         }
         CountType* data = static_cast< CountType* >( image.Origin() );
         if( params.inBuffer.size() > maskBuffer ) {
            // We have a mask image.
            bin const* mask = static_cast< bin const* >( params.inBuffer[ maskBuffer ].buffer );
            auto maskStride = params.inBuffer[ maskBuffer ].stride;
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               if( *mask ) {
                  bool include = true;
                  for( dip::uint jj = 0; jj < nDims; ++jj ) {
                     if( configuration_[ jj ].IsOutOfRange( static_cast< dfloat >( *( in[ jj ] )))) {
                        include = false;
                        break;
                     }
                  }
                  if( include ) {
                     dip::sint offset = 0;
                     for( dip::uint jj = 0; jj < nDims; ++jj ) {
                        offset += image_.Stride( jj ) * configuration_[ jj ].FindBin( static_cast< dfloat >( *( in[ jj ] )));
                     }
                     ++data[ offset ];
                  }
               }
               for( dip::uint jj = 0; jj < nDims; ++jj ) {
                  in[ jj ] += stride[ jj ];
               }
               mask += maskStride;
            }
         } else {
            // We don't have a mask image.
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               bool include = true;
               for( dip::uint jj = 0; jj < nDims; ++jj ) {
                  if( configuration_[ jj ].IsOutOfRange( static_cast< dfloat >( *( in[ jj ] )))) {
                     include = false;
                     break;
                  }
               }
               if( include ) {
                  dip::sint offset = 0;
                  for( dip::uint jj = 0; jj < nDims; ++jj ) {
                     offset += image_.Stride( jj ) * configuration_[ jj ].FindBin( static_cast< dfloat >( *( in[ jj ] )));
                  }
                  ++data[ offset ];
               }
               for( dip::uint jj = 0; jj < nDims; ++jj ) {
                  in[ jj ] += stride[ jj ];
               }
            }
         }
      }
      JointImageHistogramLineFilter( Image& image, Histogram::ConfigurationArray const& configuration, bool tensorInput ) :
            HistogramBaseLineFilter( image ), configuration_( configuration ), tensorInput_( tensorInput ) {}
   private:
      Histogram::ConfigurationArray const& configuration_;
      bool tensorInput_;
};

} // namespace

void Histogram::ScalarImageHistogram( Image const& input, Image const& mask, Histogram::Configuration& configuration ) {
   DIP_STACK_TRACE_THIS( configuration.Complete( input, mask ));
   lowerBounds_ = { configuration.lowerBound };
   binSizes_ = { configuration.binSize };
   data_.SetSizes( { configuration.nBins } );
   data_.SetDataType( DT_COUNT );
   std::unique_ptr< HistogramBaseLineFilter >scanLineFilter;
   DIP_OVL_NEW_REAL( scanLineFilter, ScalarImageHistogramLineFilter, ( data_, configuration ), input.DataType() );
   Framework::ScanOptions opts;
   if( GetNumberOfThreads() > 1 ) {
      dip::uint parallelOperations = input.NumberOfPixels() * 6;
      dip::uint sequentialOperations = ( GetNumberOfThreads() - 1 ) * ( data_.NumberOfPixels() * 2 + 10000 );
      if( parallelOperations / GetNumberOfThreads() + sequentialOperations + threadingThreshold > parallelOperations ) {
         opts = Framework::ScanOption::NoMultiThreading; // Turn off multithreading if we'll do a lot of work to reduce.
      }
   }
   DIP_STACK_TRACE_THIS( Framework::ScanSingleInput( input, mask, input.DataType(), *scanLineFilter, opts ));
   scanLineFilter->Reduce();
}

void Histogram::TensorImageHistogram( Image const& input, Image const& mask, Histogram::ConfigurationArray& configuration ) {
   dip::uint ndims = input.TensorElements();
   lowerBounds_.resize( ndims );
   binSizes_.resize( ndims );
   UnsignedArray sizes( ndims, 1 );
   for( dip::uint ii = 0; ii < ndims; ++ii ) {
      DIP_STACK_TRACE_THIS( configuration[ ii ].Complete( input[ static_cast< dip::sint >( ii ) ], mask ));
      lowerBounds_[ ii ] = configuration[ ii ].lowerBound;
      binSizes_[ ii ] = configuration[ ii ].binSize;
      sizes[ ii ] = configuration[ ii ].nBins;
   }
   data_.SetSizes( std::move( sizes ));
   data_.SetDataType( DT_COUNT );
   std::unique_ptr< HistogramBaseLineFilter >scanLineFilter;
   DIP_OVL_NEW_REAL( scanLineFilter, JointImageHistogramLineFilter, ( data_, configuration, true ), input.DataType() );
   Framework::ScanOptions opts;
   if( GetNumberOfThreads() > 1 ) {
      dip::uint parallelOperations = input.NumberOfPixels() * ndims * 6;
      dip::uint sequentialOperations = ( GetNumberOfThreads() - 1 ) * ( data_.NumberOfPixels() * 2 + 10000 );
      if( parallelOperations / GetNumberOfThreads() + sequentialOperations + threadingThreshold > parallelOperations ) {
         opts = Framework::ScanOption::NoMultiThreading; // Turn off multithreading if we'll do a lot of work to reduce.
      }
   }
   DIP_STACK_TRACE_THIS( Framework::ScanSingleInput( input, mask, input.DataType(), *scanLineFilter, opts ));
   scanLineFilter->Reduce();
}

void Histogram::JointImageHistogram( Image const& input1, Image const& input2, Image const& c_mask, Histogram::ConfigurationArray& configuration ) {
   DIP_START_STACK_TRACE
      configuration[ 0 ].Complete( input1, c_mask );
      configuration[ 1 ].Complete( input2, c_mask );
   DIP_END_STACK_TRACE
   lowerBounds_ = { configuration[ 0 ].lowerBound, configuration[ 1 ].lowerBound };
   binSizes_ = { configuration[ 0 ].binSize, configuration[ 1 ].binSize };
   UnsignedArray sizes{ configuration[ 0 ].nBins, configuration[ 1 ].nBins };
   data_.SetSizes( std::move( sizes ));
   data_.SetDataType( DT_COUNT );
   DataType dtype = DataType::SuggestDyadicOperation( input1.DataType(), input2.DataType() );
   std::unique_ptr< HistogramBaseLineFilter >scanLineFilter;
   DIP_OVL_NEW_REAL( scanLineFilter, JointImageHistogramLineFilter, ( data_, configuration, false ), dtype );
   ImageConstRefArray inar{ input1, input2 };
   DataTypeArray inBufT{ dtype, dtype };
   if( c_mask.IsForged() ) {
      // If we have a mask, add it to the input array.
      Image mask = c_mask.QuickCopy();
      DIP_START_STACK_TRACE
         mask.CheckIsMask( input1.Sizes(), Option::AllowSingletonExpansion::DO_ALLOW, Option::ThrowException::DO_THROW );
         mask.ExpandSingletonDimensions( input1.Sizes() );
      DIP_END_STACK_TRACE
      inar.push_back( mask );
      inBufT.push_back( mask.DataType() );
   }
   ImageRefArray outar{};
   Framework::ScanOptions opts;
   if( GetNumberOfThreads() > 1 ) {
      dip::uint parallelOperations = input1.NumberOfPixels() * 2 * 6;
      dip::uint sequentialOperations = ( GetNumberOfThreads() - 1 ) * ( data_.NumberOfPixels() * 2 + 10000 );
      if( parallelOperations / GetNumberOfThreads() + sequentialOperations + threadingThreshold > parallelOperations ) {
         opts = Framework::ScanOption::NoMultiThreading; // Turn off multithreading if we'll do a lot of work to reduce.
      }
   }
   DIP_STACK_TRACE_THIS( Framework::Scan( inar, outar, inBufT, {}, {}, {}, *scanLineFilter, opts ));
   scanLineFilter->Reduce();
}

void Histogram::MeasurementFeatureHistogram( Measurement::IteratorFeature const& featureValues, Histogram::ConfigurationArray& configuration ) {
   dip::uint ndims = featureValues.NumberOfValues();
   lowerBounds_.resize( ndims );
   binSizes_.resize( ndims );
   UnsignedArray sizes( ndims );
   for( dip::uint ii = 0; ii < ndims; ++ii ) {
      DIP_START_STACK_TRACE
         Measurement::IteratorFeature featureColumn = featureValues;
         featureColumn.Subset( ii );
         configuration[ ii ].Complete( featureColumn );
      DIP_END_STACK_TRACE
      lowerBounds_[ ii ] = configuration[ ii ].lowerBound;
      binSizes_[ ii ] = configuration[ ii ].binSize;
      sizes[ ii ] = configuration[ ii ].nBins;
   }
   data_.SetSizes( std::move( sizes ));
   data_.SetDataType( DT_COUNT );
   data_.Forge();
   data_.Fill( 0 );
   CountType* data = static_cast< CountType* >( data_.Origin() );
   auto in = featureValues.FirstObject();
   while( in ) {
      auto tin = in.begin();
      dip::sint offset = 0;
      bool include = true;
      for( dip::uint jj = 0; jj < ndims; ++jj ) {
         if( configuration[ jj ].IsOutOfRange( *tin )) {
            include = false;
            break;
         }
         offset += data_.Stride( jj ) * configuration[ jj ].FindBin( *tin );
         ++tin;
      }
      if( include ) {
         ++data[ offset ];
      }
      ++in;
   }
}

void Histogram::EmptyHistogram( Histogram::ConfigurationArray configuration ) {
   dip::uint ndims = configuration.size();
   lowerBounds_.resize( ndims );
   binSizes_.resize( ndims );
   UnsignedArray sizes( ndims );
   for( dip::uint ii = 0; ii < ndims; ++ii ) {
      DIP_STACK_TRACE_THIS( configuration[ ii ].Complete( false ));
      lowerBounds_[ ii ] = configuration[ ii ].lowerBound;
      binSizes_[ ii ] = configuration[ ii ].binSize;
      sizes[ ii ] = configuration[ ii ].nBins;
   }
   data_.ReForge( sizes, 1, DT_COUNT );
   data_.Fill( 0 );
}

void Histogram::HistogramFromDataPointer( Histogram::CountType const* data, Histogram::Configuration const& configuration ) {
   lowerBounds_.resize( 1 );
   binSizes_.resize( 1 );
   UnsignedArray sizes( 1 );
   lowerBounds_[ 0 ] = configuration.lowerBound;
   binSizes_[ 0 ] = configuration.binSize;
   sizes[ 0 ] = configuration.nBins;
   data_ = Image( data, sizes );
   DIP_ASSERT( data_.DataType() == DT_COUNT );
}

namespace {

template< typename TPI >
class ReverseLookupLineFilter : public Framework::ScanLineFilter {
   public:
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint tensorElements ) override {
         return tensorElements * 6;
      }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         dip::sint inStride = params.inBuffer[ 0 ].stride;
         dip::uint nDims = params.inBuffer[ 0 ].tensorLength;
         dip::sint tensorStride = params.inBuffer[ 0 ].tensorStride;
         CountType* out = static_cast< CountType* >( params.outBuffer[ 0 ].buffer );
         dip::sint outStride = params.outBuffer[ 0 ].stride;
         dip::uint bufferLength = params.bufferLength;
         CountType* data = static_cast< CountType* >( histogram_.Origin() );
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            bool include = true;
            TPI const* in_t = in;
            for( dip::uint jj = 0; jj < nDims; ++jj, in_t += tensorStride ) {
               if( configuration_[ jj ].IsOutOfRange( static_cast< dfloat >( *in_t ))) {
                  include = false;
                  break;
               }
            }
            if( include ) {
               dip::sint offset = 0;
               in_t = in;
               for( dip::uint jj = 0; jj < nDims; ++jj, in_t += tensorStride ) {
                  offset += histogram_.Stride( jj ) * configuration_[ jj ].FindBin( static_cast< dfloat >( *in_t ));
               }
               *out = data[ offset ];
            } else {
               *out = 0;
            }
            in += inStride;
            out += outStride;
         }
      }
      ReverseLookupLineFilter( Image const& histogram, Histogram::ConfigurationArray const& configuration ) :
            histogram_( histogram ), configuration_( configuration ) {}
   private:
      Image const& histogram_;
      Histogram::ConfigurationArray const& configuration_;
};

}

void Histogram::ReverseLookup( Image const& input, Image& out, BooleanArray excludeOutOfBoundValues ) {
   // Check inputs
   DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
   DIP_THROW_IF( !input.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !input.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint nDims = Dimensionality();
   DIP_THROW_IF( input.TensorElements() != nDims, E::NTENSORELEM_DONT_MATCH );

   // Compute ConfigurationArray for the histogram
   DIP_STACK_TRACE_THIS( ArrayUseParameter( excludeOutOfBoundValues, nDims, false ));
   ConfigurationArray configuration( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      configuration[ ii ].lowerBound = lowerBounds_[ ii ];
      configuration[ ii ].upperBound = lowerBounds_[ ii ] + binSizes_[ ii ] * static_cast< dfloat >( data_.Size( ii ));
      configuration[ ii ].nBins = data_.Size( ii );
      configuration[ ii ].binSize = binSizes_[ ii ];
      configuration[ ii ].excludeOutOfBoundValues = excludeOutOfBoundValues[ ii ];
   }

   // Create and call ReverseLookupLineFilter
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_REAL( scanLineFilter, ReverseLookupLineFilter, ( data_, configuration ), input.DataType() );
   ImageRefArray outar{ out };
   DIP_STACK_TRACE_THIS( Framework::Scan( { input }, outar, { input.DataType() }, { DT_COUNT }, { DT_COUNT }, { 1 }, *scanLineFilter ));
}

dip::uint Histogram::Count() const {
   DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
   return Sum( data_ ).As< dip::uint >();
}

Histogram& Histogram::Cumulative() {
   DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
   data_.Protect();
   CumulativeSum( data_, {}, data_ );
   data_.Protect( false );
   return *this;
}

Histogram Histogram::GetMarginal( dip::uint dim ) const {
   DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
   DIP_THROW_IF( dim >= Dimensionality(), E::INVALID_PARAMETER );
   Histogram out{ Configuration( lowerBounds_[ dim ], data_.Size( dim ), binSizes_[ dim ] ) };
   BooleanArray ps( Dimensionality(), true );
   ps[ dim ] = false;
   out.data_.Strip();
   out.data_.Protect(); // so that Sum() produces a DT_COUNT image.
   Sum( data_, {}, out.data_, ps );
   out.data_.Protect( false );
   out.data_.PermuteDimensions( { dim } );
   return out;
}

Histogram& Histogram::Smooth( FloatArray sigma ) {
   DIP_THROW_IF( !IsInitialized(), E::HISTOGRAM_NOT_INITIALIZED );
   UnsignedArray sizes = data_.Sizes();
   dip::uint nDims = sizes.size();
   DIP_STACK_TRACE_THIS( ArrayUseParameter( sigma, nDims, 1.0 ));
   dfloat truncation = 3.0;
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      dfloat extension = std::ceil( sigma[ ii ] * truncation );
      sizes[ ii ] += 2 * static_cast< dip::uint >( extension );
      lowerBounds_[ ii ] -= binSizes_[ ii ] * extension;
   }
   data_ = data_.Pad( sizes );
   data_.Protect(); // so that GaussFIR() produces a DT_COUNT image.
   GaussFIR( data_, data_, sigma, { 0 }, { S::ADD_ZEROS }, truncation );
   data_.Protect( false );
   return *this;
}

} // namespace dip


#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/iterators.h"
#include "diplib/random.h"

DOCTEST_TEST_CASE( "[DIPlib] testing dip::Histogram" ) {
   dip::Image zero( {}, 1, dip::DT_SFLOAT );
   zero = 0;
   dip::Histogram zeroH( zero );
   DOCTEST_CHECK( zeroH.Dimensionality() == 1 );
   DOCTEST_CHECK( zeroH.Bins() == 256 );
   DOCTEST_CHECK( zeroH.BinSize() == 1.0 / 256.0 );
   DOCTEST_CHECK( zeroH.LowerBound() == 0.0 );
   DOCTEST_CHECK( zeroH.UpperBound() == 1.0 );
   DOCTEST_CHECK( zeroH.Count() == 1 );
   DOCTEST_CHECK( zeroH.At( 0 ) == 1 );
   DOCTEST_CHECK( zeroH.At( 1 ) == 0 );

   dip::dfloat meanval = 2000.0;
   dip::dfloat sigma = 500.0;
   dip::Image img( { 180, 160, 80 }, 1, dip::DT_UINT16 );
   {
      dip::Random random( 0 );
      dip::GaussianRandomGenerator normDist( random );
      dip::ImageIterator< dip::uint16 > it( img );
      do {
         *it = dip::clamp_cast< dip::uint16 >( normDist( meanval, sigma ));
      } while( ++it );
   }
   dip::dfloat upperBound = 2 * meanval;
   dip::uint nBins = 200;
   dip::dfloat binSize = upperBound / static_cast< dip::dfloat >( nBins );
   dip::Histogram::Configuration settings( 0.0, upperBound, nBins );
   dip::Histogram gaussH( img, {}, settings );
   DOCTEST_CHECK( gaussH.Dimensionality() == 1 );
   DOCTEST_CHECK( gaussH.Bins() == nBins );
   DOCTEST_CHECK( gaussH.BinSize() == binSize );
   DOCTEST_CHECK( gaussH.LowerBound() == 0.0 );
   DOCTEST_CHECK( gaussH.UpperBound() == upperBound );
   DOCTEST_CHECK( gaussH.Count() == img.NumberOfPixels() );
   DOCTEST_CHECK( std::abs( dip::Mean( gaussH )[ 0 ] - meanval ) < 1.0 ); // less than 0.05% error.
   DOCTEST_CHECK( std::abs( dip::MarginalMedian( gaussH )[ 0 ] - meanval ) <= binSize );
   DOCTEST_CHECK( std::abs( dip::MarginalPercentile( gaussH, 25 )[ 0 ] - ( meanval - 0.675 * sigma )) <= binSize );
   DOCTEST_CHECK( std::abs( dip::Mode( gaussH )[ 0 ] - meanval ) <= 3.0 * binSize );
   DOCTEST_CHECK( std::abs( dip::Covariance( gaussH )[ 0 ] - sigma * sigma ) < 500.0 ); // less than 0.2% error.

   dip::Image mask = img > meanval;
   dip::Histogram halfGaussH( img, mask, settings );
   DOCTEST_CHECK( halfGaussH.Dimensionality() == 1 );
   DOCTEST_CHECK( halfGaussH.Bins() == nBins );
   DOCTEST_CHECK( halfGaussH.BinSize() == binSize );
   DOCTEST_CHECK( halfGaussH.LowerBound() == 0.0 );
   DOCTEST_CHECK( halfGaussH.UpperBound() == upperBound );
   DOCTEST_CHECK( halfGaussH.Count() == Count( mask ));
   DOCTEST_CHECK( halfGaussH.At( 95 ) == 0 );
   DOCTEST_CHECK( halfGaussH.At( 105 ) == gaussH.At( 105 ));

   auto optimalSettings = dip::Histogram::OptimalConfiguration();
   auto quartiles = dip::Quartiles( img );
   dip::dfloat iqr = quartiles.upperQuartile - quartiles.lowerQuartile;
   // The parameters to this configuration are not trivial, everything is re-computed to make bin size and bin centers integers
   binSize = std::ceil( 2 * iqr / std::cbrt( img.NumberOfSamples() ));
   dip::dfloat lowerBound = std::floor( std::max( quartiles.minimum, quartiles.lowerQuartile - 50 * iqr ));
   upperBound = std::ceil( std::min( quartiles.maximum, quartiles.upperQuartile + 50 * iqr) * ( 1.0 + 1e-15 ));
   binSize = std::ceil( binSize );
   nBins = static_cast< dip::uint >( std::round(( upperBound - lowerBound ) / binSize ));
   upperBound = lowerBound + static_cast< dip::dfloat >( nBins ) * binSize;
   if( static_cast< int >( binSize ) % 2 ) {
      // For odd bin sizes we move bounds down by 0.5
      lowerBound -= 0.5;
      upperBound -= 0.5;
   }
   dip::uint expectedCount = dip::Count(( img >= lowerBound ) & ( img < upperBound ));
   gaussH = dip::Histogram( img, {}, optimalSettings );
   DOCTEST_CHECK( gaussH.Dimensionality() == 1 );
   DOCTEST_CHECK( gaussH.Bins() == nBins );
   DOCTEST_CHECK( gaussH.BinSize() == binSize );
   DOCTEST_CHECK( gaussH.LowerBound() == lowerBound );
   DOCTEST_CHECK( gaussH.UpperBound() == upperBound );
   DOCTEST_CHECK( gaussH.Count() == expectedCount );

   dip::Image complexIm( { 75, 25 }, 3, dip::DT_DCOMPLEX );
   DOCTEST_CHECK_THROWS( dip::Histogram{ complexIm } );

   dip::Image tensorIm( { 275, 225 }, 3, dip::DT_SINT32 );
   {
      dip::Random random( 0 );
      dip::GaussianRandomGenerator normDist( random );
      dip::ImageIterator< dip::sint32 > it( tensorIm );
      do {
         it[ 0 ] = dip::clamp_cast< dip::sint32 >( normDist( meanval, sigma ));
         it[ 1 ] = dip::clamp_cast< dip::sint32 >( normDist( meanval, sigma ));
         it[ 2 ] = 1000;
      } while( ++it );
   }
   nBins = 100;
   upperBound = 2 * meanval;
   binSize = upperBound / static_cast< dip::dfloat >( nBins );
   settings.nBins = nBins;
   dip::Histogram tensorH( tensorIm, {}, settings );
   DOCTEST_CHECK( tensorH.Dimensionality() == 3 );
   DOCTEST_CHECK( tensorH.Bins( 0 ) == nBins );
   DOCTEST_CHECK( tensorH.Bins( 1 ) == nBins );
   DOCTEST_CHECK( tensorH.Bins( 2 ) == nBins );
   DOCTEST_CHECK( tensorH.BinSize( 0 ) == binSize );
   DOCTEST_CHECK( tensorH.BinSize( 1 ) == binSize );
   DOCTEST_CHECK( tensorH.BinSize( 2 ) == binSize );
   DOCTEST_CHECK( tensorH.LowerBound( 0 ) == 0.0 );
   DOCTEST_CHECK( tensorH.UpperBound( 0 ) == upperBound );
   DOCTEST_CHECK( tensorH.LowerBound( 1 ) == 0.0 );
   DOCTEST_CHECK( tensorH.UpperBound( 1 ) == upperBound );
   DOCTEST_CHECK( tensorH.LowerBound( 2 ) == 0.0 );
   DOCTEST_CHECK( tensorH.UpperBound( 2 ) == upperBound );
   DOCTEST_CHECK( tensorH.Count() == tensorIm.NumberOfPixels() );
   dip::Histogram tensorM = tensorH.GetMarginal( 2 );
   DOCTEST_CHECK( tensorM.Dimensionality() == 1 );
   DOCTEST_CHECK( tensorM.Bins() == nBins );
   DOCTEST_CHECK( tensorM.BinSize() == binSize );
   DOCTEST_CHECK( tensorM.LowerBound() == 0.0 );
   DOCTEST_CHECK( tensorM.UpperBound() == upperBound );
   DOCTEST_CHECK( tensorM.GetImage().DataType() == dip::DT_COUNT );
   DOCTEST_CHECK( tensorM.GetImage().Sizes().size() == 1 );
   DOCTEST_CHECK( tensorM.GetImage().Sizes()[0] == nBins );
   DOCTEST_CHECK( tensorM.Count() == tensorIm.NumberOfPixels() );
   dip::uint bin1000 = static_cast< dip::uint >( std::floor( 1000.0 / binSize ));
   DOCTEST_CHECK( tensorM.Bin( 1000.0 ) == bin1000 );
   auto bounds = tensorM.BinBoundaries();
   DOCTEST_CHECK( bounds[ 0 ] == 0.0 );
   DOCTEST_CHECK( bounds[ 1 ] == binSize );
   DOCTEST_CHECK( bounds[ 2 ] == binSize * 2.0 );
   DOCTEST_CHECK( bounds.back() == upperBound );
   DOCTEST_CHECK( tensorM.At( bin1000 ) == tensorIm.NumberOfPixels() );
   auto tensorMean = dip::Mean( tensorH );
   DOCTEST_CHECK( std::abs( tensorMean[ 0 ] - meanval ) < 5.0 );
   DOCTEST_CHECK( std::abs( tensorMean[ 1 ] - meanval ) < 5.0 );
   DOCTEST_CHECK( tensorMean[ 2 ] == 1000.0 + 0.5 * binSize );
   auto tensorMed = dip::MarginalMedian( tensorH );
   DOCTEST_CHECK( std::abs( tensorMed[ 0 ] - meanval ) <= binSize );
   DOCTEST_CHECK( std::abs( tensorMed[ 1 ] - meanval ) <= binSize );
   DOCTEST_CHECK( tensorMed[ 2 ] == 1000.0 + 0.5 * binSize );
   auto tensorMode = dip::Mode( tensorH );
   DOCTEST_CHECK( std::abs( tensorMode[ 0 ] - meanval ) <= 15.0 * binSize ); // We've got few pixels, so this is imprecise
   DOCTEST_CHECK( std::abs( tensorMode[ 1 ] - meanval ) <= 15.0 * binSize );
   DOCTEST_CHECK( tensorMode[ 2 ] == 1000.0 + 0.5 * binSize );
   auto tensorCov = dip::Covariance( tensorH );
   DOCTEST_CHECK( std::abs( tensorCov[ 0 ] - sigma * sigma ) < 5000.0 ); // variance 1st element
   DOCTEST_CHECK( std::abs( tensorCov[ 1 ] - sigma * sigma ) < 5000.0 ); // variance 2nd element
   DOCTEST_CHECK( std::abs( tensorCov[ 3 ] ) < 5000.0 ); // covariance elements 1st & 2nd
   DOCTEST_CHECK( tensorCov[ 2 ] == 0.0 ); // variance 3rd element
   DOCTEST_CHECK( tensorCov[ 4 ] == 0.0 ); // covariance 1st & 3rd
   DOCTEST_CHECK( tensorCov[ 5 ] == 0.0 ); // covariance 2nd & 3rd

   std::vector< dip::Histogram > histograms( 3 );
   DOCTEST_CHECK( !histograms[ 0 ].IsInitialized() );
   DOCTEST_CHECK_THROWS( histograms[ 0 ].Count() );
   DOCTEST_CHECK_THROWS( dip::Mean( histograms[ 0 ] ));
   histograms[ 0 ] = tensorH;
   histograms[ 1 ] = halfGaussH;
   DOCTEST_CHECK( histograms[ 0 ].IsInitialized() );
   DOCTEST_CHECK( histograms[ 0 ].Count() == tensorIm.NumberOfPixels() );
   DOCTEST_CHECK( histograms[ 0 ].Dimensionality() == 3 );
   DOCTEST_CHECK( histograms[ 1 ].Count() == Count( mask ));
   DOCTEST_CHECK( histograms[ 1 ].Dimensionality() == 1 );
   DOCTEST_CHECK_THROWS( histograms[ 2 ].Count() );
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
