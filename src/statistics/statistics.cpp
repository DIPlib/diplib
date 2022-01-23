/*
 * (c)2016-2017, Cris Luengo.
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
#include "diplib/statistics.h"
#include "diplib/math.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {

namespace {

class CountLineFilter : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 2; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         bin const* in = static_cast< bin const* >( params.inBuffer[ 0 ].buffer );
         dip::uint count = 0;
         auto bufferLength = params.bufferLength;
         auto inStride = params.inBuffer[ 0 ].stride;
         if( params.inBuffer.size() > 1 ) {
            // If there's two input buffers, we have a mask image.
            auto maskStride = params.inBuffer[ 1 ].stride;
            bin const* mask = static_cast< bin const* >( params.inBuffer[ 1 ].buffer );
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               if( *mask && *in ) {
                  ++count;
               }
               in += inStride;
               mask += maskStride;
            }
         } else {
            // Otherwise we don't.
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               if( *in ) {
                  ++count;
               }
               in += inStride;
            }
         }
         counts_[ params.thread ] += count;
      }
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         counts_.resize( threads );
      }
      dip::uint GetResult() {
         dip::uint out = counts_[ 0 ];
         for( dip::uint ii = 1; ii < counts_.size(); ++ii ) {
            out += counts_[ ii ];
         }
         return out;
      }
   private:
      std::vector< dip::uint > counts_;
};

} // namespace

dip::uint Count(
      Image const& in,
      Image const& mask
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   CountLineFilter scanLineFilter;
   DIP_STACK_TRACE_THIS( Framework::ScanSingleInput( in, mask, DT_BIN, scanLineFilter ));
   return scanLineFilter.GetResult();
}

namespace {

class MaxMinPixelLineFilter : public Framework::ScanLineFilter {
   public:
      virtual UnsignedArray GetResult() = 0;
};

template< typename TPI >
class MaxPixelLineFilter : public MaxMinPixelLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 2; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         UnsignedArray coord( params.position.size() );
         TPI value = std::numeric_limits< TPI >::lowest();
         auto bufferLength = params.bufferLength;
         auto inStride = params.inBuffer[ 0 ].stride;
         if( params.inBuffer.size() > 1 ) {
            // If there's two input buffers, we have a mask image.
            auto maskStride = params.inBuffer[ 1 ].stride;
            bin const* mask = static_cast< bin const* >( params.inBuffer[ 1 ].buffer );
            if( first_ ) {
               for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
                  if( *mask && ( *in > value )) {
                     value = *in;
                     coord = params.position;
                     coord[ params.dimension ] += ii;
                  }
                  in += inStride;
                  mask += maskStride;
               }
            } else {
               for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
                  if( *mask && ( *in >= value )) {
                     value = *in;
                     coord = params.position;
                     coord[ params.dimension ] += ii;
                  }
                  in += inStride;
                  mask += maskStride;
               }
            }
         } else {
            // Otherwise we don't.
            if( first_ ) {
               for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
                  if( *in > value ) {
                     value = *in;
                     coord = params.position;
                     coord[ params.dimension ] += ii;
                  }
                  in += inStride;
               }
            } else {
               for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
                  if( *in >= value ) {
                     value = *in;
                     coord = params.position;
                     coord[ params.dimension ] += ii;
                  }
                  in += inStride;
               }
            }
         }
         if( coord_[ params.thread ].empty() ) {
            // Ensure we always have something in `coord_`, even if the whole image is NaN.
            value_[ params.thread ] = value;
            coord_[ params.thread ] = coord;
         } else {
            if( first_ ) {
               if( value > value_[ params.thread ] ) {
                  value_[ params.thread ] = value;
                  coord_[ params.thread ] = coord;
               }
            } else {
               if( value >= value_[ params.thread ] ) {
                  value_[ params.thread ] = value;
                  coord_[ params.thread ] = coord;
               }
            }
         }
      }
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         coord_.resize( threads );
         value_.resize( threads, std::numeric_limits< TPI >::lowest() );
      }
      MaxPixelLineFilter( bool first ) : first_( first ) {}
      virtual UnsignedArray GetResult() override {
         dip::uint index = 0;
         for( dip::uint ii = 1; ii < coord_.size(); ++ii ) {
            if( first_ ? value_[ ii ] > value_[ index ] : value_[ ii ] >= value_[ index ] ) {
               index = ii;
            }
         }
         return coord_[ index ];
      }
   private:
      std::vector< UnsignedArray > coord_;
      std::vector< TPI > value_;
      bool first_;
};

template< typename TPI >
class MinPixelLineFilter : public MaxMinPixelLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 2; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         UnsignedArray coord( params.position.size() );
         TPI value = std::numeric_limits< TPI >::max();
         auto bufferLength = params.bufferLength;
         auto inStride = params.inBuffer[ 0 ].stride;
         if( params.inBuffer.size() > 1 ) {
            // If there's two input buffers, we have a mask image.
            auto maskStride = params.inBuffer[ 1 ].stride;
            bin const* mask = static_cast< bin const* >( params.inBuffer[ 1 ].buffer );
            if( first_ ) {
               for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
                  if( *mask && ( *in < value )) {
                     value = *in;
                     coord = params.position;
                     coord[ params.dimension ] += ii;
                  }
                  in += inStride;
                  mask += maskStride;
               }
            } else {
               for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
                  if( *mask && ( *in <= value )) {
                     value = *in;
                     coord = params.position;
                     coord[ params.dimension ] += ii;
                  }
                  in += inStride;
                  mask += maskStride;
               }
            }
         } else {
            // Otherwise we don't.
            if( first_ ) {
               for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
                  if( *in < value ) {
                     value = *in;
                     coord = params.position;
                     coord[ params.dimension ] += ii;
                  }
                  in += inStride;
               }
            } else {
               for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
                  if( *in <= value ) {
                     value = *in;
                     coord = params.position;
                     coord[ params.dimension ] += ii;
                  }
                  in += inStride;
               }
            }
         }
         if( coord_[ params.thread ].empty() ) {
            // Ensure we always have something in `coord_`, even if the whole image is NaN.
            value_[ params.thread ] = value;
            coord_[ params.thread ] = coord;
         } else {
            if( first_ ) {
               if( value < value_[ params.thread ] ) {
                  value_[ params.thread ] = value;
                  coord_[ params.thread ] = coord;
               }
            } else {
               if( value <= value_[ params.thread ] ) {
                  value_[ params.thread ] = value;
                  coord_[ params.thread ] = coord;
               }
            }
         }
      }
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         coord_.resize( threads );
         value_.resize( threads, std::numeric_limits< TPI >::max() );
      }
      MinPixelLineFilter( bool first ) : first_( first ) {}
      virtual UnsignedArray GetResult() override {
         dip::uint index = 0;
         for( dip::uint ii = 1; ii < coord_.size(); ++ii ) {
            if( first_ ? value_[ ii ] < value_[ index ] : value_[ ii ] <= value_[ index ] ) {
               index = ii;
            }
         }
         return coord_[ index ];
      }
   private:
      std::vector< UnsignedArray > coord_;
      std::vector< TPI > value_;
      bool first_;
};

} // namespace

UnsignedArray MaximumPixel( Image const& in, Image const& mask, String const& positionFlag ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   bool first;
   DIP_STACK_TRACE_THIS( first = BooleanFromString( positionFlag, S::FIRST, S::LAST ));
   DataType dataType = DataType::SuggestReal( in.DataType() );
   std::unique_ptr< MaxMinPixelLineFilter > scanLineFilter;
   DIP_OVL_NEW_REAL( scanLineFilter, MaxPixelLineFilter, ( first ), dataType );
   DIP_STACK_TRACE_THIS( Framework::ScanSingleInput( in, mask, dataType, *scanLineFilter,
                                                     Framework::ScanOption::NeedCoordinates ));
   return scanLineFilter->GetResult();
}

UnsignedArray MinimumPixel( Image const& in, Image const& mask, String const& positionFlag ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   bool first;
   DIP_STACK_TRACE_THIS( first = BooleanFromString( positionFlag, S::FIRST, S::LAST ));
   DataType dataType = DataType::SuggestReal( in.DataType() );
   std::unique_ptr< MaxMinPixelLineFilter > scanLineFilter;
   DIP_OVL_NEW_REAL( scanLineFilter, MinPixelLineFilter, ( first ), dataType );
   DIP_STACK_TRACE_THIS( Framework::ScanSingleInput( in, mask, dataType, *scanLineFilter,
                                                     Framework::ScanOption::NeedCoordinates ));
   return scanLineFilter->GetResult();
}

namespace {

template< typename TPI >
class CumSumFilter : public Framework::SeparableLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint ) override {
         return lineLength;
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::uint length = params.inBuffer.length;
         dip::sint inStride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         TPI sum = 0;
         for( dip::uint ii = 0; ii < length; ++ii ) {
            sum += *in;
            *out = sum;
            in += inStride;
            out += outStride;
         }
      }
};

} // namespace

void CumulativeSum(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray const& process
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( in.Dimensionality() < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DataType dataType = DataType::SuggestFlex( in.DataType() );
   std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
   DIP_OVL_NEW_FLEX( lineFilter, CumSumFilter, (), dataType );
   if( mask.IsForged() ) {
      Select( in, Image( 0, dataType ), mask, out );
      DIP_STACK_TRACE_THIS( Framework::Separable( out, out, dataType, dataType, process, { 0 }, {}, *lineFilter,
                                                  Framework::SeparableOption::AsScalarImage ));
   } else {
      DIP_STACK_TRACE_THIS( Framework::Separable( in, out, dataType, dataType, process, { 0 }, {}, *lineFilter,
                                                  Framework::SeparableOption::AsScalarImage ));
   }
}

namespace {

class MaximumAndMinimumLineFilterBase : public Framework::ScanLineFilter {
   public:
      virtual MinMaxAccumulator GetResult() = 0;
};

template< typename TPI >
class MaximumAndMinimumLineFilter : public MaximumAndMinimumLineFilterBase {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 3; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         MinMaxAccumulator vars;
         auto bufferLength = params.bufferLength;
         auto inStride = params.inBuffer[ 0 ].stride;
         if( params.inBuffer.size() > 1 ) {
            // If there's two input buffers, we have a mask image.
            auto maskStride = params.inBuffer[ 1 ].stride;
            bin const* mask = static_cast< bin const* >( params.inBuffer[ 1 ].buffer );
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               if( *mask ) {
                  vars.Push( static_cast< dfloat >( *in ));
               }
               in += inStride;
               mask += maskStride;
            }
         } else {
            // Otherwise we don't.
            dip::uint ii = 0;
            for( ; ii < bufferLength - 1; ii += 2 ) {
               TPI v = *in;
               in += inStride;
               vars.Push( static_cast< dfloat >( v ), static_cast< dfloat >( *in ));
               in += inStride;
            }
            if( ii < bufferLength ) {
               vars.Push( static_cast< dfloat >( *in ));
            }
         }
         accArray_[ params.thread ] += vars;
      }
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         accArray_.resize( threads );
      }
      virtual MinMaxAccumulator GetResult() override {
         MinMaxAccumulator out = accArray_[ 0 ];
         for( dip::uint ii = 1; ii < accArray_.size(); ++ii ) {
            out += accArray_[ ii ];
         }
         return out;
      }
   private:
      std::vector< MinMaxAccumulator > accArray_;
};

} // namespace

MinMaxAccumulator MaximumAndMinimum(
      Image const& in,
      Image const& mask
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   // In case of complex images, separate them as a new dimension.
   Image c_in = in.QuickCopy();
   if( c_in.DataType().IsComplex() ) {
      c_in.SplitComplex();
      // Note that mask will be singleton-expanded, which allows adding dimensions at the end.
   }
   std::unique_ptr< MaximumAndMinimumLineFilterBase > scanLineFilter;
   DIP_OVL_NEW_NONCOMPLEX( scanLineFilter, MaximumAndMinimumLineFilter, (), c_in.DataType() );
   DIP_STACK_TRACE_THIS( Framework::ScanSingleInput( c_in, mask, c_in.DataType(), *scanLineFilter,
                                                     Framework::ScanOption::TensorAsSpatialDim ));
   return scanLineFilter->GetResult();
}

namespace {

class SampleStatisticsLineFilterBase : public Framework::ScanLineFilter {
   public:
      virtual StatisticsAccumulator GetResult() = 0;
};

template< typename TPI >
class SampleStatisticsLineFilter : public SampleStatisticsLineFilterBase {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 23; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         StatisticsAccumulator vars;
         auto bufferLength = params.bufferLength;
         auto inStride = params.inBuffer[ 0 ].stride;
         if( params.inBuffer.size() > 1 ) {
            // If there's two input buffers, we have a mask image.
            auto maskStride = params.inBuffer[ 1 ].stride;
            bin const* mask = static_cast< bin const* >( params.inBuffer[ 1 ].buffer );
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               if( *mask ) {
                  vars.Push( static_cast< dfloat >( *in ));
               }
               in += inStride;
               mask += maskStride;
            }
         } else {
            // Otherwise we don't.
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               vars.Push( static_cast< dfloat >( *in ));
               in += inStride;
            }
         }
         accArray_[ params.thread ] += vars;
      }
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         accArray_.resize( threads );
      }
      virtual StatisticsAccumulator GetResult() override {
         StatisticsAccumulator out = accArray_[ 0 ];
         for( dip::uint ii = 1; ii < accArray_.size(); ++ii ) {
            out += accArray_[ ii ];
         }
         return out;
      }
   private:
      std::vector< StatisticsAccumulator > accArray_;
};

} // namespace

StatisticsAccumulator SampleStatistics(
      Image const& in,
      Image const& mask
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   std::unique_ptr< SampleStatisticsLineFilterBase > scanLineFilter;
   DIP_OVL_NEW_REAL( scanLineFilter, SampleStatisticsLineFilter, (), in.DataType() );
   DIP_STACK_TRACE_THIS( Framework::ScanSingleInput( in, mask, in.DataType(), *scanLineFilter,
                                                     Framework::ScanOption::TensorAsSpatialDim ));
   return scanLineFilter->GetResult();
}

namespace {

class CovarianceLineFilterBase : public Framework::ScanLineFilter {
   public:
      virtual CovarianceAccumulator GetResult() = 0;
};

template< typename TPI >
class CovarianceLineFilter : public CovarianceLineFilterBase {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 10; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in1 = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI const* in2 = static_cast< TPI const* >( params.inBuffer[ 1 ].buffer );
         CovarianceAccumulator vars;
         auto bufferLength = params.bufferLength;
         auto in1Stride = params.inBuffer[ 0 ].stride;
         auto in2Stride = params.inBuffer[ 1 ].stride;
         if( params.inBuffer.size() > 2 ) {
            // If there's three input buffers, we have a mask image.
            auto maskStride = params.inBuffer[ 2 ].stride;
            bin const* mask = static_cast< bin const* >( params.inBuffer[ 2 ].buffer );
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               if( *mask ) {
                  vars.Push( static_cast< dfloat >( *in1 ), static_cast< dfloat >( *in2 ));
               }
               in1 += in1Stride;
               in2 += in2Stride;
               mask += maskStride;
            }
         } else {
            // Otherwise we don't.
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               vars.Push( static_cast< dfloat >( *in1 ), static_cast< dfloat >( *in2 ));
               in1 += in1Stride;
               in2 += in2Stride;
            }
         }
         accArray_[ params.thread ] += vars;
      }
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         accArray_.resize( threads );
      }
      virtual CovarianceAccumulator GetResult() override {
         CovarianceAccumulator out = accArray_[ 0 ];
         for( dip::uint ii = 1; ii < accArray_.size(); ++ii ) {
            out += accArray_[ ii ];
         }
         return out;
      }
   private:
      std::vector< CovarianceAccumulator > accArray_;
};

} // namespace

CovarianceAccumulator Covariance(
      Image const& in1,
      Image const& in2,
      Image const& c_mask ) {
   DIP_THROW_IF( !in1.IsForged() || !in2.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_STACK_TRACE_THIS( in1.CompareProperties( in2, Option::CmpProp::AllSizes ));
   DataType ovlDataType = DataType::SuggestDyadicOperation( in1.DataType(), in2.DataType() );
   ImageConstRefArray inar;
   inar.reserve( 3 );
   inar.push_back( in1 );
   inar.push_back( in2 );
   DataTypeArray inBufT{ ovlDataType, ovlDataType };
   Image mask;
   if( c_mask.IsForged() ) {
      // If we have a mask, add it to the input array.
      mask = c_mask.QuickCopy();
      DIP_START_STACK_TRACE
         mask.CheckIsMask( in1.Sizes(), Option::AllowSingletonExpansion::DO_ALLOW, Option::ThrowException::DO_THROW );
         mask.ExpandSingletonDimensions( in1.Sizes() );
      DIP_END_STACK_TRACE
      inar.push_back( mask );
      inBufT.push_back( mask.DataType() );
   }
   ImageRefArray outar{};
   std::unique_ptr< CovarianceLineFilterBase > scanLineFilter;
   DIP_OVL_NEW_REAL( scanLineFilter, CovarianceLineFilter, (), ovlDataType );
   DIP_STACK_TRACE_THIS( Framework::Scan( inar, outar, inBufT, {}, {}, {}, *scanLineFilter,
                                          Framework::ScanOption::TensorAsSpatialDim ));
   return scanLineFilter->GetResult();
}

namespace {

template< typename TPI >
std::vector< dip::uint > ComputeRank( void const* ptr, std::vector< dip::uint >& indices ) {
   // First sort the indices
   // NOTE!!! The indices must be contiguous, starting at 0, and with max_element(indices) == indices.size()-1.
   TPI const* data = static_cast< TPI const* >( ptr );
   std::sort( indices.begin(), indices.end(), [ & ]( dip::uint const& a, dip::uint const& b ) {
      return data[ a ] < data[ b ];
   } );
   // Next find the ranks
   std::vector< dip::uint > rank( indices.size() );
   for( dip::uint ii = 0; ii < indices.size(); ++ii ) {
      // Identify the equal-valued pixels
      dip::uint rr = ii + 1;
      while(( rr < indices.size()) && ( data[ indices[ rr ]] == data[ indices[ ii ]] )) {
         ++rr;
      }
      // Assign the mean rank to all these pixels
      dip::uint mean = ( rr + ii - 1 ) / 2;
      for( dip::uint jj = ii; jj < rr; ++jj ) {
         rank[ indices[ jj ]] = mean;
      }
      // Advance to next group of equal-valued pixels
      ii = rr - 1;
   }
   return rank;
}

std::vector< dip::uint > CreateRankArray( Image const& img ) {
   DIP_ASSERT( img.HasContiguousData() );
   // Create indices array to each sample in the image
   std::vector< dip::uint > indices( img.Sizes().product() * img.TensorElements() );
   std::iota( indices.begin(), indices.end(), dip::uint( 0 ));
   // Get the rank for each pixel
   std::vector< dip::uint > rank;
   DIP_OVL_CALL_ASSIGN_REAL( rank, ComputeRank, ( img.Origin(), indices ), img.DataType() );
   return rank;
}

} // namespace

dfloat SpearmanRankCorrelation( Image const& in1, Image const& in2, Image const& mask ) {
   DIP_THROW_IF( !in1.IsForged() || !in2.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_STACK_TRACE_THIS( in1.CompareProperties( in2, Option::CmpProp::AllSizes ));
   // Get the data in normal stride order. We need the data to be contiguous and the two images to have
   // the same strides. This is a simple way of accomplishing that.
   Image in1_c;
   Image in2_c;
   if( mask.IsForged() ) {
      DIP_START_STACK_TRACE
         in1_c = in1.At( mask );
         in2_c = in2.At( mask );
      DIP_END_STACK_TRACE
   } else {
      in1_c = in1.QuickCopy();
      in2_c = in2.QuickCopy();
   }
   in1_c.ForceNormalStrides(); // Might copy the data, but if we already copied it (through `mask`) it won't need to,
   in2_c.ForceNormalStrides(); //    so we're guaranteed to copy the image data at most once.
   // Find the rank for each pixel
   auto idx1 = CreateRankArray( in1_c );
   auto idx2 = CreateRankArray( in2_c );
   // Now compute correlation between the two sorted index arrays.
   // We're not using the cheaper formula because we're not guaranteed a unique sort order (some pixels can have
   // the same value).
   CovarianceAccumulator vars;
   for( auto it1 = idx1.begin(), it2 = idx2.begin(); it1 != idx1.end(); ++it1, ++it2 ) {
      vars.Push( static_cast< dfloat >( *it1 ), static_cast< dfloat >( *it2 ));
   }
   return vars.Correlation();
}

namespace {

class CenterOfMassLineFilterBase : public Framework::ScanLineFilter {
   public:
      virtual FloatArray GetResult() = 0;
};

template< typename TPI >
class CenterOfMassLineFilter : public CenterOfMassLineFilterBase {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return nD_ + 1; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         FloatArray vars( nD_ + 1, 0.0 );
         auto bufferLength = params.bufferLength;
         auto inStride = params.inBuffer[ 0 ].stride;
         UnsignedArray pos = params.position;
         dip::uint procDim = params.dimension;
         if( params.inBuffer.size() > 1 ) {
            // If there's two input buffers, we have a mask image.
            auto maskStride = params.inBuffer[ 1 ].stride;
            bin const* mask = static_cast< bin const* >( params.inBuffer[ 1 ].buffer );
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               if( *mask ) {
                  for( dip::uint jj = 0; jj < nD_; ++jj ) {
                     vars[ jj ] += static_cast< dfloat >( pos[ jj ] ) * static_cast< dfloat >( *in );
                  }
                  vars[ nD_ ] += static_cast< dfloat >( *in );
               }
               in += inStride;
               mask += maskStride;
               ++( pos[ procDim ] );
            }
         } else {
            // Otherwise we don't.
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               for( dip::uint jj = 0; jj < nD_; ++jj ) {
                  vars[ jj ] += static_cast< dfloat >( pos[ jj ] ) * static_cast< dfloat >( *in );
               }
               vars[ nD_ ] += static_cast< dfloat >( *in );
               in += inStride;
               ++( pos[ procDim ] );
            }
         }
         accArray_[ params.thread ] += vars;
      }
      CenterOfMassLineFilter( dip::uint nD ) : nD_( nD ) {}
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         accArray_.resize( threads );
         for( dip::uint ii = 0; ii < threads; ++ii ) {
            accArray_[ ii ].resize( nD_ + 1, 0.0 );
         }
      }
      virtual FloatArray GetResult() override {
         FloatArray out = accArray_[ 0 ];
         for( dip::uint ii = 1; ii < accArray_.size(); ++ii ) {
            out += accArray_[ ii ];
         }
         dfloat n = out[ nD_ ];
         out.resize( nD_ );
         if( n != 0 ) {
            out /= n;
         } else {
            out.fill( 0.0 );
         }
         return out;
      }
   private:
      std::vector< FloatArray > accArray_; // one per thread, each one contains: sum(I*x),sum(I*y),...,sum(I)
      dip::uint nD_;
};

} // namespace

FloatArray CenterOfMass(
      Image const& in,
      Image const& mask
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   std::unique_ptr< CenterOfMassLineFilterBase > scanLineFilter;
   DIP_OVL_NEW_NONCOMPLEX( scanLineFilter, CenterOfMassLineFilter, ( in.Dimensionality() ), in.DataType() );
   DIP_STACK_TRACE_THIS( Framework::ScanSingleInput( in, mask, in.DataType(), *scanLineFilter,
                                                     Framework::ScanOption::NeedCoordinates ));
   return scanLineFilter->GetResult();
}

namespace {

class MomentsLineFilterBase : public Framework::ScanLineFilter {
   public:
      virtual MomentAccumulator GetResult() = 0;
};

template< typename TPI >
class MomentsLineFilter : public MomentsLineFilterBase {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override {
         return nD_ * ( nD_ + 1 ) / 2 * 3 + nD_ + 2;
      }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         MomentAccumulator vars( nD_ );
         auto bufferLength = params.bufferLength;
         auto inStride = params.inBuffer[ 0 ].stride;
         FloatArray pos{ params.position };
         dip::uint procDim = params.dimension;
         if( params.inBuffer.size() > 1 ) {
            // If there's two input buffers, we have a mask image.
            auto maskStride = params.inBuffer[ 1 ].stride;
            bin const* mask = static_cast< bin const* >( params.inBuffer[ 1 ].buffer );
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               if( *mask ) {
                  vars.Push( pos, static_cast< dfloat >( *in ));
               }
               in += inStride;
               mask += maskStride;
               ++( pos[ procDim ] );
            }
         } else {
            // Otherwise we don't.
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               vars.Push( pos, static_cast< dfloat >( *in ));
               in += inStride;
               ++( pos[ procDim ] );
            }
         }
         accArray_[ params.thread ] += vars;
      }
      MomentsLineFilter( dip::uint nD ) : nD_( nD ) {}
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         accArray_.resize( threads, MomentAccumulator( nD_ ));
      }
      virtual MomentAccumulator GetResult() override {
         MomentAccumulator out = accArray_[ 0 ];
         for( dip::uint ii = 1; ii < accArray_.size(); ++ii ) {
            out += accArray_[ ii ];
         }
         return out;
      }
   private:
      std::vector< MomentAccumulator > accArray_;
      dip::uint nD_;
};

} // namespace

MomentAccumulator Moments(
      Image const& in,
      Image const& mask
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   std::unique_ptr< MomentsLineFilterBase > scanLineFilter;
   DIP_OVL_NEW_NONCOMPLEX( scanLineFilter, MomentsLineFilter, ( in.Dimensionality() ), in.DataType() );
   DIP_STACK_TRACE_THIS( Framework::ScanSingleInput( in, mask, in.DataType(), *scanLineFilter,
                                                     Framework::ScanOption::NeedCoordinates ));
   return scanLineFilter->GetResult();
}

} // namespace dip
