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
#include "diplib/statistics.h"
#include "diplib/linear.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {

namespace {

using CountType = Histogram::CountType;

constexpr auto DT_COUNT = DataType( CountType( 0 ));

void CompleteConfiguration( Histogram::Configuration& configuration, bool isInteger ) {
   if( configuration.mode != Histogram::Configuration::Mode::COMPUTE_BINS ) {
      if( configuration.nBins < 1 ) {
         configuration.nBins = 256;
      }
   }
   bool hasNoBinSize = false;
   if( configuration.mode != Histogram::Configuration::Mode::COMPUTE_BINSIZE ) {
      if( configuration.binSize <= 0.0 ) {
         hasNoBinSize = true;
         configuration.binSize = 1.0;
      }
   }
   if(( configuration.mode != Histogram::Configuration::Mode::COMPUTE_LOWER ) &&
      ( configuration.mode != Histogram::Configuration::Mode::COMPUTE_UPPER )) {
      if( configuration.upperBound < configuration.lowerBound ) {
         std::swap( configuration.upperBound, configuration.lowerBound );
      } else if( configuration.upperBound == configuration.lowerBound ) {
         configuration.upperBound += 1.0;
      }
   }
   switch( configuration.mode ) {
      default:
         //case Histogram::Configuration::Mode::COMPUTE_BINSIZE:
         configuration.binSize = ( configuration.upperBound - configuration.lowerBound ) / static_cast< dfloat >( configuration.nBins );
         break;
      case Histogram::Configuration::Mode::COMPUTE_BINS:
         if( hasNoBinSize && isInteger ) {
            // Find a suitable bin size that is power of 2:
            dfloat range = configuration.upperBound - configuration.lowerBound; // an integer value...
            configuration.binSize = std::pow( 2.0, std::ceil( std::log2( range / 256.0 )));
            // Shift lower bound to be a multiple of the binSize:
            configuration.lowerBound = std::floor( configuration.lowerBound / configuration.binSize ) * configuration.binSize;
            // Update range in case we shifted lower bound:
            range = configuration.upperBound - configuration.lowerBound;
            // Find number of bins we need to use:
            configuration.nBins = static_cast< dip::uint >( std::ceil( range / configuration.binSize ));
            // Update upper bound so that it matches what the histogram class would compute:
            configuration.upperBound = configuration.lowerBound + static_cast< dfloat >( configuration.nBins ) * configuration.binSize;
         } else {
            configuration.nBins = static_cast< dip::uint >( std::round( configuration.upperBound - configuration.lowerBound ) / configuration.binSize );
            configuration.binSize = ( configuration.upperBound - configuration.lowerBound ) / static_cast< dfloat >( configuration.nBins );
         }
         break;
      case Histogram::Configuration::Mode::COMPUTE_LOWER:
         configuration.lowerBound = configuration.upperBound - static_cast< dfloat >( configuration.nBins ) * configuration.binSize;
         break;
      case Histogram::Configuration::Mode::COMPUTE_UPPER:
         configuration.upperBound = configuration.lowerBound + static_cast< dfloat >( configuration.nBins ) * configuration.binSize;
         break;
   }
}

void CompleteConfiguration( Image const& input, Image const& mask, Histogram::Configuration& configuration ) {
   if( configuration.lowerIsPercentile && configuration.mode != Histogram::Configuration::Mode::COMPUTE_LOWER ) {
      configuration.lowerBound = Percentile( input, mask, configuration.lowerBound ).As< dfloat >();
   }
   if( configuration.upperIsPercentile && configuration.mode != Histogram::Configuration::Mode::COMPUTE_UPPER ) {
      // NOTE: we increase the upper bound when computed as a percentile, because we do lowerBound <= value < upperBound.
      configuration.upperBound = Percentile( input, mask, configuration.upperBound ).As< dfloat >() * ( 1.0 + 1e-15 );
   }
   CompleteConfiguration( configuration, input.DataType().IsInteger() );
}

void CompleteConfiguration( Measurement::IteratorFeature const& featureValues, Histogram::Configuration& configuration ) {
   if( configuration.lowerIsPercentile && configuration.mode != Histogram::Configuration::Mode::COMPUTE_LOWER ) {
      configuration.lowerBound = Percentile( featureValues, configuration.lowerBound );
   }
   if( configuration.upperIsPercentile && configuration.mode != Histogram::Configuration::Mode::COMPUTE_UPPER ) {
      // NOTE: we increase the upper bound when computed as a percentile, because we do lowerBound <= value < upperBound.
      configuration.upperBound = Percentile( featureValues, configuration.upperBound ) * ( 1.0 + 1e-15 );
   }
   CompleteConfiguration( configuration, false );
}

inline dip::sint FindBin( dfloat value, dfloat lowerBound, dfloat binSize, dip::uint nBins ) {
   return static_cast< dip::sint >( clamp( std::floor(( value - lowerBound ) / binSize ), 0.0, static_cast< dfloat >( nBins - 1 )));
}

class dip__HistogramBase : public Framework::ScanLineFilter {
   public:
      dip__HistogramBase( Image& image ) : image_( image ) {}
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         for( dip::uint ii = 1; ii < threads; ++ii ) {
            imageArray_.emplace_back( image_ );       // makes a copy; image_ is not yet forged, so data is not shared.
         }
         // We don't forge the images here, the Filter() function should do that so each thread allocates its own
         // data segment. This ensures there's no false sharing.
      }
      void Reduce() {
         for( auto& img : imageArray_ ) {
            image_ += img;
         }
      }
   protected:
      Image& image_;
      ImageArray imageArray_;
};

template< typename TPI >
class dip__ScalarImageHistogram : public dip__HistogramBase {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 6; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
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
                  if( *mask && ( *in >= configuration_.lowerBound ) && ( *in < configuration_.upperBound )) {
                     ++data[ FindBin( *in, configuration_.lowerBound, configuration_.binSize, configuration_.nBins ) ];
                  }
                  in += inStride;
                  mask += maskStride;
               }
            } else {
               for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
                  if( *mask ) {
                     ++data[ FindBin( *in, configuration_.lowerBound, configuration_.binSize, configuration_.nBins ) ];
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
                     ++data[ FindBin( *in, configuration_.lowerBound, configuration_.binSize, configuration_.nBins ) ];
                  }
                  in += inStride;
               }
            } else {
               for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
                  ++data[ FindBin( *in, configuration_.lowerBound, configuration_.binSize, configuration_.nBins ) ];
                  in += inStride;
               }
            }
         }
      }
      dip__ScalarImageHistogram( Image& image, Histogram::Configuration const& configuration ) :
            dip__HistogramBase( image ), configuration_( configuration ) {}
   private:
      Histogram::Configuration const& configuration_;
};

template< typename TPI >
class dip__JointImageHistogram : public dip__HistogramBase {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint tensorElements ) override {
         return ( tensorInput_ ? tensorElements : 2 ) * 6;
      }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         std::vector< TPI const* >in;
         std::vector< dip::sint >stride;
         dip::uint nDims;
         dip::uint maskBuffer;
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
                     if( configuration_[ jj ].excludeOutOfBoundValues ) {
                        if(( *( in[ jj ] ) < configuration_[ jj ].lowerBound ) || ( *( in[ jj ] ) >= configuration_[ jj ].upperBound )) {
                           include = false;
                           break;
                        }
                     }
                  }
                  if( include ) {
                     dip::sint offset = 0;
                     for( dip::uint jj = 0; jj < nDims; ++jj ) {
                        offset += image_.Stride( jj ) *
                                  FindBin( *( in[ jj ] ), configuration_[ jj ].lowerBound, configuration_[ jj ].binSize, configuration_[ jj ].nBins );
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
                  if( configuration_[ jj ].excludeOutOfBoundValues ) {
                     if(( *( in[ jj ] ) < configuration_[ jj ].lowerBound ) || ( *( in[ jj ] ) >= configuration_[ jj ].upperBound )) {
                        include = false;
                        break;
                     }
                  }
               }
               if( include ) {
                  dip::sint offset = 0;
                  for( dip::uint jj = 0; jj < nDims; ++jj ) {
                     offset += image_.Stride( jj ) *
                               FindBin( *( in[ jj ] ), configuration_[ jj ].lowerBound, configuration_[ jj ].binSize, configuration_[ jj ].nBins );
                  }
                  ++data[ offset ];
               }
               for( dip::uint jj = 0; jj < nDims; ++jj ) {
                  in[ jj ] += stride[ jj ];
               }
            }
         }
      }
      dip__JointImageHistogram( Image& image, Histogram::ConfigurationArray const& configuration, bool tensorInput ) :
            dip__HistogramBase( image ), configuration_( configuration ), tensorInput_( tensorInput ) {}
   private:
      Histogram::ConfigurationArray const& configuration_;
      bool tensorInput_;
};

} // namespace

void Histogram::ScalarImageHistogram( Image const& input, Image const& mask, Histogram::Configuration& configuration ) {
   DIP_STACK_TRACE_THIS( CompleteConfiguration( input, mask, configuration ));
   lowerBounds_ = { configuration.lowerBound };
   binSizes_ = { configuration.binSize };
   data_.SetSizes( { configuration.nBins } );
   data_.SetDataType( DT_COUNT );
   std::unique_ptr< dip__HistogramBase >scanLineFilter;
   DIP_OVL_NEW_REAL( scanLineFilter, dip__ScalarImageHistogram, ( data_, configuration ), input.DataType() );
   DIP_STACK_TRACE_THIS( Framework::ScanSingleInput( input, mask, input.DataType(), *scanLineFilter ));
   scanLineFilter->Reduce();
}

void Histogram::TensorImageHistogram( Image const& input, Image const& mask, Histogram::ConfigurationArray& configuration ) {
   dip::uint ndims = input.TensorElements();
   lowerBounds_.resize( ndims );
   binSizes_.resize( ndims );
   UnsignedArray sizes( ndims, 1 );
   for( dip::uint ii = 0; ii < ndims; ++ii ) {
      DIP_STACK_TRACE_THIS( CompleteConfiguration( input[ ii ], mask, configuration[ ii ] ));
      lowerBounds_[ ii ] = configuration[ ii ].lowerBound;
      binSizes_[ ii ] = configuration[ ii ].binSize;
      sizes[ ii ] = configuration[ ii ].nBins;
   }
   data_.SetSizes( sizes );
   data_.SetDataType( DT_COUNT );
   std::unique_ptr< dip__HistogramBase >scanLineFilter;
   DIP_OVL_NEW_REAL( scanLineFilter, dip__JointImageHistogram, ( data_, configuration, true ), input.DataType() );
   DIP_STACK_TRACE_THIS( Framework::ScanSingleInput( input, mask, input.DataType(), *scanLineFilter ));
   scanLineFilter->Reduce();
}

void Histogram::JointImageHistogram( Image const& input1, Image const& input2, Image const& c_mask, Histogram::ConfigurationArray& configuration ) {
   DIP_START_STACK_TRACE
      CompleteConfiguration( input1, c_mask, configuration[ 0 ] );
      CompleteConfiguration( input2, c_mask, configuration[ 1 ] );
   DIP_END_STACK_TRACE
   lowerBounds_ = { configuration[ 0 ].lowerBound, configuration[ 1 ].lowerBound };
   binSizes_ = { configuration[ 0 ].binSize, configuration[ 1 ].binSize };
   UnsignedArray sizes{ configuration[ 0 ].nBins, configuration[ 1 ].nBins };
   data_.SetSizes( sizes );
   data_.SetDataType( DT_COUNT );
   DataType dtype = DataType::SuggestDyadicOperation( input1.DataType(), input2.DataType() );
   std::unique_ptr< dip__HistogramBase >scanLineFilter;
   DIP_OVL_NEW_REAL( scanLineFilter, dip__JointImageHistogram, ( data_, configuration, false ), dtype );
   ImageConstRefArray inar{ input1, input2 };
   DataTypeArray inBufT{ dtype, dtype };
   Image mask;
   if( c_mask.IsForged() ) {
      // If we have a mask, add it to the input array.
      mask = c_mask.QuickCopy();
      DIP_START_STACK_TRACE
         mask.CheckIsMask( input1.Sizes(), Option::AllowSingletonExpansion::DO_ALLOW, Option::ThrowException::DO_THROW );
         mask.ExpandSingletonDimensions( input1.Sizes() );
      DIP_END_STACK_TRACE
      inar.push_back( mask );
      inBufT.push_back( mask.DataType() );
   }
   ImageRefArray outar{};
   DIP_STACK_TRACE_THIS( Framework::Scan( inar, outar, inBufT, {}, {}, {}, *scanLineFilter ));
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
         CompleteConfiguration( featureColumn, configuration[ ii ] );
      DIP_END_STACK_TRACE
      lowerBounds_[ ii ] = configuration[ ii ].lowerBound;
      binSizes_[ ii ] = configuration[ ii ].binSize;
      sizes[ ii ] = configuration[ ii ].nBins;
   }
   data_.SetSizes( sizes );
   data_.SetDataType( DT_COUNT );
   CountType* data = static_cast< CountType* >( data_.Origin() );
   auto in = featureValues.FirstObject();
   while( in ) {
      auto tin = in.begin();
      dip::sint offset = 0;
      bool include = true;
      for( dip::uint jj = 0; jj < ndims; ++jj ) {
         if( configuration[ jj ].excludeOutOfBoundValues ) {
            if(( *tin < configuration[ jj ].lowerBound ) || ( *tin >= configuration[ jj ].upperBound )) {
               include = false;
               break;
            }
         }
         offset += data_.Stride( jj ) *
                   dip::FindBin( *tin, configuration[ jj ].lowerBound, configuration[ jj ].binSize, configuration[ jj ].nBins );
         ++tin;
      }
      if( include ) {
         ++data[ offset ];
      }
      ++in;
   }
}

dip::uint Histogram::Count() const {
   return Sum( data_ ).As< dip::uint >();
}

Histogram Histogram::Cumulative() const {
   Histogram out = *this;
   out.data_.Strip();
   out.data_.Protect();
   CumulativeSum( data_, {}, out.data_ );
   out.data_.Protect( false );
   return out;
}

Histogram Histogram::Marginal( dip::uint dim ) const {
   DIP_THROW_IF( dim >= Dimensionality(), E::PARAMETER_OUT_OF_RANGE );
   Histogram out = *this;
   BooleanArray ps( Dimensionality(), true );
   ps[ dim ] = false;
   out.data_.Strip();
   out.data_.Protect(); // so that Sum() produces a DT_COUNT image.
   Sum( data_, {}, out.data_, ps );
   out.data_.Protect( false );
   out.data_.PermuteDimensions( { dim } );
   out.lowerBounds_ = { lowerBounds_[ dim ] };
   out.binSizes_ = { binSizes_[ dim ] };
   return out;
}

Histogram Histogram::Smooth( FloatArray sigma ) const {
   Histogram out = *this;
   UnsignedArray sizes = out.data_.Sizes();
   dip::uint nDims = sizes.size();
   DIP_STACK_TRACE_THIS( ArrayUseParameter( sigma, nDims, 1.0 ));
   dfloat truncation = 3.0;
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      dfloat extension = std::ceil( sigma[ ii ] * truncation );
      sizes[ ii ] += 2 * static_cast< dip::uint >( extension );
      out.lowerBounds_[ ii ] -= out.binSizes_[ ii ] * extension;
   }
   out.data_ = out.data_.Pad( sizes );
   out.data_.Protect(); // so that GaussFIR() produces a DT_COUNT image.
   GaussFIR( out.data_, out.data_, sigma, { 0 }, { "add zeros" }, truncation );
   out.data_.Protect( false );
   return out;
}

} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
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
         *it = dip::clamp_cast< dip::uint16 >( normDist( meanval, sigma ) );
      } while( ++it );
   }
   dip::dfloat upperBound = 2 * meanval;
   dip::uint nBins = 200;
   dip::dfloat binSize = upperBound / static_cast< dip::dfloat >( nBins );
   dip::Histogram::Configuration settings( 0.0, upperBound, ( int )nBins );
   dip::Histogram gaussH( img, {}, settings );
   DOCTEST_CHECK( gaussH.Dimensionality() == 1 );
   DOCTEST_CHECK( gaussH.Bins() == nBins );
   DOCTEST_CHECK( gaussH.BinSize() == binSize );
   DOCTEST_CHECK( gaussH.LowerBound() == 0.0 );
   DOCTEST_CHECK( gaussH.UpperBound() == upperBound );
   DOCTEST_CHECK( gaussH.Count() == img.NumberOfPixels() );
   DOCTEST_CHECK( std::abs( dip::Mean( gaussH )[ 0 ] - meanval ) < 1.0 ); // less than 0.05% error.
   DOCTEST_CHECK( std::abs( dip::MarginalMedian( gaussH )[ 0 ] - meanval ) <= binSize );
   DOCTEST_CHECK( std::abs( dip::Mode( gaussH )[ 0 ] - meanval ) <= 3.0 * binSize );
   DOCTEST_CHECK( std::abs( dip::Covariance( gaussH )[ 0 ] - sigma * sigma ) < 500.0 ); // less than 0.2% error.

   dip::Image mask = img > meanval;
   dip::Histogram halfGaussH( img, mask, settings );
   DOCTEST_CHECK( halfGaussH.Dimensionality() == 1 );
   DOCTEST_CHECK( halfGaussH.Bins() == nBins );
   DOCTEST_CHECK( halfGaussH.BinSize() == binSize );
   DOCTEST_CHECK( halfGaussH.LowerBound() == 0.0 );
   DOCTEST_CHECK( halfGaussH.UpperBound() == upperBound );
   DOCTEST_CHECK( halfGaussH.Count() == Count( mask ) );
   DOCTEST_CHECK( halfGaussH.At( 95 ) == 0 );
   DOCTEST_CHECK( halfGaussH.At( 105 ) == gaussH.At( 105 ) );

   dip::Image complexIm( { 75, 25 }, 3, dip::DT_DCOMPLEX );
   DOCTEST_CHECK_THROWS( dip::Histogram{ complexIm } );

   dip::Image tensorIm( { 275, 225 }, 3, dip::DT_SINT32 );
   {
      dip::Random random( 0 );
      dip::GaussianRandomGenerator normDist( random );
      dip::ImageIterator< dip::sint32 > it( tensorIm );
      do {
         it[ 0 ] = dip::clamp_cast< dip::sint32 >( normDist( meanval, sigma ) );
         it[ 1 ] = dip::clamp_cast< dip::sint32 >( normDist( meanval, sigma ) );
         it[ 2 ] = 1000;
      } while( ++it );
   }
   nBins = 100;
   settings.nBins = nBins;
   binSize = upperBound / static_cast< dip::dfloat >( nBins );
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
   dip::Histogram tensorM = tensorH.Marginal( 2 );
   DOCTEST_CHECK( tensorM.Dimensionality() == 1 );
   DOCTEST_CHECK( tensorM.Bins() == nBins );
   DOCTEST_CHECK( tensorM.BinSize() == binSize );
   DOCTEST_CHECK( tensorM.LowerBound() == 0.0 );
   DOCTEST_CHECK( tensorM.UpperBound() == upperBound );
   DOCTEST_CHECK( tensorM.Count() == tensorIm.NumberOfPixels() );
   dip::uint bin1000 = static_cast< dip::uint >( std::floor( 1000.0 / binSize ));
   DOCTEST_CHECK( tensorM.Bin( 1000.0 ) == bin1000 );
   auto bounds = tensorM.BinBoundaries();
   DOCTEST_CHECK( bounds[ 0 ] == 0.0 );
   DOCTEST_CHECK( bounds[ 1 ] == binSize );
   DOCTEST_CHECK( bounds[ 2 ] == binSize * 2.0 );
   DOCTEST_CHECK( bounds.back() == upperBound );
   DOCTEST_CHECK( tensorM.At( bin1000 ) == tensorIm.NumberOfPixels());
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
}

#endif // DIP__ENABLE_DOCTEST
