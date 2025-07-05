/*
 * (c)2016-2025, Cris Luengo.
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

#include "diplib/statistics.h"

#include <memory>
#include <limits>
#include <numeric>
#include <vector>

#include "diplib.h"
#include "diplib/accumulators.h"
#include "diplib/framework.h"
#include "diplib/iterators.h"
#include "diplib/math.h"
#include "diplib/overload.h"

#include "copy_non_nan.h"

namespace dip {

namespace {

class CountLineFilter : public Framework::ScanLineFilter {
   public:
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override { return 2; }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
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
      void SetNumberOfThreads( dip::uint threads ) override {
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

template< typename TPI >
bool ContainsValue( Image const& in, Image const& mask, bool f( TPI ) ) {
   if( mask.IsForged() ) {
      JointImageIterator< TPI, bin > it( { in, mask } );
      it.OptimizeAndFlatten();
      do {
         if( it.template Sample< 1 >() ) {
            if( f( it.In() )) {
               return true;
            }
         }
      } while( ++it );
   } else {
      ImageIterator< TPI > it( in );
      it.OptimizeAndFlatten();
      do {
         if( f( *it )) {
            return true;
         }
      } while( ++it );
   }
   return false;
}

template< typename TPI >
bool ContainsNaN( Image const& in, Image const& mask ) {
   return ContainsValue< TPI >( in, mask, std::isnan );
}

template< typename TPI >
bool ContainsInf( Image const& in, Image const& mask ) {
   return ContainsValue< TPI >( in, mask, std::isinf );
}

template< typename TPI >
bool ContainsNonFinite( Image const& in, Image const& mask ) {
   return ContainsValue< TPI >( in, mask, []( TPI v ){ return !std::isfinite( v ); } );
}

void PrepareImageAndMask( Image& in, Image& mask ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   if( in.TensorElements() > 1 ) {
      in.TensorToSpatial();
   }
   if( in.DataType().IsComplex() ) {
      in.SplitComplex();
   }
   if( mask.IsForged() ) {
      DIP_STACK_TRACE_THIS( mask.CheckIsMask( in.Sizes(), Option::AllowSingletonExpansion::DO_ALLOW ));
      mask.ExpandSingletonDimensions( in.Sizes() );
   }
}

} // namespace

bool ContainsNotANumber( Image const& c_in, Image const& c_mask ) {
   Image in = c_in.QuickCopy();
   Image mask = c_mask.QuickCopy();
   DIP_STACK_TRACE_THIS( PrepareImageAndMask( in, mask ));
   if( !in.DataType().IsFloat() ) {
      return false;
   }
   bool result = false;
   DIP_OVL_CALL_ASSIGN_FLOAT( result, ContainsNaN, ( in, mask ), in.DataType() );
   return result;
}

bool ContainsInfinity( Image const& c_in, Image const& c_mask ) {
   Image in = c_in.QuickCopy();
   Image mask = c_mask.QuickCopy();
   DIP_STACK_TRACE_THIS( PrepareImageAndMask( in, mask ));
   if( !in.DataType().IsFloat() ) {
      return false;
   }
   bool result = false;
   DIP_OVL_CALL_ASSIGN_FLOAT( result, ContainsInf, ( in, mask ), in.DataType() );
   return result;
}

bool ContainsNonFiniteValue( Image const& c_in, Image const& c_mask ) {
   Image in = c_in.QuickCopy();
   Image mask = c_mask.QuickCopy();
   DIP_STACK_TRACE_THIS( PrepareImageAndMask( in, mask ));
   if( !in.DataType().IsFloat() ) {
      return false;
   }
   bool result = false;
   DIP_OVL_CALL_ASSIGN_FLOAT( result, ContainsNonFinite, ( in, mask ), in.DataType() );
   return result;
}

namespace {

class MaxMinPixelLineFilter : public Framework::ScanLineFilter {
   public:
      virtual UnsignedArray GetResult() = 0;
};

template< typename TPI >
class MaxPixelLineFilter : public MaxMinPixelLineFilter {
   public:
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override { return 2; }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
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
      void SetNumberOfThreads( dip::uint threads ) override {
         coord_.resize( threads );
         value_.resize( threads, std::numeric_limits< TPI >::lowest() );
      }
      MaxPixelLineFilter( bool first ) : first_( first ) {}
      UnsignedArray GetResult() override {
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
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override { return 2; }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
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
      void SetNumberOfThreads( dip::uint threads ) override {
         coord_.resize( threads );
         value_.resize( threads, std::numeric_limits< TPI >::max() );
      }
      MinPixelLineFilter( bool first ) : first_( first ) {}
      UnsignedArray GetResult() override {
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
   bool first{};
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
   bool first{};
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
      dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override {
         return lineLength;
      }
      void Filter( Framework::SeparableLineFilterParameters const& params ) override {
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
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override { return 3; }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         MinMaxAccumulator& vars = accArray_[ params.thread ];
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
      }
      void SetNumberOfThreads( dip::uint threads ) override {
         accArray_.resize( threads );
      }
      MinMaxAccumulator GetResult() override {
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

template< typename TPI >
QuartilesResult QuartilesInternal( Image const& in, Image const& mask ) {
   std::vector< TPI > buffer;
   CopyNonNaNValues( in, mask, buffer );
   TPI* begin = buffer.data();
   TPI* end = begin + buffer.size();
   TPI* lower = begin + RankFromPercentile( 25.0, buffer.size() );
   TPI* median = begin + RankFromPercentile( 50.0, buffer.size() );
   TPI* upper = begin + RankFromPercentile( 75.0, buffer.size() );
   std::nth_element( begin, median, end );
   if( begin >= median - 1 ) {
      lower = begin;
   } else {
      std::nth_element( begin, lower, median );
   }
   if( median >= end - 1 ) {
      upper = median;
   } else {
      std::nth_element( median, upper, end );
   }
   return {
      static_cast< dfloat >( *std::min_element( begin, lower )),
      static_cast< dfloat >( *lower ),
      static_cast< dfloat >( *median ),
      static_cast< dfloat >( *upper ),
      static_cast< dfloat >( *std::max_element( upper, end )),
   };
}

} // namespace

QuartilesResult Quartiles( Image const& c_in, Image const& c_mask ) {
   Image in = c_in.QuickCopy();
   Image mask = c_mask.QuickCopy();
   DIP_STACK_TRACE_THIS( PrepareImageAndMask( in, mask ));
   QuartilesResult quartiles{};
   DIP_OVL_CALL_ASSIGN_NONCOMPLEX( quartiles, QuartilesInternal, ( in, mask ), in.DataType() );
   return quartiles;
}

namespace {

class SampleStatisticsLineFilterBase : public Framework::ScanLineFilter {
   public:
      virtual StatisticsAccumulator GetResult() = 0;
};

template< typename TPI >
class SampleStatisticsLineFilter : public SampleStatisticsLineFilterBase {
   public:
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override { return 23; }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         StatisticsAccumulator& vars = accArray_[ params.thread ];
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
      }
      void SetNumberOfThreads( dip::uint threads ) override {
         accArray_.resize( threads );
      }
      StatisticsAccumulator GetResult() override {
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
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override { return 10; }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in1 = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI const* in2 = static_cast< TPI const* >( params.inBuffer[ 1 ].buffer );
         CovarianceAccumulator& vars = accArray_[ params.thread ];
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
      }
      void SetNumberOfThreads( dip::uint threads ) override {
         accArray_.resize( threads );
      }
      CovarianceAccumulator GetResult() override {
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
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override { return nD_ + 1; }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         FloatArray& vars = accArray_[ params.thread ];
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
      }
      CenterOfMassLineFilter( dip::uint nD ) : nD_( nD ) {}
      void SetNumberOfThreads( dip::uint threads ) override {
         accArray_.resize( threads );
         for( dip::uint ii = 0; ii < threads; ++ii ) {
            accArray_[ ii ].resize( nD_ + 1, 0.0 );
         }
      }
      FloatArray GetResult() override {
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
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override {
         return nD_ * ( nD_ + 1 ) / 2 * 3 + nD_ + 2;
      }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         MomentAccumulator& vars = accArray_[ params.thread ];
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
      }
      MomentsLineFilter( dip::uint nD ) : nD_( nD ) {}
      void SetNumberOfThreads( dip::uint threads ) override {
         accArray_.resize( threads, MomentAccumulator( nD_ ));
      }
      MomentAccumulator GetResult() override {
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
