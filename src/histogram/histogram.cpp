/*
 * DIPlib 3.0
 * This file contains definitions for histogram-related functionality.
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
#include "diplib/histogram.h"
#include "diplib/framework.h"
#include "diplib/math.h"
#include "diplib/overload.h"

namespace dip {

namespace {

void CompleteConfiguration( Image const& input, Image const& mask, Histogram::Configuration& configuration ) {
   if( configuration.lowerIsPercentile && configuration.mode != Histogram::Configuration::Mode::COMPUTE_LOWER ) {
      configuration.lowerBound = Percentile( input, mask, configuration.lowerBound ).As< dfloat >();
   }
   if( configuration.upperIsPercentile && configuration.mode != Histogram::Configuration::Mode::COMPUTE_UPPER ) {
      configuration.upperBound = Percentile( input, mask, configuration.upperBound ).As< dfloat >();
   }
   if( configuration.mode != Histogram::Configuration::Mode::COMPUTE_BINS ) {
      if( configuration.nBins < 1 ) {
         configuration.nBins = 256;
      }
   }
   if( configuration.mode != Histogram::Configuration::Mode::COMPUTE_BINSIZE ) {
      if( configuration.binSize <= 0 ) {
         configuration.binSize = 1;
      }
   }
   if(( configuration.mode != Histogram::Configuration::Mode::COMPUTE_LOWER ) &&
      ( configuration.mode != Histogram::Configuration::Mode::COMPUTE_UPPER )) {
      if( configuration.upperBound < configuration.lowerBound ) {
         std::swap( configuration.upperBound, configuration.lowerBound );
      } else if( configuration.upperBound == configuration.lowerBound ) {
         configuration.upperBound += 1;
      }
   }
   switch( configuration.mode ) {
      default:
      //case Histogram::Configuration::Mode::COMPUTE_BINSIZE:
         configuration.binSize = ( configuration.upperBound - configuration.lowerBound ) / static_cast< dfloat >( configuration.nBins );
         break;
      case Histogram::Configuration::Mode::COMPUTE_BINS:
         configuration.nBins = static_cast< dip::uint >( std::round( configuration.upperBound - configuration.lowerBound ) / configuration.binSize );
         configuration.binSize = ( configuration.upperBound - configuration.lowerBound ) / static_cast< dfloat >( configuration.nBins );
         break;
      case Histogram::Configuration::Mode::COMPUTE_LOWER:
         configuration.lowerBound = configuration.upperBound - static_cast< dfloat >( configuration.nBins ) * configuration.binSize;
         break;
      case Histogram::Configuration::Mode::COMPUTE_UPPER:
         configuration.upperBound = configuration.lowerBound + static_cast< dfloat >( configuration.nBins ) * configuration.binSize;
         break;
   }
}

inline dfloat FindBin( dfloat value, dfloat lowerBound, dfloat binSize ) {
   return std::floor(( value - lowerBound ) / binSize );
}
inline dfloat ClampedBin( dfloat bin, dip::uint lastBin ) {
   return clamp( bin, 0.0, static_cast< dfloat >( lastBin ));
}

template< typename TPI >
class dip__ScalarImageHistogram : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         auto bufferLength = params.bufferLength;
         auto inStride = params.inBuffer[ 0 ].stride;
         uint32* data = static_cast< uint32* >( image_.Origin() ) + image_.Stride( 1 ) * static_cast< dip::sint >( params.thread );
         // Note: `image_` strides are always normal.
         if( params.inBuffer.size() > 1 ) {
            // If there's two input buffers, we have a mask image.
            bin const* mask = static_cast< bin const* >( params.inBuffer[ 1 ].buffer );
            auto maskStride = params.inBuffer[ 1 ].stride;
            if( configuration_.excludeOutOfBoundValues ) {
               for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
                  if( *mask && ( *in >= configuration_.lowerBound ) && ( *in < configuration_.upperBound )) {
                     ++data[ static_cast< dip::uint >(
                                   FindBin( *in, configuration_.lowerBound, configuration_.binSize  )) ];
                  }
                  in += inStride;
                  mask += maskStride;
               }
            } else {
               for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
                  if( *mask ) {
                     ++data[ static_cast< dip::uint >( ClampedBin(
                                    FindBin( *in, configuration_.lowerBound, configuration_.binSize ),
                                    configuration_.nBins - 1 )) ];
                  }
                  in += inStride;
                  mask += maskStride;
               }
            }
         } else {
            // Otherwise we don't.
            if( configuration_.excludeOutOfBoundValues ) {
               for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
                  if(( *in >= configuration_.lowerBound ) && ( *in < configuration_.upperBound )) {
                     ++data[ static_cast< dip::uint >(
                                   FindBin( *in, configuration_.lowerBound, configuration_.binSize  )) ];
                  }
                  in += inStride;
               }
            } else {
               for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
                  ++data[ static_cast< dip::uint >( ClampedBin(
                                FindBin( *in, configuration_.lowerBound, configuration_.binSize ),
                                configuration_.nBins - 1 )) ];
                  in += inStride;
               }
            }
         }
      }
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         if( threads > 1 ) {
            UnsignedArray sizes = image_.Sizes();
            sizes.back() = threads;
            image_.SetSizes( sizes );
         }
         image_.Forge();
         image_.Fill( 0 );
      }
      dip__ScalarImageHistogram( Image& image, Histogram::Configuration const& configuration ) :
            image_( image ), configuration_( configuration ) {}
   private:
      Image& image_;
      Histogram::Configuration const& configuration_;
};

template< typename TPI >
class dip__TensorImageHistogram : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         auto bufferLength = params.bufferLength;
         auto inStride = params.inBuffer[ 0 ].stride;
         auto tensorLength = params.inBuffer[ 0 ].tensorLength;
         auto tensorStride = params.inBuffer[ 0 ].tensorStride;
         uint32* data = static_cast< uint32* >( image_.Origin() ) + image_.Strides().back() * static_cast< dip::sint >( params.thread );
         if( params.inBuffer.size() > 1 ) {
            // If there's two input buffers, we have a mask image.
            bin const* mask = static_cast< bin const* >( params.inBuffer[ 1 ].buffer );
            auto maskStride = params.inBuffer[ 1 ].stride;
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               if( *mask ) {
                  TPI const* tin = in;
                  dip::sint offset = 0;
                  bool include = true;
                  for( dip::uint jj = 0; jj < tensorLength; ++jj ) {
                     dfloat bin;
                     if( configuration_[ jj ].excludeOutOfBoundValues ) {
                        if(( *tin < configuration_[ jj ].lowerBound ) || ( *tin >= configuration_[ jj ].upperBound )) {
                           include = false;
                           break;
                        }
                        bin = FindBin( *in, configuration_[ jj ].lowerBound, configuration_[ jj ].binSize  );
                     } else {
                        bin = ClampedBin( FindBin( *tin, configuration_[ jj ].lowerBound, configuration_[ jj ].binSize ),
                                          configuration_[ jj ].nBins - 1 );
                     }
                     offset += image_.Stride( jj ) * static_cast< dip::sint >( bin );
                     tin += tensorStride;
                  }
                  if( include ) {
                     ++data[ offset ];
                  }
               }
               in += inStride;
               mask += maskStride;
            }
         } else {
            // Otherwise we don't.
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               TPI const* tin = in;
               dip::sint offset = 0;
               bool include = true;
               for( dip::uint jj = 0; jj < tensorLength; ++jj ) {
                  dfloat bin;
                  if( configuration_[ jj ].excludeOutOfBoundValues ) {
                     if(( *tin < configuration_[ jj ].lowerBound ) || ( *tin >= configuration_[ jj ].upperBound )) {
                        include = false;
                        break;
                     }
                     bin = FindBin( *in, configuration_[ jj ].lowerBound, configuration_[ jj ].binSize  );
                  } else {
                     bin = ClampedBin( FindBin( *tin, configuration_[ jj ].lowerBound, configuration_[ jj ].binSize ),
                                       configuration_[ jj ].nBins - 1 );
                  }
                  offset += image_.Stride( jj ) * static_cast< dip::sint >( bin );
                  tin += tensorStride;
               }
               if( include ) {
                  ++data[ offset ];
               }
               in += inStride;
            }
         }
      }
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         if( threads > 1 ) {
            UnsignedArray sizes = image_.Sizes();
            sizes.back() = threads;
            image_.SetSizes( sizes );
         }
         image_.Forge();
         image_.Fill( 0 );
      }
      dip__TensorImageHistogram( Image& image, Histogram::ConfigurationArray const& configuration ) :
            image_( image ), configuration_( configuration ) {}
   private:
      Image& image_;
      Histogram::ConfigurationArray const& configuration_;
};

} // namespace

void Histogram::ScalarImageHistogram( Image const& input, Image const& mask, Histogram::Configuration& configuration ) {
   DIP_START_STACK_TRACE
      CompleteConfiguration( input, mask, configuration );
   DIP_END_STACK_TRACE
   lowerBounds_ = { configuration.lowerBound };
   binSizes_ = { configuration.binSize };
   data_.SetSizes( { configuration.nBins, 1 } );
   data_.SetDataType( DT_UINT32 );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_REAL( scanLineFilter, dip__ScalarImageHistogram, ( data_, configuration ), input.DataType() );
   DIP_START_STACK_TRACE
      Framework::ScanSingleInput( input, mask, input.DataType(), *scanLineFilter );
   DIP_END_STACK_TRACE
   if( data_.Size( 1 ) > 1 ) {
      data_ = Sum( data_, {}, { false, true } );
   }
   data_.Squeeze( 1 );
}

void Histogram::TensorImageHistogram( Image const& input, Image const& mask, Histogram::ConfigurationArray& configuration ) {
   dip::uint ndims = input.TensorElements();
   lowerBounds_.resize( ndims );
   binSizes_.resize( ndims );
   UnsignedArray sizes( ndims+1, 1 );
   for( dip::uint ii = 0; ii < ndims; ++ii ) {
      DIP_START_STACK_TRACE
         CompleteConfiguration( input[ ii ], mask, configuration[ ii ] );
      DIP_END_STACK_TRACE
      lowerBounds_[ ii ] = configuration[ ii ].lowerBound;
      binSizes_[ ii ] = configuration[ ii ].binSize;
      sizes[ ii ] = configuration[ ii ].nBins;
   }
   data_.SetSizes( sizes );
   data_.SetDataType( DT_UINT32 );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_REAL( scanLineFilter, dip__TensorImageHistogram, ( data_, configuration ), input.DataType() );
   DIP_START_STACK_TRACE
      Framework::ScanSingleInput( input, mask, input.DataType(), *scanLineFilter );
   DIP_END_STACK_TRACE
   if( data_.Size( ndims ) > 1 ) {
      BooleanArray process( ndims + 1, false );
      process[ ndims ] = true;
      data_ = Sum( data_, {}, process );
   }
   data_.Squeeze( ndims );
}

dip::uint Histogram::Count() const {
   return Sum( data_ ).As< dip::uint >();
}

dfloat Histogram::Mean() const {
   DIP_THROW( E::NOT_IMPLEMENTED );
}

Image::Pixel Histogram::Covariance() const {
   DIP_THROW( E::NOT_IMPLEMENTED );
}

Image::Pixel Histogram::MarginalMedian() const {
   // Uses the cumulative histogram, look at the last line along each dimension
   DIP_THROW( E::NOT_IMPLEMENTED );
}

DIP_EXPORT Image::Pixel Histogram::Mode() const {
   DIP_THROW( E::NOT_IMPLEMENTED );
}

Histogram Histogram::Cumulative() const {
   Histogram out = *this;
   out.data_ = CumulativeSum( data_ );
   return out;
}

} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include <random>

DOCTEST_TEST_CASE( "[DIPlib] testing dip::Histogram" ) {
   dip::Image zero( {}, 1, dip::DT_SFLOAT );
   zero = 0;
   dip::Histogram zeroH( zero );
   DOCTEST_CHECK( zeroH.Dimensionality() == 1 );
   DOCTEST_CHECK( zeroH.Bins() == 256 );
   DOCTEST_CHECK( zeroH.BinSize() == 1.0 );
   DOCTEST_CHECK( zeroH.LowerBound() == 0.0 );
   DOCTEST_CHECK( zeroH.UpperBound() == 256.0 );
   DOCTEST_CHECK( zeroH.Count() == 1 );
   DOCTEST_CHECK( zeroH.At( 0 ) == 1 );
   DOCTEST_CHECK( zeroH.At( 1 ) == 0 );

   dip::dfloat meanval = 2000.0;
   dip::Image img( { 80, 6, 5 }, 1, dip::DT_UINT16 );
   {
      std::random_device rd;
      std::mt19937 gen( rd() );
      std::normal_distribution< dip::dfloat > normDist( meanval, 500.0 );
      dip::ImageIterator< dip::uint16 > it( img );
      do {
         *it = dip::clamp_cast< dip::uint16 >( normDist( gen ) );
      } while( ++it );
   }
   dip::Histogram::Configuration settings( 0.0, 4000.0, 200 );
   dip::Histogram gaussH( img, {}, settings );
   DOCTEST_CHECK( gaussH.Dimensionality() == 1 );
   DOCTEST_CHECK( gaussH.Bins() == 200 );
   DOCTEST_CHECK( gaussH.BinSize() == 20.0 );
   DOCTEST_CHECK( gaussH.LowerBound() == 0.0 );
   DOCTEST_CHECK( gaussH.UpperBound() == 4000.0 );
   DOCTEST_CHECK( gaussH.Count() == 80 * 6 * 5 );
   // TODO: add calls to statistics functions.

   dip::Image mask = img > meanval;
   dip::Histogram halfGaussH( img, mask, settings );
   DOCTEST_CHECK( halfGaussH.Dimensionality() == 1 );
   DOCTEST_CHECK( halfGaussH.Bins() == 200 );
   DOCTEST_CHECK( halfGaussH.BinSize() == 20.0 );
   DOCTEST_CHECK( halfGaussH.LowerBound() == 0.0 );
   DOCTEST_CHECK( halfGaussH.UpperBound() == 4000.0 );
   DOCTEST_CHECK( halfGaussH.Count() == Count( mask ) );
   DOCTEST_CHECK( halfGaussH.At( 95 ) == 0 );
   DOCTEST_CHECK( halfGaussH.At( 105 ) == gaussH.At( 105 ) );

   dip::Image complexIm( { 75, 25 }, 3, dip::DT_DCOMPLEX );
   DOCTEST_CHECK_THROWS( dip::Histogram{ complexIm } );

   dip::Image tensorIm( { 75, 25 }, 3, dip::DT_SINT32 );
   {
      std::random_device rd;
      std::mt19937 gen( rd() );
      std::normal_distribution< dip::dfloat > normDist( meanval, 500.0 );
      dip::ImageIterator< dip::uint16 > it( img );
      do {
         it[ 0 ] = dip::clamp_cast< dip::uint16 >( normDist( gen ) );
         it[ 1 ] = dip::clamp_cast< dip::uint16 >( normDist( gen ) );
         it[ 2 ] = 1000;
      } while( ++it );
   }
   dip::Histogram tensorH( tensorIm, {}, settings );
   DOCTEST_CHECK( tensorH.Dimensionality() == 3 );
   DOCTEST_CHECK( tensorH.Bins( 0 ) == 200 );
   DOCTEST_CHECK( tensorH.Bins( 1 ) == 200 );
   DOCTEST_CHECK( tensorH.Bins( 2 ) == 200 );
   DOCTEST_CHECK( tensorH.BinSize( 0 ) == 20.0 );
   DOCTEST_CHECK( tensorH.BinSize( 1 ) == 20.0 );
   DOCTEST_CHECK( tensorH.BinSize( 2 ) == 20.0 );
   DOCTEST_CHECK( tensorH.LowerBound( 0 ) == 0.0 );
   DOCTEST_CHECK( tensorH.UpperBound( 0 ) == 4000.0 );
   DOCTEST_CHECK( tensorH.LowerBound( 1 ) == 0.0 );
   DOCTEST_CHECK( tensorH.UpperBound( 1 ) == 4000.0 );
   DOCTEST_CHECK( tensorH.LowerBound( 2 ) == 0.0 );
   DOCTEST_CHECK( tensorH.UpperBound( 2 ) == 4000.0 );
   DOCTEST_CHECK( tensorH.Count() == 75 * 25 );

}

#endif // DIP__ENABLE_DOCTEST
