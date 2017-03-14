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
#include "diplib/iterators.h"
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/math.h"
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
   GenericJointImageIterator srcIt( *this, mask ); // srcIt.InXXX is for *this, srcIt.OutXXX is for mask
   GenericImageIterator destIt( destination );
   if( telems == 1 ) { // most frequent case, really.
      do {
         if( *( static_cast< bin* >( srcIt.OutPointer() ) ) ) {
            std::memcpy( destIt.Pointer(), srcIt.InPointer(), bytes );
            ++destIt;
         }
      } while( ++srcIt );
   } else {
      do {
         if( *( static_cast< bin* >( srcIt.OutPointer() ) ) ) {
            for( dip::uint ii = 0; ii < telems; ++ii ) {
               std::memcpy( destIt.Sample( ii ), srcIt.InSample( ii ), bytes );
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
         std::memcpy( destIt.Pointer(), Pointer( coordinates( *indIt )), bytes );
      } while( ++indIt, ++destIt ); // these two must end at the same time, we test the image iterator, as indIt should be compared with the end iterator.
   } else {
      do {
         dip::sint offset = Offset( coordinates( *indIt ));
         for( dip::uint ii = 0; ii < telems; ++ii ) {
            std::memcpy( destIt.Sample( ii ), Pointer( offset + ii * TensorStride() ), bytes );
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
            std::memcpy( destIt.Sample( ii ), Pointer( offset + ii * TensorStride() ), bytes );
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
      GenericJointImageIterator destIt( *this, mask ); // destIt.InXXX is for *this, destIt.OutXXX is for mask
      GenericImageIterator srcIt( source );
      if( telems == 1 ) { // most frequent case, really.
         do {
            if( *( static_cast< bin* >( destIt.OutPointer() ))) {
               std::memcpy( destIt.InPointer(), srcIt.Pointer(), bytes );
               ++srcIt;
            }
         } while( ++destIt );
      } else {
         do {
            if( *( static_cast< bin* >( destIt.OutPointer() ))) {
               for( dip::uint ii = 0; ii < telems; ++ii ) {
                  std::memcpy( destIt.InSample( ii ), srcIt.Sample( ii ), bytes );
               }
               ++srcIt;
            }
         } while( ++destIt );
      }
   } else {
      // Iterate over *this and mask, copying pixels from source
      GenericJointImageIterator destIt( *this, mask ); // destIt.InXXX is for *this, destIt.OutXXX is for mask
      GenericImageIterator srcIt( source );
      do {
         if( *( static_cast< bin* >( destIt.OutPointer() ))) {
            // This might not be the most efficient way, but it's effective and prevents us from defining yet another chain of 2 templated functions.
            CopyBuffer(
                  srcIt.Pointer(),
                  source.DataType(),
                  1, // stride ignored, we're reading only one pixel
                  source.TensorStride(),
                  destIt.InPointer(),
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
            std::memcpy( Pointer( coordinates( *indIt )), srcIt.Pointer(), bytes );
         } while( ++indIt, ++srcIt ); // these two must end at the same time, we test the image iterator, as indIt should be compared with the end iterator.
      } else {
         do {
            dip::sint offset = Offset( coordinates( *indIt ));
            for( dip::uint ii = 0; ii < telems; ++ii ) {
               std::memcpy( Pointer( offset + ii * TensorStride() ), srcIt.Sample( ii ), bytes );
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
               Pointer( coordinates( *indIt )),
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
               std::memcpy( Pointer( offset + ii * TensorStride() ), srcIt.Sample( ii ), bytes );
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
   dip::uint sstride_d;
   void* porigin_d;
   GetSimpleStrideAndOrigin( sstride_d, porigin_d );
   if( porigin_d ) {
      //std::cout << "dip::Image::Copy: destination has simple strides\n";
      dip::uint sstride_s;
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
                  static_cast< dip::sint >( sstride_s ),
                  src.tensorStride_,
                  porigin_d,
                  dataType_,
                  static_cast< dip::sint >( sstride_d ),
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
   auto it = dip::GenericJointImageIterator( src, *this, processingDim );
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
void ExpandTensor(
      Image const& src,
      Image& dest
) {
   DIP_THROW_IF( !src.IsForged(), E::IMAGE_NOT_FORGED );
   if( src.Tensor().HasNormalOrder() ) {
      dest.Copy( src );
   } else {
      // Prepare data
      std::vector< dip::sint > lookUpTable = src.Tensor().LookUpTable();
      Tensor tensor{ src.Tensor().Rows(), src.Tensor().Columns() };
      Image source = src.QuickCopy(); // preserve the image in case `&src`==`&dest`.
      PixelSize pixelSize = src.PixelSize();
      // Prepare output image
      dest.ReForge( source.Sizes(), tensor.Elements(), source.DataType(), Option::AcceptDataTypeChange::DO_ALLOW );
      dest.ReshapeTensor( tensor );
      dest.SetPixelSize( pixelSize );
      // A single CopyBuffer call if both images have simple strides and same dimension order
      dip::uint sstride_d;
      void* porigin_d;
      dest.GetSimpleStrideAndOrigin( sstride_d, porigin_d );
      if( porigin_d ) {
         //std::cout << "dip::ExpandTensor: destination has simple strides\n";
         dip::uint sstride_s;
         void* porigin_s;
         source.GetSimpleStrideAndOrigin( sstride_s, porigin_s );
         if( porigin_s ) {
            //std::cout << "dip::ExpandTensor: source has simple strides\n";
            if( dest.HasSameDimensionOrder( source )) {
               // No need to loop
               //std::cout << "dip::ExpandTensor: no need to loop\n";
               CopyBuffer(
                     porigin_s,
                     source.DataType(),
                     static_cast< dip::sint >( sstride_s ),
                     source.TensorStride(),
                     porigin_d,
                     dest.DataType(),
                     static_cast< dip::sint >( sstride_d ),
                     dest.TensorStride(),
                     dest.NumberOfPixels(),
                     dest.TensorElements(),
                     lookUpTable
               );
               return;
            }
         }
      }
      // Otherwise, make nD loop
      //std::cout << "dip::ExpandTensor: nD loop\n";
      dip::uint processingDim = Framework::OptimalProcessingDim( source );
      auto it = dip::GenericJointImageIterator( source, dest, processingDim );
      DataType srcDataType = source.DataType();
      dip::sint srcStride = source.Stride( processingDim );
      dip::sint srcTStride = source.TensorStride();
      DataType destDataType = dest.DataType();
      dip::sint destStride = dest.Stride( processingDim );
      dip::sint destTStride = dest.TensorStride();
      dip::uint nPixels = dest.Size( processingDim );
      dip::uint nTElems = dest.TensorElements();
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
         dip::uint sstride;
         void* porigin;
         GetSimpleStrideAndOrigin( sstride, porigin );
         if( porigin ) {
            // No need to loop
            //std::cout << "dip::Image::Convert: in-place, no need to loop\n";
            CopyBuffer(
                  porigin,
                  dataType_,
                  static_cast< dip::sint >( sstride ),
                  tensorStride_,
                  porigin,
                  dt,
                  static_cast< dip::sint >( sstride ),
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
template< typename inT >
static inline void InternFill( Image& dest, inT v ) {
   DIP_THROW_IF( !dest.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint sstride_d;
   void* porigin_d;
   dest.GetSimpleStrideAndOrigin( sstride_d, porigin_d );
   if( porigin_d ) {
      // No need to loop
      FillBuffer(
            porigin_d,
            dest.DataType(),
            sstride_d,
            dest.TensorStride(),
            dest.NumberOfPixels(),
            dest.TensorElements(),
            v
      );
   } else {
      // Make nD loop
      dip::uint processingDim = Framework::OptimalProcessingDim( dest );
      auto it = GenericImageIterator( dest, processingDim );
      do {
         FillBuffer(
               it.Pointer(),
               dest.DataType(),
               dest.Stride( processingDim ),
               dest.TensorStride(),
               dest.Size( processingDim ),
               dest.TensorElements(),
               v
         );
      } while( ++it );
   }
}

void Image::Fill( bool v ) {
   InternFill( *this, static_cast< dip::sint >( v ));
}

void Image::Fill( int v ) {
   InternFill( *this, static_cast< dip::sint >( v ));
}

void Image::Fill( dip::uint v ) {
   InternFill( *this, clamp_cast< dip::sint >( v ));
   // `v` could potentially be clamped here, but:
   //  - it would be clamped anyway for any integer typed image.
   //  - if the image is float type, they should use the `dfloat` overload.
}

void Image::Fill( dip::sint v ) {
   InternFill( *this, v );
}

void Image::Fill( dfloat v ) {
   InternFill( *this, v );
}

void Image::Fill( dcomplex v ) {
   InternFill( *this, v );
}

// Casting the first sample (the first tensor component of the first pixel) to dcomplex.
template< typename TPI >
static inline dcomplex CastValueComplex( void* p ) {
   return clamp_cast< dcomplex >( *static_cast< TPI* >( p ));
}
Image::operator dcomplex() const {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   dcomplex x;
   DIP_OVL_CALL_ASSIGN_ALL( x, CastValueComplex, ( origin_ ), dataType_ );
   return x;
}

// Casting the first sample (the first tensor component of the first pixel) to dfloat.
template< typename TPI >
static inline dfloat CastValueDouble( void* p ) {
   return clamp_cast< dfloat >( *static_cast< TPI* >( p ));
}
Image::operator dfloat() const {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   dfloat x;
   DIP_OVL_CALL_ASSIGN_ALL( x, CastValueDouble, ( origin_ ), dataType_ );
   return x;
}

// Casting the first sample (the first tensor component of the first pixel) to sint.
template< typename TPI >
static inline dip::sint CastValueInteger( void* p ) {
   return clamp_cast< dip::sint >( *static_cast< TPI* >( p ));
}
Image::operator dip::sint() const {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::sint x;
   DIP_OVL_CALL_ASSIGN_ALL( x, CastValueInteger, ( origin_ ), dataType_ );
   return x;
}

} // namespace dip
