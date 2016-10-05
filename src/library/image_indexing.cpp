/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"

namespace dip {

Image Image::operator[]( UnsignedArray const& indices ) const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint i = 0;
   dip::uint j = 0;
   switch( indices.size() ) {
      case 2:
         j = indices[ 1 ];
         // no break!
      case 1:
         i = indices[ 0 ];
         break;
      default: dip_Throw( E::ARRAY_ILLEGAL_SIZE );
   }
   dip::uint m = tensor_.Rows();
   dip::uint n = tensor_.Columns();
   dip_ThrowIf( ( i >= m ) || ( j >= n ), E::INDEX_OUT_OF_RANGE );
   switch( tensor_.Shape() ) {
      case Tensor::Shape::COL_VECTOR:
         break;
      case Tensor::Shape::ROW_VECTOR:
         i = j;
         break;
      case Tensor::Shape::ROW_MAJOR_MATRIX:
         std::swap( i, j );
         // no break!
      case Tensor::Shape::COL_MAJOR_MATRIX:
         i += j * m;
         break;
      case Tensor::Shape::DIAGONAL_MATRIX:
         dip_ThrowIf( i != j, E::INDEX_OUT_OF_RANGE );
         break;
      case Tensor::Shape::LOWTRIANG_MATRIX:
         std::swap( i, j );
         // no break!
      case Tensor::Shape::UPPTRIANG_MATRIX:
         dip_ThrowIf( i > j, E::INDEX_OUT_OF_RANGE );
         // no break!
      case Tensor::Shape::SYMMETRIC_MATRIX:
         if( i != j ) {
            // |0 4 5 6|     |0 1 2|
            // |x 1 7 8| --\ |x 3 4| (index + 4)
            // |x x 2 9| --/ |x x 5|
            // |x x x 3|
            if( i > j ) { std::swap( i, j ); }
            // we know: j >= 1
            dip::uint k = 0;
            for( dip::uint ii = 0; ii < i; ++ii ) {
               --j;
               --n;
               k += n;
            }
            --j;
            k += j;
            i = k + m;
         }
         break;
   }
   // Now `i` contains the linear index to the tensor element.
   Image out;
   out = *this;
   out.tensor_.SetScalar();
   out.origin_ = Pointer( i * tensorStride_ );
   out.ResetColorSpace();
   return out;
}

Image Image::operator[]( dip::uint index ) const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip_ThrowIf( index >= tensor_.Elements(), E::INDEX_OUT_OF_RANGE );
   Image out;
   out = *this;
   out.tensor_.SetScalar();
   out.origin_ = Pointer( index * tensorStride_ );
   out.ResetColorSpace();
   return out;
}

Image Image::Diagonal() const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   Image out;
   out = *this;
   if( tensor_.IsScalar() || tensor_.IsDiagonal() ) {
      out.tensor_.SetVector( tensor_.Elements() );
   } else if( tensor_.IsVector() ) {
      out.tensor_.SetScalar();                // Keep the first tensor element only
   } else if( tensor_.IsSymmetric() || tensor_.IsTriangular() ) {
      out.tensor_.SetVector( tensor_.Rows() );   // The diagonal elements are the first ones.
   } else { // matrix
      dip::uint m = tensor_.Rows();
      dip::uint n = tensor_.Columns();
      out.tensor_.SetVector( std::min( m, n ) );
      if( tensor_.Shape() == Tensor::Shape::COL_MAJOR_MATRIX ) {
         out.tensorStride_ = ( m + 1 ) * tensorStride_;
      } else { // row-major matrix
         out.tensorStride_ = ( n + 1 ) * tensorStride_;
      }
   }
   if( out.tensor_.Elements() != tensor_.Elements() ) {
      out.ResetColorSpace();
   }
   return out;
}

Image Image::At( UnsignedArray const& coords ) const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip_ThrowIf( coords.size() != sizes_.size(), E::ARRAY_ILLEGAL_SIZE );
   Image out;
   out = *this;
   out.sizes_.resize( 0 );
   out.strides_.resize( 0 );
   out.origin_ = Pointer( coords );
   return out;
}

Image Image::At( dip::uint index ) const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   if( sizes_.size() < 2 ) {
      dip::uint n = sizes_.size() == 0 ? 1 : sizes_[ 0 ];
      dip_ThrowIf( index >= n, E::INDEX_OUT_OF_RANGE );
      Image out;
      out = *this;
      out.sizes_.resize( 0 );
      out.strides_.resize( 0 );
      out.origin_ = Pointer( ( dip::sint )index );
      return out;
   } else {
      return At( IndexToCoordinates( index ) );
   }
}

Image Image::At( Range x_range ) const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip_ThrowIf( sizes_.size() != 1, E::ILLEGAL_DIMENSIONALITY );
   x_range.Fix( sizes_[ 0 ] );
   Image out;
   out = *this;
   out.sizes_[ 0 ] = x_range.Size();
   out.strides_[ 0 ] *= x_range.Step();
   out.pixelSize_.Scale( 0, x_range.Step() );
   out.origin_ = Pointer( x_range.Offset() * strides_[ 0 ] );
   return out;
}

Image Image::At( Range x_range, Range y_range ) const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip_ThrowIf( sizes_.size() != 2, E::ILLEGAL_DIMENSIONALITY );
   x_range.Fix( sizes_[ 0 ] );
   y_range.Fix( sizes_[ 1 ] );
   Image out;
   out = *this;
   out.sizes_[ 0 ] = x_range.Size();
   out.sizes_[ 1 ] = y_range.Size();
   out.strides_[ 0 ] *= x_range.Step();
   out.strides_[ 1 ] *= y_range.Step();
   out.pixelSize_.Scale( 0, x_range.Step() );
   out.pixelSize_.Scale( 1, y_range.Step() );
   out.origin_ = Pointer(
         x_range.Offset() * strides_[ 0 ] +
         y_range.Offset() * strides_[ 1 ] );
   return out;
}

Image Image::At( Range x_range, Range y_range, Range z_range ) const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip_ThrowIf( sizes_.size() != 3, E::ILLEGAL_DIMENSIONALITY );
   x_range.Fix( sizes_[ 0 ] );
   y_range.Fix( sizes_[ 1 ] );
   z_range.Fix( sizes_[ 2 ] );
   Image out;
   out = *this;
   out.sizes_[ 0 ] = x_range.Size();
   out.sizes_[ 1 ] = y_range.Size();
   out.sizes_[ 2 ] = z_range.Size();
   out.strides_[ 0 ] *= x_range.Step();
   out.strides_[ 1 ] *= y_range.Step();
   out.strides_[ 2 ] *= z_range.Step();
   out.pixelSize_.Scale( 0, x_range.Step() );
   out.pixelSize_.Scale( 1, y_range.Step() );
   out.pixelSize_.Scale( 2, z_range.Step() );
   out.origin_ = Pointer(
         x_range.Offset() * strides_[ 0 ] +
         y_range.Offset() * strides_[ 1 ] +
         z_range.Offset() * strides_[ 2 ] );
   return out;
}

Image Image::At( RangeArray ranges ) const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip_ThrowIf( sizes_.size() != ranges.size(), E::ARRAY_ILLEGAL_SIZE );
   for( dip::uint ii = 0; ii < sizes_.size(); ++ii ) {
      ranges[ ii ].Fix( sizes_[ ii ] );
   }
   Image out;
   out = *this;
   dip::sint offset = 0;
   for( dip::uint ii = 0; ii < sizes_.size(); ++ii ) {
      out.sizes_[ ii ] = ranges[ ii ].Size();
      out.strides_[ ii ] *= ranges[ ii ].Step();
      out.pixelSize_.Scale( ii, ranges[ ii ].Step() );
      offset += ranges[ ii ].Offset() * strides_[ ii ];
   }
   out.origin_ = Pointer( offset );
   return out;
}

Image Image::Real() const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip_ThrowIf( !dataType_.IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   Image out;
   out = *this;
   // Change data type
   out.dataType_ = dataType_ == DT_SCOMPLEX ? DT_SFLOAT : DT_DFLOAT;
   // Sample size is halved, meaning all strides must be doubled
   for( dip::uint ii = 0; ii < strides_.size(); ++ii ) {
      out.strides_[ ii ] *= 2;
   }
   out.tensorStride_ *= 2;
   return out;
}

Image Image::Imaginary() const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip_ThrowIf( !dataType_.IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   Image out;
   out = *this;
   // Change data type
   out.dataType_ = dataType_ == DT_SCOMPLEX ? DT_SFLOAT : DT_DFLOAT;
   // Sample size is halved, meaning all strides must be doubled
   for( dip::uint ii = 0; ii < strides_.size(); ++ii ) {
      out.strides_[ ii ] *= 2;
   }
   out.tensorStride_ *= 2;
   // Change the offset
   out.origin_ = out.Pointer( 1 );
   return out;
}

void DefineROI(
      Image const& src,
      Image& dest,
      UnsignedArray const& origin,
      UnsignedArray const& sizes,
      IntegerArray const& spacing
) {
   dip_ThrowIf( !dest.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint n = src.Dimensionality();
   dip_ThrowIf( origin.size() != n, E::ARRAY_ILLEGAL_SIZE );
   dip_ThrowIf( sizes.size() != n, E::ARRAY_ILLEGAL_SIZE );
   dip_ThrowIf( spacing.size() != n, E::ARRAY_ILLEGAL_SIZE );
   RangeArray ranges( n );
   for( dip::uint ii = 0; ii < n; ++ii ) {
      ranges[ ii ] = Range( origin[ ii ], sizes[ ii ] + origin[ ii ] - 1, spacing[ ii ] );
   }
   dest = src.At( ranges );
}

} // namespace dip
