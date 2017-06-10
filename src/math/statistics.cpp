/*
 * DIPlib 3.0
 * This file contains the definition for the image statistics functions.
 *
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

class dip__Count : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         bin const* in = static_cast< bin const* >( params.inBuffer[ 0 ].buffer );
         dip::uint& count = counts_[ params.thread ];
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
   dip__Count scanLineFilter;
   Framework::ScanSingleInput( in, mask, DT_BIN, scanLineFilter );
   return scanLineFilter.GetResult();
}

namespace {

class dip__MaxMinPixel : public Framework::ScanLineFilter {
   public:
      virtual UnsignedArray GetResult() = 0;
};

template< typename TPI >
class dip__MaxPixel : public dip__MaxMinPixel {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         UnsignedArray& coord = coord_[ params.thread ];
         TPI& value = value_[ params.thread ];
         if( coord.empty()) {
            coord.resize( params.position.size());
            value = std::numeric_limits< TPI >::lowest();
         }
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
      }
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         coord_.resize( threads );
         value_.resize( threads, TPI( 0 ));
      }
      dip__MaxPixel( bool first ) : first_( first ) {}
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
class dip__MinPixel : public dip__MaxMinPixel {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         UnsignedArray& coord = coord_[ params.thread ];
         TPI& value = value_[ params.thread ];
         if( coord.empty()) {
            coord.resize( params.position.size());
            value = std::numeric_limits< TPI >::max();
         }
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
      }
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         coord_.resize( threads );
         value_.resize( threads, TPI( 0 ));
      }
      dip__MinPixel( bool first ) : first_( first ) {}
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
   bool first = positionFlag == "first";
   std::unique_ptr< dip__MaxMinPixel >scanLineFilter;
   DIP_OVL_NEW_REAL( scanLineFilter, dip__MaxPixel, ( first ), in.DataType() );
   Framework::ScanSingleInput( in, mask, in.DataType(), *scanLineFilter,
                               Framework::Scan_TensorAsSpatialDim + Framework::Scan_NeedCoordinates );
   return scanLineFilter->GetResult();
}

UnsignedArray MinimumPixel( Image const& in, Image const& mask, String const& positionFlag ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   bool first = positionFlag == "first";
   std::unique_ptr< dip__MaxMinPixel >scanLineFilter;
   DIP_OVL_NEW_REAL( scanLineFilter, dip__MinPixel, ( first ), in.DataType() );
   Framework::ScanSingleInput( in, mask, in.DataType(), *scanLineFilter,
                               Framework::Scan_TensorAsSpatialDim + Framework::Scan_NeedCoordinates );
   return scanLineFilter->GetResult();
}

namespace {

template< typename TPI >
class CumSumFilter : public Framework::SeparableLineFilter {
   public:
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
      BooleanArray process
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( in.Dimensionality() < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DataType dataType = DataType::SuggestFlex( in.DataType() );
   std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
   DIP_OVL_NEW_FLEX( lineFilter, CumSumFilter, (), dataType );
   if( mask.IsForged() ) {
      Select( in, Image( 0, dataType ), mask, out );
      Framework::Separable( out, out, dataType, dataType,
                            process, { 0 }, {}, *lineFilter,
                            Framework::Separable_AsScalarImage );
   }
   Framework::Separable( in, out, dataType, dataType,
                         process, { 0 }, {}, *lineFilter,
                         Framework::Separable_AsScalarImage );
}

namespace {

class dip__GetMaximumAndMinimumBase : public Framework::ScanLineFilter {
   public:
      virtual MinMaxAccumulator GetResult() = 0;
};

template< typename TPI >
class dip__GetMaximumAndMinimum : public dip__GetMaximumAndMinimumBase {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
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
                  vars.Push( *in );
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
               vars.Push( v, *in );
               in += inStride;
            }
            if( ii < bufferLength ) {
               vars.Push( *in );
            }
         }
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

MinMaxAccumulator GetMaximumAndMinimum(
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
   std::unique_ptr< dip__GetMaximumAndMinimumBase >scanLineFilter;
   DIP_OVL_NEW_NONCOMPLEX( scanLineFilter, dip__GetMaximumAndMinimum, (), c_in.DataType() );
   Framework::ScanSingleInput( c_in, mask, c_in.DataType(), *scanLineFilter, Framework::Scan_TensorAsSpatialDim );
   return scanLineFilter->GetResult();
}

namespace {

class dip__GetSampleStatisticsBase : public Framework::ScanLineFilter {
   public:
      virtual StatisticsAccumulator GetResult() = 0;
};

template< typename TPI >
class dip__GetSampleStatistics : public dip__GetSampleStatisticsBase {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
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
                  vars.Push( *in );
               }
               in += inStride;
               mask += maskStride;
            }
         } else {
            // Otherwise we don't.
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               vars.Push( *in );
               in += inStride;
            }
         }
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

StatisticsAccumulator GetSampleStatistics(
      Image const& in,
      Image const& mask
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   std::unique_ptr< dip__GetSampleStatisticsBase >scanLineFilter;
   DIP_OVL_NEW_NONCOMPLEX( scanLineFilter, dip__GetSampleStatistics, (), in.DataType() );
   Framework::ScanSingleInput( in, mask, in.DataType(), *scanLineFilter, Framework::Scan_TensorAsSpatialDim );
   return scanLineFilter->GetResult();
}


} // namespace dip
