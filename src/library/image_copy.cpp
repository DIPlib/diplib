/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2017, Cris Luengo.
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

#include <cstring> // std::memcpy

#include "diplib.h"
#include "diplib/statistics.h"
#include "diplib/framework.h"
#include "diplib/iterators.h"
#include "diplib/generic_iterators.h"
#include "diplib/overload.h"
#include "diplib/library/copy_buffer.h"

namespace dip {

namespace {

template< typename T >
void WriteSamples( dfloat const* src, void* destination, dip::uint n ) {
   T* dest = static_cast< T* >( destination );  // dest stride is always 1 (we created the pixel like that)
   dfloat const* end = src + n;                 // src stride is always 1 (it's an array)
   for( ; src != end; ++src, ++dest ) {
      *dest = clamp_cast< T >( *src );
   }
}

template< typename T >
void ReadSamples( void const* source, dfloat* dest, dip::uint n, dip::sint stride ) {
   T const* src = static_cast< T const* >( source );
   dfloat* end = dest + n;                            // dest stride is always 1 (it's an array)
   for( ; dest != end; src += stride, ++dest ) {
      *dest = clamp_cast< dfloat >( *src );
   }
}

} // namespace

Image::Pixel::Pixel( FloatArray const& values, dip::DataType dt ) : dataType_( dt ), tensor_( values.size() ) {
   SetInternalData();
   DIP_OVL_CALL_ALL( WriteSamples, ( values.data(), origin_, values.size()), dataType_ );
}

Image::Pixel::operator FloatArray() const {
   FloatArray out( TensorElements() );
   DIP_OVL_CALL_ALL( ReadSamples, ( origin_, out.data(), out.size(), tensorStride_ ), dataType_ );
   return out;
}

//

void CopyFrom( Image const& src, Image& dest, Image const& mask ) {
   // Check input
   DIP_THROW_IF( !src.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !mask.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_STACK_TRACE_THIS( mask.CheckIsMask( src.Sizes(), Option::AllowSingletonExpansion::DONT_ALLOW, Option::ThrowException::DO_THROW ));
   dip::uint N = Count( mask );
   DIP_STACK_TRACE_THIS( dest.ReForge( UnsignedArray( { N } ), src.TensorElements(), src.DataType(), Option::AcceptDataTypeChange::DONT_ALLOW ));
   dest.CopyNonDataProperties( src );
   // Samples
   dip::uint telems = src.TensorElements();
   dip::uint bytes = DataType().SizeOf(); // both source and destination have the same types
   if(( src.TensorStride() == 1 ) && ( dest.TensorStride() == 1 )) {
      // We copy the whole tensor as a single data block
      bytes *= telems;
      telems = 1;
   }
   // Iterate over *this and mask, copying pixels to destination
   GenericJointImageIterator< 2 > srcIt( { src, mask } );
   GenericImageIterator<> destIt( dest );
   if( telems == 1 ) { // most frequent case, really.
      do {
         if( *( static_cast< bin* >( srcIt.Pointer< 1 >() ))) {
            std::memcpy( destIt.Pointer(), srcIt.Pointer< 0 >(), bytes );
            ++destIt;
         }
      } while( ++srcIt );
   } else {
      do {
         if( *( static_cast< bin* >( srcIt.Pointer< 1 >() ))) {
            for( dip::uint ii = 0; ii < telems; ++ii ) {
               std::memcpy( destIt.Pointer( ii ), srcIt.Pointer< 0 >( ii ), bytes );
            }
            ++destIt;
         }
      } while( ++srcIt );
   }
}

void CopyFrom( Image const& src, Image& dest, IntegerArray const& offsets ) {
   // Check input
   DIP_THROW_IF( !src.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( offsets.empty(), E::ARRAY_PARAMETER_EMPTY );
   DIP_STACK_TRACE_THIS( dest.ReForge( UnsignedArray( { offsets.size() } ), src.TensorElements(), src.DataType(), Option::AcceptDataTypeChange::DONT_ALLOW ));
   dest.CopyNonDataProperties( src );
   // Samples
   dip::uint telems = src.TensorElements();
   dip::uint bytes = DataType().SizeOf(); // both source and destination have the same types
   if(( src.TensorStride() == 1 ) && ( dest.TensorStride() == 1 )) {
      // We copy the whole tensor as a single data block
      bytes *= telems;
      telems = 1;
   }
   // Iterate over coordinates and destination, copying pixels to destination
   auto arrIt = offsets.begin();
   GenericImageIterator<> destIt( dest );
   if( telems == 1 ) { // most frequent case, really.
      do {
         std::memcpy( destIt.Pointer(), src.Pointer( *arrIt ), bytes );
      } while( ++arrIt, ++destIt ); // these two must end at the same time, we test the image iterator, as arrIt should be compared with the end iterator.
   } else {
      do {
         dip::sint offset = *arrIt;
         for( dip::uint ii = 0; ii < telems; ++ii ) {
            std::memcpy( destIt.Pointer( ii ), src.Pointer( offset ), bytes );
            offset += src.TensorStride();
         }
      } while( ++arrIt, ++destIt ); // these two must end at the same time, we test the image iterator, as arrIt should be compared with the end iterator.
   }
}

void CopyTo( Image const& src, Image& dest, Image const& mask ) {
   // Check input
   DIP_THROW_IF( !src.IsForged() || !dest.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( src.TensorElements() != dest.TensorElements(), E::NTENSORELEM_DONT_MATCH );
   DIP_THROW_IF( !mask.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_STACK_TRACE_THIS( mask.CheckIsMask( dest.Sizes(), Option::AllowSingletonExpansion::DONT_ALLOW, Option::ThrowException::DO_THROW ));
   if( dest.DataType() == src.DataType() ) {
      dip::uint telems = dest.TensorElements();
      dip::uint bytes = dest.DataType().SizeOf();
      if(( dest.TensorStride() == 1 ) && ( src.TensorStride() == 1 )) {
         // We copy the whole tensor as a single data block
         bytes *= telems;
         telems = 1;
      }
      // Iterate over dest and mask, copying pixels from source
      GenericJointImageIterator< 2 > destIt( { dest, mask } );
      GenericImageIterator<> srcIt( src );
      if( telems == 1 ) { // most frequent case, really.
         do {
            if( *( static_cast< bin* >( destIt.Pointer< 1 >() ))) {
               DIP_THROW_IF( !srcIt, E::SIZES_DONT_MATCH );
               std::memcpy( destIt.Pointer< 0 >(), srcIt.Pointer(), bytes );
               ++srcIt;
            }
         } while( ++destIt );
      } else {
         do {
            if( *( static_cast< bin* >( destIt.Pointer< 1 >() ))) {
               DIP_THROW_IF( !srcIt, E::SIZES_DONT_MATCH );
               for( dip::uint ii = 0; ii < telems; ++ii ) {
                  std::memcpy( destIt.Pointer< 0 >( ii ), srcIt.Pointer( ii ), bytes );
               }
               ++srcIt;
            }
         } while( ++destIt );
      }
   } else {
      // Iterate over dest and mask, copying pixels from source
      GenericJointImageIterator< 2 > destIt( { dest, mask } );
      GenericImageIterator<> srcIt( src );
      do {
         if( *( static_cast< bin* >( destIt.Pointer< 1 >() ))) {
            DIP_THROW_IF( !srcIt, E::SIZES_DONT_MATCH );
            destIt.Pixel< 0 >() = *srcIt;
            ++srcIt;
         }
      } while( ++destIt );
   }
}

void CopyTo( Image const& src, Image& dest, IntegerArray const& offsets ) {
   // Check input
   DIP_THROW_IF( !src.IsForged() || !dest.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( src.TensorElements() != dest.TensorElements(), E::NTENSORELEM_DONT_MATCH );
   DIP_THROW_IF( offsets.empty(), E::ARRAY_PARAMETER_EMPTY );
   DIP_THROW_IF( src.NumberOfPixels() != offsets.size(), "Number of pixels does not match offset list" );
   if( DataType() == src.DataType() ) {
      dip::uint telems = dest.TensorElements();
      dip::uint bytes = DataType().SizeOf(); // both source and destination have the same types
      if(( dest.TensorStride() == 1 ) && ( src.TensorStride() == 1 )) {
         // We copy the whole tensor as a single data block
         bytes *= telems;
         telems = 1;
      }
      // Iterate over offsets and src, copying pixels to dest
      auto indIt = offsets.begin();
      GenericImageIterator<> srcIt( src );
      if( telems == 1 ) { // most frequent case, really.
         do {
            std::memcpy( dest.Pointer( *indIt ), srcIt.Pointer(), bytes );
         } while( ++indIt, ++srcIt ); // these two must end at the same time, we test the image iterator, as indIt should be compared with the end iterator.
      } else {
         do {
            dip::sint offset = *indIt;
            for( dip::uint ii = 0; ii < telems; ++ii ) {
               std::memcpy( dest.Pointer( offset ), srcIt.Pointer( ii ), bytes );
               offset += dest.TensorStride();
            }
         } while( ++indIt, ++srcIt ); // these two must end at the same time, we test the image iterator, as indIt should be compared with the end iterator.
      }
   } else {
      // Iterate over offsets and src, copying pixels to dest
      auto arrIt = offsets.begin();
      GenericImageIterator<> srcIt( src );
      do {
         Image::Pixel d( dest.Pointer( *arrIt ), dest.DataType(), dest.Tensor(), dest.TensorStride() );
         d = *srcIt;
      } while( ++arrIt, ++srcIt ); // these two must end at the same time, we test the image iterator, as indIt should be compared with the end iterator.
   }
}

//

Image Image::Pad( UnsignedArray const& sizes, Option::CropLocation cropLocation ) const {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = sizes_.size();
   DIP_THROW_IF( sizes.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );
   DIP_THROW_IF( sizes < sizes_, E::INDEX_OUT_OF_RANGE );
   Image out;
   out.CopyProperties( *this );
   out.sizes_ = sizes;
   out.Forge();
   out.Fill( 0 );
   auto tmp = out.Cropped( sizes_, cropLocation ); // this is a view into the new image that corresponds to *this
   tmp.Copy( *this ); // copy the data over, we're done!
   return out;
}

//

void Image::Copy( Image const& src ) {
   DIP_THROW_IF( !src.IsForged(), E::IMAGE_NOT_FORGED );
   if( &src == this ) {
      // Copying self... what for?
      return;
   }
   if( IsForged() ) {
      if( IsIdenticalView( src )) {
         // Copy is a no-op, make sure additional properties are identical also
         CopyNonDataProperties( src );
         return;
      }
      if( !CompareProperties( src, Option::CmpProp::Sizes + Option::CmpProp::TensorElements,
                              Option::ThrowException::DONT_THROW ) || IsOverlappingView( src )) {
         // We cannot reuse the data segment
         DIP_STACK_TRACE_THIS( Strip() );
      } else {
         // We've got the data segment covered. Copy over additional properties
         CopyNonDataProperties( src );
      }
   }
   if( !IsForged() ) {
      CopyProperties( src );
      DIP_STACK_TRACE_THIS( Forge() );
   }
   // A single CopyBuffer call if both images have simple strides and same dimension order
   dip::sint sstride_d;
   void* origin_d;
   std::tie( sstride_d, origin_d ) = GetSimpleStrideAndOrigin();
   if( origin_d ) {
      //std::cout << "dip::Image::Copy: destination has simple strides\n";
      dip::sint sstride_s;
      void* origin_s;
      std::tie( sstride_s, origin_s ) = src.GetSimpleStrideAndOrigin();
      if( origin_s ) {
         //std::cout << "dip::Image::Copy: source has simple strides\n";
         if( HasSameDimensionOrder( src )) {
            // No need to loop
            //std::cout << "dip::Image::Copy: no need to loop\n";
            detail::CopyBuffer(
                  origin_s,
                  src.dataType_,
                  sstride_s,
                  src.tensorStride_,
                  origin_d,
                  dataType_,
                  sstride_d,
                  tensorStride_,
                  NumberOfPixels(),
                  tensor_.Elements()
            );
            return;
         }
      }
   }
   // Otherwise, make nD loop
   //std::cout << "dip::Image::Copy: nD loop\n";
   dip::uint processingDim = Framework::OptimalProcessingDim( src );
   GenericJointImageIterator< 2 > it( { src, *this }, processingDim );
   dip::sint srcStride = src.strides_[ processingDim ];
   dip::sint destStride = strides_[ processingDim ];
   dip::uint nPixels = sizes_[ processingDim ];
   dip::uint nTElems = tensor_.Elements();
   do {
      detail::CopyBuffer(
            it.InPointer(),
            src.dataType_,
            srcStride,
            src.tensorStride_,
            it.OutPointer(),
            dataType_,
            destStride,
            tensorStride_,
            nPixels,
            nTElems
      );
   } while( ++it );
}

//

void Image::ExpandTensor() {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   if( !Tensor().HasNormalOrder() ) {
      //if( TensorShape() == dip::Tensor::Shape::ROW_MAJOR_MATRIX ) {
         // Shuffle the data
         // TODO: shuffle the data instead of copying it over to a new image.
         // return
      //}
      // Copy the data into a new segment
      std::vector< dip::sint > lookUpTable = Tensor().LookUpTable();
      dip::Tensor tensor{ Tensor().Rows(), Tensor().Columns() };
      dip::PixelSize pixelSize = pixelSize_;
      Image source = QuickCopy();
      // Prepare output image
      ReForge( source.Sizes(), tensor.Elements(), source.DataType(), Option::AcceptDataTypeChange::DO_ALLOW );
      tensor_ = tensor;
      pixelSize_ = pixelSize;
      // A single CopyBuffer call if both images have simple strides and same dimension order
      dip::sint sstride_d;
      void* origin_d;
      std::tie( sstride_d, origin_d ) = GetSimpleStrideAndOrigin();
      if( origin_d ) {
         //std::cout << "dip::ExpandTensor: destination has simple strides\n";
         dip::sint sstride_s;
         void* origin_s;
         std::tie( sstride_s, origin_s ) = source.GetSimpleStrideAndOrigin();
         if( origin_s ) {
            //std::cout << "dip::ExpandTensor: source has simple strides\n";
            if( HasSameDimensionOrder( source )) {
               // No need to loop
               //std::cout << "dip::ExpandTensor: no need to loop\n";
               detail::CopyBuffer(
                     origin_s,
                     source.DataType(),
                     sstride_s,
                     source.TensorStride(),
                     origin_d,
                     DataType(),
                     sstride_d,
                     TensorStride(),
                     NumberOfPixels(),
                     TensorElements(),
                     lookUpTable
               );
               return;
            }
         }
      }
      // Otherwise, make nD loop
      //std::cout << "dip::ExpandTensor: nD loop\n";
      dip::uint processingDim = Framework::OptimalProcessingDim( source );
      GenericJointImageIterator< 2 > it( { source, *this }, processingDim );
      dip::DataType srcDataType = source.DataType();
      dip::sint srcStride = source.Stride( processingDim );
      dip::sint srcTStride = source.TensorStride();
      dip::DataType destDataType = DataType();
      dip::sint destStride = Stride( processingDim );
      dip::sint destTStride = TensorStride();
      dip::uint nPixels = Size( processingDim );
      dip::uint nTElems = TensorElements();
      do {
         detail::CopyBuffer(
               it.InPointer(),
               srcDataType,
               srcStride,
               srcTStride,
               it.OutPointer(),
               destDataType,
               destStride,
               destTStride,
               nPixels,
               nTElems,
               lookUpTable
         );
      } while( ++it );
   }
}

//

void Image::Convert( dip::DataType dt ) {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   if( dt != dataType_ ) {
      if(( dataType_ == DT_BIN ) && (( dt == DT_UINT8 ) || ( dt == DT_SINT8 ))) {
         // bin->uint8 or bin->sint8
         // These can happen without touching the data, it's OK even if the data is shared. Just change the flag.
         dataType_ = dt;
         return;
      }
      if( !IsShared() && ( dt.SizeOf() == dataType_.SizeOf() )) {
         // The operation can happen in place.
         // Loop over all pixels, casting with clamp each of the values; finally set the data type field.
         dip::sint sstride;
         void* origin;
         std::tie( sstride, origin ) = GetSimpleStrideAndOrigin();
         if( origin ) {
            // No need to loop
            //std::cout << "dip::Image::Convert: in-place, no need to loop\n";
            detail::CopyBuffer(
                  origin,
                  dataType_,
                  sstride,
                  tensorStride_,
                  origin,
                  dt,
                  sstride,
                  tensorStride_,
                  NumberOfPixels(),
                  tensor_.Elements()
            );
         } else {
            // Make nD loop
            //std::cout << "dip::Image::Convert: in-place, nD loop\n";
            dip::uint processingDim = Framework::OptimalProcessingDim( *this );
            GenericImageIterator<> it( *this, processingDim );
            it.OptimizeAndFlatten();
            do {
               detail::CopyBuffer(
                     it.Pointer(),
                     dataType_,
                     strides_[ processingDim ],
                     tensorStride_,
                     it.Pointer(),
                     dt,
                     strides_[ processingDim ],
                     tensorStride_,
                     sizes_[ processingDim ],
                     tensor_.Elements()
               );
            } while( ++it );
         }
         dataType_ = dt;
      } else {
         // We need to create a new data segment and copy it over.
         // Simply create a new image, identical copy of *this, with a different data type, copy
         // the data, then swap the two images.
         //std::cout << "dip::Image::Convert: using Copy\n";
         Image newimg;
         newimg.externalInterface_ = externalInterface_;
         newimg.ReForge( *this, dt );
         newimg.Copy( *this );
         this->move( std::move( newimg ));
      }
   }
}

//

namespace {

template< typename TPI >
static inline void InternFill( Image& dest, TPI value ) {
   DIP_THROW_IF( !dest.IsForged(), E::IMAGE_NOT_FORGED );
   dip::sint sstride;
   void* origin;
   std::tie( sstride, origin ) = dest.GetSimpleStrideAndOrigin();
   if( origin ) {
      // No need to loop
      detail::FillBufferFromTo( static_cast< TPI* >( origin ), sstride, dest.TensorStride(), dest.NumberOfPixels(), dest.TensorElements(), value );
   } else {
      // Make nD loop
      dip::uint processingDim = Framework::OptimalProcessingDim( dest );
      ImageIterator< TPI > it( dest, processingDim );
      it.OptimizeAndFlatten();
      dip::uint size = it.ProcessingDimensionSize();
      dip::sint stride = it.ProcessingDimensionStride();
      do {
         detail::FillBufferFromTo( it.Pointer(), stride, dest.TensorStride(), size, dest.TensorElements(), value );
      } while( ++it );
   }
}

} // namespace

void Image::Fill( Image::Pixel const& pixel ) {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint N = tensor_.Elements();
   if( pixel.TensorElements() == 1 ) {
      DIP_STACK_TRACE_THIS( Fill( pixel[ 0 ] ));
   } else {
      DIP_THROW_IF( pixel.TensorElements() != N, E::NTENSORELEM_DONT_MATCH );
      Image tmp = QuickCopy();
      tmp.tensor_.SetScalar();
      for( dip::uint ii = 0; ii < N; ++ii, tmp.origin_ = tmp.Pointer( tmp.tensorStride_ )) {
         // NOTE: tmp.Pointer( tmp.tensorStride_ ) takes the current tmp.origin_ and adds the tensor stride to it.
         // Thus, assigning this into tmp.origin_ is equivalent to tmp.origin += tmp_tensorStride_ if tmp.origin_
         // were a pointer to the correct data type.
         DIP_STACK_TRACE_THIS( tmp.Fill( pixel[ ii ] ));
      }
   }
}

void Image::Fill( Image::Sample const& sample ) {
   switch( dataType_ ) {
      case DT_BIN:      InternFill( *this, sample.As< bin      >() ); break;
      case DT_UINT8:    InternFill( *this, sample.As< uint8    >() ); break;
      case DT_SINT8:    InternFill( *this, sample.As< sint8    >() ); break;
      case DT_UINT16:   InternFill( *this, sample.As< uint16   >() ); break;
      case DT_SINT16:   InternFill( *this, sample.As< sint16   >() ); break;
      case DT_UINT32:   InternFill( *this, sample.As< uint32   >() ); break;
      case DT_SINT32:   InternFill( *this, sample.As< sint32   >() ); break;
      case DT_SFLOAT:   InternFill( *this, sample.As< sfloat   >() ); break;
      case DT_DFLOAT:   InternFill( *this, sample.As< dfloat   >() ); break;
      case DT_SCOMPLEX: InternFill( *this, sample.As< scomplex >() ); break;
      case DT_DCOMPLEX: InternFill( *this, sample.As< dcomplex >() ); break;
      default:
         DIP_THROW( E::DATA_TYPE_NOT_SUPPORTED );
   }
}

} // namespace dip
