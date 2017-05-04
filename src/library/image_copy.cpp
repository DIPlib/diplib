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
#include "diplib/generic_iterators.h"
#include "diplib/framework.h"
#include "diplib/iterators.h"
#include "copy_buffer.h"


namespace dip {


//
Image Image::CopyAt( Image const& mask ) const {
   // Check input
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !mask.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_START_STACK_TRACE
      mask.CheckIsMask( Sizes(), Option::AllowSingletonExpansion::DONT_ALLOW, Option::ThrowException::DO_THROW );
   DIP_END_STACK_TRACE
   // Create output
   dip::uint N = Count( mask );
   Image destination;
   destination.CopyProperties( *this );
   destination.SetSizes( UnsignedArray({ N }) );
   destination.Forge();
   // Samples
   dip::uint telems = TensorElements();
   dip::uint bytes = DataType().SizeOf(); // both source and destination have the same types
   if(( TensorStride() == 1 ) && ( destination.TensorStride() == 1 )) {
      // We copy the whole tensor as a single data block
      bytes *= telems;
      telems = 1;
   }
   // Iterate over *this and mask, copying pixels to destination
   GenericJointImageIterator< 2 > srcIt( { *this, mask } );
   GenericImageIterator destIt( destination );
   if( telems == 1 ) { // most frequent case, really.
      do {
         if( *( static_cast< bin* >( srcIt.Pointer< 1 >() ) ) ) {
            std::memcpy( destIt.Pointer(), srcIt.Pointer< 0 >(), bytes );
            ++destIt;
         }
      } while( ++srcIt );
   } else {
      do {
         if( *( static_cast< bin* >( srcIt.Pointer< 1 >() ) ) ) {
            for( dip::uint ii = 0; ii < telems; ++ii ) {
               std::memcpy( destIt.Sample( ii ), srcIt.Pointer< 0 >( ii ), bytes );
            }
            ++destIt;
         }
      } while( ++srcIt );
   }
   // Finished
   return destination;
}

//
Image Image::CopyAt( UnsignedArray const& indices ) const {
   // Check input
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( indices.size() == 0, E::ARRAY_ILLEGAL_SIZE );
   dip::uint maxIndex = NumberOfPixels();
   for( auto const& ii : indices ) {
      DIP_THROW_IF( ii >= maxIndex, E::INDEX_OUT_OF_RANGE );
   }
   CoordinatesComputer coordinates = IndexToCoordinatesComputer();
   // Create output
   Image destination;
   destination.CopyProperties( *this );
   destination.SetSizes( UnsignedArray({ indices.size() }) );
   destination.Forge();
   // Samples
   dip::uint telems = TensorElements();
   dip::uint bytes = DataType().SizeOf(); // both source and destination have the same types
   if(( TensorStride() == 1 ) && ( destination.TensorStride() == 1 )) {
      // We copy the whole tensor as a single data block
      bytes *= telems;
      telems = 1;
   }
   // Iterate over indices and destination, copying pixels to destination
   auto indIt = indices.begin();
   GenericImageIterator destIt( destination );
   if( telems == 1 ) { // most frequent case, really.
      do {
         std::memcpy( destIt.Pointer(), Pointer( coordinates( static_cast< dip::sint >( *indIt ))), bytes );
      } while( ++indIt, ++destIt ); // these two must end at the same time, we test the image iterator, as indIt should be compared with the end iterator.
   } else {
      do {
         dip::sint offset = Offset( coordinates( static_cast< dip::sint >( *indIt )));
         for( dip::uint ii = 0; ii < telems; ++ii ) {
            std::memcpy( destIt.Sample( ii ), Pointer( offset + static_cast< dip::sint >( ii ) * TensorStride() ), bytes );
         }
      } while( ++indIt, ++destIt ); // these two must end at the same time, we test the image iterator, as indIt should be compared with the end iterator.
   }
   // Finished
   return destination;
}

//
Image Image::CopyAt( CoordinateArray const& coordinates ) const {
   // Check input
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( coordinates.size() == 0, E::ARRAY_ILLEGAL_SIZE );
   dip::uint nDims = Dimensionality();
   for( UnsignedArray const& pp : coordinates ) {
      DIP_THROW_IF( pp.size() != nDims, E::COORDINATES_OUT_OF_RANGE );
      DIP_THROW_IF( !( pp < Sizes() ), E::COORDINATES_OUT_OF_RANGE );
   }
   // Create output
   Image destination;
   destination.CopyProperties( *this );
   destination.SetSizes( UnsignedArray({ coordinates.size() }) );
   destination.Forge();
   // Samples
   dip::uint telems = TensorElements();
   dip::uint bytes = DataType().SizeOf(); // both source and destination have the same types
   if(( TensorStride() == 1 ) && ( destination.TensorStride() == 1 )) {
      // We copy the whole tensor as a single data block
      bytes *= telems;
      telems = 1;
   }
   // Iterate over coordinates and destination, copying pixels to destination
   auto corIt = coordinates.begin();
   GenericImageIterator destIt( destination );
   if( telems == 1 ) { // most frequent case, really.
      do {
         std::memcpy( destIt.Pointer(), Pointer( *corIt ), bytes );
      } while( ++corIt, ++destIt ); // these two must end at the same time, we test the image iterator, as corIt should be compared with the end iterator.
   } else {
      do {
         dip::sint offset = Offset( *corIt );
         for( dip::uint ii = 0; ii < telems; ++ii ) {
            std::memcpy( destIt.Sample( ii ), Pointer( offset + static_cast< dip::sint >( ii ) * TensorStride() ), bytes );
         }
      } while( ++corIt, ++destIt ); // these two must end at the same time, we test the image iterator, as corIt should be compared with the end iterator.
   }
   // Finished
   return destination;
}

//
void Image::CopyAt( Image const& source, Image const& mask, Option::ThrowException throws ) {
   // Check input
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !source.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !mask.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( TensorElements() != source.TensorElements(), E::NTENSORELEM_DONT_MATCH );
   DIP_START_STACK_TRACE
      mask.CheckIsMask( Sizes(), Option::AllowSingletonExpansion::DONT_ALLOW, Option::ThrowException::DO_THROW );
   DIP_END_STACK_TRACE
   if( throws == Option::ThrowException::DO_THROW ) {
      // Test the size of data
      dip::uint N = Count( mask );
      DIP_THROW_IF( source.NumberOfPixels() != N, "Number of pixels does not match mask" );
   }
   if( DataType() == source.DataType() ) {
      dip::uint telems = TensorElements();
      dip::uint bytes = DataType().SizeOf(); // both source and destination have the same types
      if(( TensorStride() == 1 ) && ( source.TensorStride() == 1 )) {
         // We copy the whole tensor as a single data block
         bytes *= telems;
         telems = 1;
      }
      // Iterate over *this and mask, copying pixels from source
      GenericJointImageIterator< 2 > destIt( { *this, mask } );
      GenericImageIterator srcIt( source );
      if( telems == 1 ) { // most frequent case, really.
         do {
            if( *( static_cast< bin* >( destIt.Pointer< 1 >() ))) {
               std::memcpy( destIt.Pointer< 0 >(), srcIt.Pointer(), bytes );
               ++srcIt;
            }
         } while( ++destIt );
      } else {
         do {
            if( *( static_cast< bin* >( destIt.Pointer< 1 >() ))) {
               for( dip::uint ii = 0; ii < telems; ++ii ) {
                  std::memcpy( destIt.Pointer< 0 >( ii ), srcIt.Sample( ii ), bytes );
               }
               ++srcIt;
            }
         } while( ++destIt );
      }
   } else {
      // Iterate over *this and mask, copying pixels from source
      GenericJointImageIterator< 2 > destIt( { *this, mask } );
      GenericImageIterator srcIt( source );
      do {
         if( *( static_cast< bin* >( destIt.Pointer< 1 >() ))) {
            // This might not be the most efficient way, but it's effective and prevents us from defining yet another chain of 2 templated functions.
            CopyBuffer(
                  srcIt.Pointer(),
                  source.DataType(),
                  1, // stride ignored, we're reading only one pixel
                  source.TensorStride(),
                  destIt.Pointer< 0 >(),
                  DataType(),
                  1, // stride ignored, we're reading only one pixel
                  TensorStride(),
                  1, // one pixel to copy
                  TensorElements()
            );
            ++srcIt;
         }
      } while( ++destIt );
   }
}

//
void Image::CopyAt( Image const& source, UnsignedArray const& indices ) {
   // Check input
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !source.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( TensorElements() != source.TensorElements(), E::NTENSORELEM_DONT_MATCH );
   DIP_THROW_IF( indices.size() == 0, E::ARRAY_ILLEGAL_SIZE );
   DIP_THROW_IF( source.NumberOfPixels() != indices.size(), "Number of pixels does not match index list" );
   dip::uint maxIndex = NumberOfPixels();
   for( auto const& ii : indices ) {
      DIP_THROW_IF( ii >= maxIndex, E::INDEX_OUT_OF_RANGE );
   }
   CoordinatesComputer coordinates = IndexToCoordinatesComputer();
   if( DataType() == source.DataType() ) {
      dip::uint telems = TensorElements();
      dip::uint bytes = DataType().SizeOf(); // both source and destination have the same types
      if(( TensorStride() == 1 ) && ( source.TensorStride() == 1 )) {
         // We copy the whole tensor as a single data block
         bytes *= telems;
         telems = 1;
      }
      // Iterate over indices and source, copying pixels from source
      auto indIt = indices.begin();
      GenericImageIterator srcIt( source );
      if( telems == 1 ) { // most frequent case, really.
         do {
            std::memcpy( Pointer( coordinates( static_cast< dip::sint >( *indIt ))), srcIt.Pointer(), bytes );
         } while( ++indIt, ++srcIt ); // these two must end at the same time, we test the image iterator, as indIt should be compared with the end iterator.
      } else {
         do {
            dip::sint offset = Offset( coordinates( static_cast< dip::sint >( *indIt )));
            for( dip::uint ii = 0; ii < telems; ++ii ) {
               std::memcpy( Pointer( offset + static_cast< dip::sint >( ii ) * TensorStride() ), srcIt.Sample( ii ), bytes );
            }
         } while( ++indIt, ++srcIt ); // these two must end at the same time, we test the image iterator, as indIt should be compared with the end iterator.
      }
   } else {
      // Iterate over indices and source, copying pixels from source
      auto indIt = indices.begin();
      GenericImageIterator srcIt( source );
      do {
         // This might not be the most efficient way, but it's effective and prevents us from defining yet another chain of 2 templated functions.
         CopyBuffer(
               srcIt.Pointer(),
               source.DataType(),
               1, // stride ignored, we're reading only one pixel
               source.TensorStride(),
               Pointer( coordinates( static_cast< dip::sint >( *indIt ))),
               DataType(),
               1, // stride ignored, we're reading only one pixel
               TensorStride(),
               1, // one pixel to copy
               TensorElements()
         );
      } while( ++indIt, ++srcIt ); // these two must end at the same time, we test the image iterator, as indIt should be compared with the end iterator.
   }
}

//
void Image::CopyAt( Image const& source, CoordinateArray const& coordinates ) {
   // Check input
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !source.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( TensorElements() != source.TensorElements(), E::NTENSORELEM_DONT_MATCH );
   DIP_THROW_IF( coordinates.size() == 0, E::ARRAY_ILLEGAL_SIZE );
   DIP_THROW_IF( source.NumberOfPixels() != coordinates.size(), "Number of pixels does not match coordinate list" );
   dip::uint nDims = Dimensionality();
   for( UnsignedArray const& pp : coordinates ) {
      DIP_THROW_IF( pp.size() != nDims, E::COORDINATES_OUT_OF_RANGE );
      DIP_THROW_IF( !( pp < Sizes() ), E::COORDINATES_OUT_OF_RANGE );
   }
   if( DataType() == source.DataType() ) {
      dip::uint telems = TensorElements();
      dip::uint bytes = DataType().SizeOf(); // both source and destination have the same types
      if(( TensorStride() == 1 ) && ( source.TensorStride() == 1 )) {
         // We copy the whole tensor as a single data block
         bytes *= telems;
         telems = 1;
      }
      // Iterate over coordinates and source, copying pixels from source
      auto corIt = coordinates.begin();
      GenericImageIterator srcIt( source );
      if( telems == 1 ) { // most frequent case, really.
         do {
            std::memcpy( Pointer( *corIt ), srcIt.Pointer(), bytes );
         } while( ++corIt, ++srcIt ); // these two must end at the same time, we test the image iterator, as corIt should be compared with the end iterator.
      } else {
         do {
            dip::sint offset = Offset( *corIt );
            for( dip::uint ii = 0; ii < telems; ++ii ) {
               std::memcpy( Pointer( offset + static_cast< dip::sint >( ii ) * TensorStride() ), srcIt.Sample( ii ), bytes );
            }
         } while( ++corIt, ++srcIt ); // these two must end at the same time, we test the image iterator, as corIt should be compared with the end iterator.
      }
   } else {
      // Iterate over coordinates and source, copying pixels from source
      auto corIt = coordinates.begin();
      GenericImageIterator srcIt( source );
      do {
         // This might not be the most efficient way, but it's effective and prevents us from defining yet another chain of 2 templated functions.
         CopyBuffer(
               srcIt.Pointer(),
               source.DataType(),
               1, // stride ignored, we're reading only one pixel
               source.TensorStride(),
               Pointer( *corIt ),
               DataType(),
               1, // stride ignored, we're reading only one pixel
               TensorStride(),
               1, // one pixel to copy
               TensorElements()
         );
      } while( ++corIt, ++srcIt ); // these two must end at the same time, we test the image iterator, as corIt should be compared with the end iterator.
   }
}

//
Image Image::Pad( UnsignedArray const& sizes, Option::CropLocation cropLocation ) const {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = sizes_.size();
   DIP_THROW_IF( sizes.size() != nDims, E::ARRAY_ILLEGAL_SIZE );
   DIP_THROW_IF( sizes < sizes_, E::INDEX_OUT_OF_RANGE );
   Image out;
   out.CopyProperties( *this );
   out.sizes_ = sizes;
   out.Forge();
   out.Fill( 0 );
   Image tmp = out.Crop( sizes_, cropLocation ); // this is a view into the new image that corresponds to *this
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
      if( !CompareProperties( src, Option::CmpProps_Sizes + Option::CmpProps_TensorElements ) ||
            IsOverlappingView( src )) {
         // We cannot reuse the data segment
         Strip();
      } else {
         // We've got the data segment covered. Copy over additional properties
         CopyNonDataProperties( src );
      }
   }
   if( !IsForged() ) {
      CopyProperties( src );
      Forge();
   }
   // A single CopyBuffer call if both images have simple strides and same dimension order
   dip::sint sstride_d;
   void* porigin_d;
   GetSimpleStrideAndOrigin( sstride_d, porigin_d );
   if( porigin_d ) {
      //std::cout << "dip::Image::Copy: destination has simple strides\n";
      dip::sint sstride_s;
      void* porigin_s;
      src.GetSimpleStrideAndOrigin( sstride_s, porigin_s );
      if( porigin_s ) {
         //std::cout << "dip::Image::Copy: source has simple strides\n";
         if( HasSameDimensionOrder( src )) {
            // No need to loop
            //std::cout << "dip::Image::Copy: no need to loop\n";
            CopyBuffer(
                  porigin_s,
                  src.dataType_,
                  sstride_s,
                  src.tensorStride_,
                  porigin_d,
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
      CopyBuffer(
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
         // TODO
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
      void* porigin_d;
      GetSimpleStrideAndOrigin( sstride_d, porigin_d );
      if( porigin_d ) {
         //std::cout << "dip::ExpandTensor: destination has simple strides\n";
         dip::sint sstride_s;
         void* porigin_s;
         source.GetSimpleStrideAndOrigin( sstride_s, porigin_s );
         if( porigin_s ) {
            //std::cout << "dip::ExpandTensor: source has simple strides\n";
            if( HasSameDimensionOrder( source )) {
               // No need to loop
               //std::cout << "dip::ExpandTensor: no need to loop\n";
               CopyBuffer(
                     porigin_s,
                     source.DataType(),
                     sstride_s,
                     source.TensorStride(),
                     porigin_d,
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
         CopyBuffer(
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
      if( !IsShared() && ( dt.SizeOf() == dataType_.SizeOf() )) {
         // The operation can happen in place.
         // Loop over all pixels, casting with clamp each of the values; finally set the data type field.
         dip::sint sstride;
         void* porigin;
         GetSimpleStrideAndOrigin( sstride, porigin );
         if( porigin ) {
            // No need to loop
            //std::cout << "dip::Image::Convert: in-place, no need to loop\n";
            CopyBuffer(
                  porigin,
                  dataType_,
                  sstride,
                  tensorStride_,
                  porigin,
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
            auto it = GenericImageIterator( *this, processingDim );
            do {
               CopyBuffer(
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
         newimg.ReForge( *this, dt );
         newimg.Copy( *this );
         swap( newimg );
      }
   }
}

//

namespace {

template< typename TPI >
static inline void InternFill( Image& dest, TPI value ) {
   DIP_THROW_IF( !dest.IsForged(), E::IMAGE_NOT_FORGED );
   dip::sint sstride_d;
   void* porigin_d;
   dest.GetSimpleStrideAndOrigin( sstride_d, porigin_d );
   if( porigin_d ) {
      // No need to loop
      FillBufferFromTo( static_cast< TPI* >( porigin_d ), sstride_d, dest.TensorStride(), dest.NumberOfPixels(), dest.TensorElements(), value );
   } else {
      // Make nD loop
      dip::uint processingDim = Framework::OptimalProcessingDim( dest );
      auto it = ImageIterator< TPI >( dest, processingDim );
      do {
         FillBufferFromTo( it.Pointer(), dest.Stride( processingDim ), dest.TensorStride(), dest.Size( processingDim ), dest.TensorElements(), value );
      } while( ++it );
   }
}

} // namespace

void Image::Fill( Image::Pixel const& pixel ) {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint N = tensor_.Elements();
   DIP_THROW_IF( pixel.TensorElements() != N, E::NTENSORELEM_DONT_MATCH );
   Image tmp = QuickCopy();
   tmp.tensor_.SetScalar();
   for( dip::uint ii = 0; ii < N; ++ii, tmp.origin_ = tmp.Pointer( tmp.tensorStride_ )) {
      // NOTE: tmp.Pointer( tmp.tensorStride_ ) takes the current tmp.origin_ and adds the tensor stride to it.
      // Thus, assigning this into tmp.origin_ is equivalent to tmp.origin += tmp_tensorStride_ if tmp.origin_
      // were a pointer to the correct data type.
      tmp.Fill( pixel[ ii ] );
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
