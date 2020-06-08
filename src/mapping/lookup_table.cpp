/*
 * DIPlib 3.0
 * This file contains definitions for look-up tables and related functionality
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
#include "diplib/lookup_table.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {

namespace {

template< typename TPI >
inline void FillPixel( TPI* out, dip::uint length, dip::sint stride, TPI value ) {
   for( dip::uint ii = 0; ii < length; ++ii ) {
      *out = value;
      out += stride;
   }
}

template< typename TPI >
inline void CopyPixel( TPI const* in, TPI* out, dip::uint length, dip::sint inStride, dip::sint outStride ) {
   for( dip::uint ii = 0; ii < length; ++ii ) {
      *out = *in;
      in += inStride;
      out += outStride;
   }
}

template< typename TPI >
inline void CopyPixelWithInterpolation( TPI const* in, TPI* out,
                                        dip::uint length, dip::sint inStride, dip::sint outStride,
                                        dfloat fraction, dip::sint interpStride ) {
   for( dip::uint ii = 0; ii < length; ++ii ) {
      *out = static_cast< TPI >( static_cast< dfloat >( *in ) * ( 1 - fraction ) + static_cast< dfloat >( *( in + interpStride )) * fraction );
      in += inStride;
      out += outStride;
   }
}

template< typename TPI >
inline void CopyPixelWithInterpolation( std::complex< TPI > const* in, std::complex< TPI >* out,
                                        dip::uint length, dip::sint inStride, dip::sint outStride,
                                        dfloat fraction, dip::sint interpStride ) {
   for( dip::uint ii = 0; ii < length; ++ii ) {
      *out = *in * static_cast< TPI >( 1 - fraction ) + *( in + interpStride ) * static_cast< TPI >( fraction );
      in += inStride;
      out += outStride;
   }
}

template< typename TPI >
class DirectLUT_Integer : public Framework::ScanLineFilter {
      // Applies the LUT with data type TPI, and no index, to an input image of type uint64.
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 3; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         uint64 const* in = static_cast< uint64 const* >( params.inBuffer[ 0 ].buffer );
         auto bufferLength = params.bufferLength;
         auto inStride = params.inBuffer[ 0 ].stride;
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         auto outStride = params.outBuffer[ 0 ].stride;
         auto tensorLength = params.outBuffer[ 0 ].tensorLength;
         auto outTensorStride = params.outBuffer[ 0 ].tensorStride;
         TPI const* values = static_cast< TPI const* >( values_.Origin() );
         auto valuesStride = values_.Stride( 0 );
         auto valuesTensorStride = values_.TensorStride();
         DIP_ASSERT( values_.DataType() == DataType( TPI( 0 )));
         DIP_ASSERT( values_.TensorElements() == tensorLength );
         dip::uint maxIndex = values_.Size( 0 ) - 1;
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            dip::uint index = static_cast< dip::uint >( *in );
            if( index > maxIndex ) {
               switch( outOfBoundsMode_ ) {
                  case LookupTable::OutOfBoundsMode::USE_OUT_OF_BOUNDS_VALUE:
                     FillPixel( out, tensorLength, outTensorStride, outOfBoundsUpperValue_ );
                     break;
                  case LookupTable::OutOfBoundsMode::KEEP_INPUT_VALUE:
                     FillPixel( out, tensorLength, outTensorStride, clamp_cast< TPI >( index ) );
                     break;
                  //case LookupTable::OutOfBoundsMode::CLAMP_TO_RANGE:
                  default:
                     CopyPixel( values + static_cast< dip::sint >( maxIndex ) * valuesStride, out, tensorLength, valuesTensorStride, outTensorStride );
                     break;
               }
            } else {
               CopyPixel( values + static_cast< dip::sint >( index ) * valuesStride, out, tensorLength, valuesTensorStride, outTensorStride );
            }
            in += inStride;
            out += outStride;
         }
      }
      DirectLUT_Integer( Image const& values, LookupTable::OutOfBoundsMode outOfBoundsMode, dfloat outOfBoundsLowerValue, dfloat outOfBoundsUpperValue )
            : values_( values ), outOfBoundsMode_( outOfBoundsMode ), outOfBoundsLowerValue_( clamp_cast< TPI >( outOfBoundsLowerValue )),
              outOfBoundsUpperValue_( clamp_cast< TPI >( outOfBoundsUpperValue )) {}
   private:
      Image const& values_;
      LookupTable::OutOfBoundsMode outOfBoundsMode_;
      TPI outOfBoundsLowerValue_;
      TPI outOfBoundsUpperValue_;
};

template< typename TPI >
class DirectLUT_Float : public Framework::ScanLineFilter {
      // Applies the LUT with data type TPI, and no index, to an input image of type dfloat.
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override {
         return interpolation_ == LookupTable::InterpolationMode::LINEAR ? 9 : 3;
      }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat const* in = static_cast< dfloat const* >( params.inBuffer[ 0 ].buffer );
         auto bufferLength = params.bufferLength;
         auto inStride = params.inBuffer[ 0 ].stride;
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         auto outStride = params.outBuffer[ 0 ].stride;
         auto tensorLength = params.outBuffer[ 0 ].tensorLength;
         auto outTensorStride = params.outBuffer[ 0 ].tensorStride;
         TPI const* values = static_cast< TPI const* >( values_.Origin() );
         auto valuesStride = values_.Stride( 0 );
         auto valuesTensorStride = values_.TensorStride();
         DIP_ASSERT( values_.DataType() == DataType( TPI( 0 )));
         DIP_ASSERT( values_.TensorElements() == tensorLength );
         dip::uint maxIndex = values_.Size( 0 ) - 1;
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            if(( *in < 0 ) || ( *in > static_cast< dfloat >( maxIndex ))) {
               switch( outOfBoundsMode_ ) {
                  case LookupTable::OutOfBoundsMode::USE_OUT_OF_BOUNDS_VALUE:
                     FillPixel( out, tensorLength, outTensorStride, *in < 0 ? outOfBoundsLowerValue_ : outOfBoundsUpperValue_ );
                     break;
                  case LookupTable::OutOfBoundsMode::KEEP_INPUT_VALUE:
                     FillPixel( out, tensorLength, outTensorStride, clamp_cast< TPI >( *in ) );
                     break;
                  //case LookupTable::OutOfBoundsMode::CLAMP_TO_RANGE:
                  default:
                     TPI const* tval = values + ( *in < 0.0 ? 0 : static_cast< dip::sint >( maxIndex )) * valuesStride;
                     CopyPixel( tval, out, tensorLength, valuesTensorStride, outTensorStride );
                     break;
               }
            } else {
               switch( interpolation_ ) {
                  case LookupTable::InterpolationMode::LINEAR: {
                     dip::uint index = static_cast< dip::uint >( *in );
                     dfloat fraction = *in - static_cast< dfloat >( index );
                     if( fraction == 0.0 ) {
                        // not just to avoid the extra computation, it especially avoids out-of-bounds indexing if in points at the last LUT element.
                        CopyPixel( values + static_cast< dip::sint >( index ) * valuesStride, out, tensorLength,
                                   valuesTensorStride, outTensorStride );
                     } else {
                        CopyPixelWithInterpolation( values + static_cast< dip::sint >( index ) * valuesStride,
                                                    out, tensorLength, valuesTensorStride, outTensorStride,
                                                    fraction, valuesStride );
                     }
                     break;
                  }
                  case LookupTable::InterpolationMode::NEAREST_NEIGHBOR: {
                     dip::uint index = static_cast< dip::uint >( *in + 0.5 );
                     CopyPixel( values + static_cast< dip::sint >( index ) * valuesStride, out, tensorLength,
                                valuesTensorStride, outTensorStride );
                     break;
                  }
                  case LookupTable::InterpolationMode::ZERO_ORDER_HOLD: {
                     dip::uint index = static_cast< dip::uint >( *in );
                     CopyPixel( values + static_cast< dip::sint >( index ) * valuesStride, out, tensorLength,
                                valuesTensorStride, outTensorStride );
                     break;
                  }
               }
            }
            in += inStride;
            out += outStride;
         }
      }
      DirectLUT_Float( Image const& values, LookupTable::OutOfBoundsMode outOfBoundsMode, dfloat outOfBoundsLowerValue,
                       dfloat outOfBoundsUpperValue, LookupTable::InterpolationMode interpolation )
            : values_( values ), outOfBoundsMode_( outOfBoundsMode ), outOfBoundsLowerValue_( clamp_cast< TPI >( outOfBoundsLowerValue )),
              outOfBoundsUpperValue_( clamp_cast< TPI >( outOfBoundsUpperValue )), interpolation_( interpolation ) {}
   private:
      Image const& values_;
      LookupTable::OutOfBoundsMode outOfBoundsMode_;
      TPI outOfBoundsLowerValue_;
      TPI outOfBoundsUpperValue_;
      LookupTable::InterpolationMode interpolation_;
};

template< typename TPI >
class IndexedLUT_Float : public Framework::ScanLineFilter {
      // Applies the LUT with data type TPI, and an index, to an input image of type dfloat.
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override {
         return interpolation_ == LookupTable::InterpolationMode::LINEAR ? 9 : 3;
      }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat const* in = static_cast< dfloat const* >( params.inBuffer[ 0 ].buffer );
         auto bufferLength = params.bufferLength;
         auto inStride = params.inBuffer[ 0 ].stride;
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         auto outStride = params.outBuffer[ 0 ].stride;
         auto tensorLength = params.outBuffer[ 0 ].tensorLength;
         auto outTensorStride = params.outBuffer[ 0 ].tensorStride;
         TPI const* values = static_cast< TPI const* >( values_.Origin() );
         auto valuesStride = values_.Stride( 0 );
         auto valuesTensorStride = values_.TensorStride();
         DIP_ASSERT( values_.DataType() == DataType( TPI( 0 )));
         DIP_ASSERT( values_.TensorElements() == tensorLength );
         dip::uint maxIndex = values_.Size( 0 ) - 1;
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            if(( *in < index_.front() ) || ( *in > index_.back() )) {
               switch( outOfBoundsMode_ ) {
                  case LookupTable::OutOfBoundsMode::USE_OUT_OF_BOUNDS_VALUE:
                     FillPixel( out, tensorLength, outTensorStride, *in < index_.front() ? outOfBoundsLowerValue_ : outOfBoundsUpperValue_ );
                     break;
                  case LookupTable::OutOfBoundsMode::KEEP_INPUT_VALUE:
                     FillPixel( out, tensorLength, outTensorStride, clamp_cast< TPI >( *in ) );
                     break;
                  //case LookupTable::OutOfBoundsMode::CLAMP_TO_RANGE:
                  default:
                     TPI const* tval = values + ( *in < index_.front() ? 0 : static_cast< dip::sint >( maxIndex )) * valuesStride;
                     CopyPixel( tval, out, tensorLength, valuesTensorStride, outTensorStride );
                     break;
               }
            } else {
               auto upper = std::upper_bound( index_.begin(), index_.end(), *in ); // index_[upper] > *in
               dip::uint index = static_cast< dip::uint >( std::distance( index_.begin(), upper )) - 1; // index_[index] <= *in
               // Because *in >= index_.front(), we can always subtract 1 from the distance.
               switch( interpolation_ ) {
                  case LookupTable::InterpolationMode::LINEAR:
                     if( *in == index_[ index ] ) {
                        CopyPixel( values + static_cast< dip::sint >( index ) * valuesStride, out, tensorLength,
                                   valuesTensorStride, outTensorStride );
                     } else {
                        dfloat fraction = ( *in - index_[ index ] ) / ( index_[ index + 1 ] - index_[ index ] );
                        CopyPixelWithInterpolation( values + static_cast< dip::sint >( index ) * valuesStride,
                                                    out, tensorLength, valuesTensorStride, outTensorStride,
                                                    fraction, valuesStride );
                     }
                     break;
                  case LookupTable::InterpolationMode::NEAREST_NEIGHBOR:
                     if(( *in != index_[ index ] ) && (( *in - index_[ index ] ) > ( index_[ index + 1 ] - *in ))) {
                        // (the `!=` test above is to avoid out-of-bounds indexing with `index+1`)
                        ++index;
                     }
                     // fallthrough
                  case LookupTable::InterpolationMode::ZERO_ORDER_HOLD:
                     CopyPixel( values + static_cast< dip::sint >( index ) * valuesStride, out, tensorLength,
                                valuesTensorStride, outTensorStride );
                     break;
               }
            }
            in += inStride;
            out += outStride;
         }
      }
      IndexedLUT_Float( Image const& values, FloatArray const& index, LookupTable::OutOfBoundsMode outOfBoundsMode,
                        dfloat outOfBoundsLowerValue, dfloat outOfBoundsUpperValue, LookupTable::InterpolationMode interpolation )
            : values_( values ), index_( index ), outOfBoundsMode_( outOfBoundsMode ), outOfBoundsLowerValue_( clamp_cast< TPI >( outOfBoundsLowerValue )),
              outOfBoundsUpperValue_( clamp_cast< TPI >( outOfBoundsUpperValue )), interpolation_( interpolation ) {}
   private:
      Image const& values_;
      FloatArray const& index_;
      LookupTable::OutOfBoundsMode outOfBoundsMode_;
      TPI outOfBoundsLowerValue_;
      TPI outOfBoundsUpperValue_;
      LookupTable::InterpolationMode interpolation_;
};

template< typename TPI >
class IndexedArrayLUT_Float : public Framework::ScanLineFilter {
      // Applies the LUT consisting of an array of value images with data type TPI, and an index, to an input image of type dfloat.
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override {
         return interpolation_ == LookupTable::InterpolationMode::LINEAR ? 9 : 3;
      }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         // inBuffer[0] is the input image; the remaining inBuffer elements contain the value images
         Framework::ScanBuffer const& inBuffer = params.inBuffer[ 0 ];
         Framework::ScanBuffer const& firstValueBuffer = params.inBuffer[ 1 ];
         dfloat const* in = static_cast< dfloat const* >( inBuffer.buffer );
         std::vector< TPI > values( 2 * inBuffer.tensorLength ); // Contains two pixels
         dip::uint numValueImages = params.inBuffer.size() - 1;
         auto bufferLength = params.bufferLength;
         auto inStride = inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         auto outStride = params.outBuffer[ 0 ].stride;
         auto tensorLength = params.outBuffer[ 0 ].tensorLength;
         auto outTensorStride = params.outBuffer[ 0 ].tensorStride;
         auto valuesStride = firstValueBuffer.stride;
         dip::sint valueImageOffset = 0;
         auto valuesTensorStride = firstValueBuffer.tensorStride;
         DIP_ASSERT( firstValueBuffer.tensorLength == tensorLength );
         dip::uint maxIndex = numValueImages - 1;
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            if(( *in < index_.front() ) || ( *in > index_.back() )) {
               switch( outOfBoundsMode_ ) {
                  case LookupTable::OutOfBoundsMode::USE_OUT_OF_BOUNDS_VALUE:
                     FillPixel( out, tensorLength, outTensorStride, *in < index_.front() ? outOfBoundsLowerValue_ : outOfBoundsUpperValue_ );
                     break;
                  case LookupTable::OutOfBoundsMode::KEEP_INPUT_VALUE:
                     FillPixel( out, tensorLength, outTensorStride, clamp_cast< TPI >( *in ));
                     break;
                  //case LookupTable::OutOfBoundsMode::CLAMP_TO_RANGE:
                  default:
                     dip::uint index = ( *in < index_.front() ? 0 : maxIndex );
                     FetchValues( params.inBuffer, values, index, valueImageOffset, tensorLength, valuesTensorStride );
                     CopyPixel( values.data(), out, tensorLength, valuesTensorStride, outTensorStride );
                     break;
               }
            } else {
               auto upper = std::upper_bound( index_.begin(), index_.end(), *in ); // index_[upper] > *in
               dip::uint index = static_cast< dip::uint >(std::distance( index_.begin(), upper )) - 1; // index_[index] <= *in
               // Because *in >= index_.front(), we can always subtract 1 from the distance.
               switch( interpolation_ ) {
                  case LookupTable::InterpolationMode::LINEAR:
                     if( *in == index_[ index ] ) {
                        FetchValues( params.inBuffer, values, index, valueImageOffset, tensorLength, valuesTensorStride );
                        CopyPixel( values.data(), out, tensorLength, valuesTensorStride, outTensorStride );
                     } else {
                        dfloat fraction = ( *in - index_[ index ] ) / ( index_[ index + 1 ] - index_[ index ] );
                        FetchValuesForInterpolation( params.inBuffer, values, index, valueImageOffset, tensorLength, valuesTensorStride );
                        CopyPixelWithInterpolation( values.data(), out, tensorLength, valuesTensorStride,
                                                    outTensorStride, fraction, valuesStride );
                     }
                     break;
                  case LookupTable::InterpolationMode::NEAREST_NEIGHBOR:
                     if(( *in != index_[ index ] ) && (( *in - index_[ index ] ) > ( index_[ index + 1 ] - *in ))) {
                        // (the `!=` test above is to avoid out-of-bounds indexing with `index+1`)
                        ++index;
                     }
                     // fallthrough
                  case LookupTable::InterpolationMode::ZERO_ORDER_HOLD:
                     FetchValues( params.inBuffer, values, index, valueImageOffset, tensorLength, valuesTensorStride );
                     CopyPixel( values.data(), out, tensorLength, valuesTensorStride, outTensorStride );
                     break;
               }
            }
            in += inStride;
            out += outStride;
            valueImageOffset += valuesStride;
         }
      }
      IndexedArrayLUT_Float(
            FloatArray const& index, LookupTable::OutOfBoundsMode outOfBoundsMode,
            dfloat outOfBoundsLowerValue, dfloat outOfBoundsUpperValue, LookupTable::InterpolationMode interpolation
      ) : index_( index ), outOfBoundsMode_( outOfBoundsMode ), outOfBoundsLowerValue_( clamp_cast< TPI >( outOfBoundsLowerValue )),
          outOfBoundsUpperValue_( clamp_cast< TPI >( outOfBoundsUpperValue )), interpolation_( interpolation ) {}
   private:
      FloatArray const& index_;
      LookupTable::OutOfBoundsMode outOfBoundsMode_;
      TPI outOfBoundsLowerValue_;
      TPI outOfBoundsUpperValue_;
      LookupTable::InterpolationMode interpolation_;

      void FetchValues( std::vector< Framework::ScanBuffer > const& valueImages, std::vector< TPI >& values,
            dip::uint valueImageIndex, dip::sint valueImageOffset, dip::uint tensorLength, dip::sint tensorStride, dip::uint localValueIndex = 0 ) {
         dip::uint scanBufferIndex = valueImageIndex + 1;   // The first image is the in-image; all others are value images
         TPI const* valuePtr = static_cast< TPI const* >( valueImages[ scanBufferIndex ].buffer ) + valueImageOffset;
         dip::uint valIndex = localValueIndex * tensorLength;
         for( dip::uint iT = 0; iT < tensorLength; ++iT ) {
            values[ valIndex ] = *valuePtr;
            ++valIndex;
            valuePtr += tensorStride;
         }
      }
      void FetchValuesForInterpolation( std::vector< Framework::ScanBuffer > const& valueImages, std::vector< TPI >& values,
            dip::uint valueImageIndex, dip::sint valueImageOffset, dip::uint tensorLength, dip::sint tensorStride ) {
         FetchValues( valueImages, values, valueImageIndex, valueImageOffset, tensorLength, tensorStride, 0 );
         FetchValues( valueImages, values, valueImageIndex + 1, valueImageOffset, tensorLength, tensorStride, 1 );
      }
};

}

void LookupTable::Apply( Image const& in, Image& out, InterpolationMode interpolation ) const {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   if( valueImages_.empty() ) {

      std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
      dip::DataType inBufType;
      if( HasIndex() ) {
         DIP_OVL_NEW_ALL( scanLineFilter, IndexedLUT_Float, (values_, index_, outOfBoundsMode_, outOfBoundsLowerValue_, outOfBoundsUpperValue_, interpolation), values_.DataType() );
         inBufType = DT_DFLOAT;
      } else {
         if( in.DataType().IsUnsigned() ) {
            DIP_OVL_NEW_ALL( scanLineFilter, DirectLUT_Integer, (values_, outOfBoundsMode_, outOfBoundsLowerValue_, outOfBoundsUpperValue_), values_.DataType() );
            inBufType = DT_UINT64;
         } else {
            DIP_OVL_NEW_ALL( scanLineFilter, DirectLUT_Float, (values_, outOfBoundsMode_, outOfBoundsLowerValue_, outOfBoundsUpperValue_, interpolation), values_.DataType() );
            inBufType = DT_DFLOAT;
         }
      }
      ImageRefArray outar{ out };
      DIP_STACK_TRACE_THIS( Scan( { in }, outar, { inBufType }, { values_.DataType() }, { values_.DataType() }, { values_.TensorElements() }, *scanLineFilter ) );

   } else {

      // Input images: [ in, valueImages_... ]
      dip::DataType valuesDataType = valueImages_.front().get().DataType();
      ImageConstRefArray inRefs = valueImages_;
      inRefs.insert( inRefs.begin(), in );
      // Input buffer data types: [ DFLOAT, valsDataType... ]
      // The input buffer type for `in` is chosen as DFLOAT so that these values can be easily looked up in the bins array
      DataTypeArray inBufferTypes( valueImages_.size(), valuesDataType );
      inBufferTypes.insert( 0, DT_DFLOAT );
      //inBufferTypes.insert( 0, in.DataType() );

      // Obtain output data type that can hold interpolated values between vals
      dip::DataType outDataType = DataType::SuggestFlex( valuesDataType );
      DataTypeArray outBufferTypes( { outDataType } );
      ImageRefArray outRefs{ out };

      // Call Scan framework
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      DIP_OVL_NEW_ALL( scanLineFilter, IndexedArrayLUT_Float, ( index_, outOfBoundsMode_, outOfBoundsLowerValue_, outOfBoundsUpperValue_, interpolation ), outDataType );
      DIP_STACK_TRACE_THIS( Scan( inRefs, outRefs, inBufferTypes, outBufferTypes, { outDataType }, { valueImages_.front().get().TensorElements() }, *scanLineFilter, Framework::ScanOption::TensorAsSpatialDim ));

   }
   out.ReshapeTensor( values_.Tensor() );
   out.SetColorSpace( values_.ColorSpace() );
}

Image::Pixel LookupTable::Apply( dfloat value, InterpolationMode interpolation ) const {
   DIP_ASSERT( valueImages_.empty() );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   if( HasIndex() ) {
      DIP_OVL_NEW_ALL( scanLineFilter, IndexedLUT_Float, ( values_, index_, outOfBoundsMode_, outOfBoundsLowerValue_, outOfBoundsUpperValue_, interpolation ), values_.DataType() );
   } else {
      DIP_OVL_NEW_ALL( scanLineFilter, DirectLUT_Float, ( values_, outOfBoundsMode_, outOfBoundsLowerValue_, outOfBoundsUpperValue_, interpolation ), values_.DataType() );
   }
   Image::Pixel out( values_.DataType(), values_.TensorElements() );
   out.ReshapeTensor( values_.Tensor() );
   std::vector< Framework::ScanBuffer > inBuffers( 1 );
   inBuffers[ 0 ] = { &value, 1, 1, 1 };
   std::vector< Framework::ScanBuffer > outBuffers( 1 );
   outBuffers[ 0 ] = { out.Origin(), 1, out.TensorStride(), out.TensorElements() };
   Framework::ScanLineFilterParameters params{ inBuffers, outBuffers, 1 /* bufferLength = 1 pixel */, 0, {}, false, 0 };
   scanLineFilter->Filter( params );
   return out;
}

} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/iterators.h"

DOCTEST_TEST_CASE( "[DIPlib] testing dip::LookupTable" ) {
   // LUT without index
   dip::Image lutIm( { 10 }, 3, dip::DT_SFLOAT );
   dip::ImageIterator< dip::sfloat > lutIt( lutIm );
   dip::sfloat v = 10;
   do {
      *lutIt = v;
      ++v;
   } while( ++lutIt );
   dip::LookupTable lut1( lutIm );
   lut1.SetOutOfBoundsValue( 255 );
   DOCTEST_CHECK( !lut1.HasIndex() );
   DOCTEST_CHECK( lut1.DataType() == dip::DT_SFLOAT );

   // Case 1: uint image
   dip::Image img1( { 5, 3 }, 1, dip::DT_UINT16 );
   dip::ImageIterator< dip::uint16 > imgIt1( img1 );
   dip::uint16 ii = 0;
   do {
      *imgIt1 = ii;
      ++ii;
   } while( ++imgIt1 );
   dip::Image out1 = lut1.Apply( img1 );
   DOCTEST_REQUIRE( out1.DataType() == dip::DT_SFLOAT );
   DOCTEST_REQUIRE( out1.TensorElements() == 3 );
   DOCTEST_REQUIRE( out1.Sizes() == img1.Sizes() );
   dip::ImageIterator< dip::sfloat > outIt1( out1 );
   ii = 0;
   do {
      if( ii <= 9 ) {
         DOCTEST_CHECK( *outIt1 == static_cast< dip::sfloat >( ii + 10 ));
      } else {
         DOCTEST_CHECK( *outIt1 == 255.0f );
      }
      ++ii;
   } while( ++outIt1 );

   // Case 2: float image
   dip::Image img2( { 5, 3 }, 1, dip::DT_DFLOAT );
   dip::ImageIterator< dip::dfloat > imgIt2( img2 );
   dip::dfloat d = 2.3;
   do {
      *imgIt2 = d;
      d += 0.8;
   } while( ++imgIt2 );
   dip::Image out2 = lut1.Apply( img2 );
   DOCTEST_REQUIRE( out2.DataType() == dip::DT_SFLOAT );
   DOCTEST_REQUIRE( out2.TensorElements() == 3 );
   DOCTEST_REQUIRE( out2.Sizes() == img2.Sizes() );
   dip::ImageIterator< dip::sfloat > outIt2( out2 );
   d = 2.3;
   do {
      if( d <= 9.0 ) {
         DOCTEST_CHECK( *outIt2 == static_cast< dip::sfloat >( d + 10.0 ));
      } else {
         DOCTEST_CHECK( *outIt2 == 255.0f );
      }
      d += 0.8;
   } while( ++outIt2 );

   // LUT with index
   dip::FloatArray index( lutIm.Size( 0 ), 0 );
   d = 8.0;
   for( auto& ind : index ) {
      ind = d;
      d += 0.5;
   }
   dip::SampleIterator< dip::sfloat > lutImIt( static_cast< dip::sfloat* >( lutIm.Origin() ), lutIm.Stride( 0 ));
   dip::LookupTable lut2( lutImIt, lutImIt + lutIm.Size( 0 ), index );
   lut2.SetOutOfBoundsValue( 255 );
   DOCTEST_CHECK( lut2.HasIndex() );
   DOCTEST_CHECK( lut2.DataType() == dip::DT_SFLOAT );

   // Case 3: float image with index
   dip::Image out3 = lut2.Apply( img2 );
   DOCTEST_REQUIRE( out3.DataType() == dip::DT_SFLOAT );
   DOCTEST_REQUIRE( out3.TensorElements() == 1 );
   DOCTEST_REQUIRE( out3.Sizes() == img2.Sizes() );
   dip::ImageIterator< dip::sfloat > outIt3( out3 );
   d = 2.3;
   do {
      if(( d >= 8.0 ) && ( d <= 12.5 )) {
         DOCTEST_CHECK( *outIt3 == static_cast< dip::sfloat >(( d - 8.0 ) * 2.0 + 10.0 ));
      } else {
         DOCTEST_CHECK( *outIt3 == 255.0f );
      }
      d += 0.8;
   } while( ++outIt3 );
}

#endif // DIP__ENABLE_DOCTEST
