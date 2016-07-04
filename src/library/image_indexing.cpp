/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"

namespace dip {

Image Image::operator[]( const UnsignedArray& indices ) const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint i = 0;
   dip::uint j = 0;
   switch (indices.size()) {
      case 2:
         j = indices[1];
         // no break!
      case 1:
         i = indices[0];
         break;
      default:
         dip_Throw( E::ARRAY_ILLEGAL_SIZE );
   }
   dip::uint m = tensor.Rows();
   dip::uint n = tensor.Columns();
   dip_ThrowIf( ( i >= m ) || ( j >= n ), E::INDEX_OUT_OF_RANGE );
   switch( tensor.Shape() ) {
      case Tensor::Shape::COL_VECTOR:
         break;
      case Tensor::Shape::ROW_VECTOR:
         i = j;
         break;
      case Tensor::Shape::ROW_MAJOR_MATRIX:
         std::swap( i, j );
         // no break!
      case Tensor::Shape::COL_MAJOR_MATRIX:
         i += j*m;
         break;
      case Tensor::Shape::DIAGONAL_MATRIX:
         dip_ThrowIf( i != j, E::INDEX_OUT_OF_RANGE );
         break;
      case Tensor::Shape::LOWTRIANG_MATRIX:
         std::swap( i, j );
         // no break!
      case Tensor::Shape::UPPTRIANG_MATRIX:
         dip_ThrowIf( i > j , E::INDEX_OUT_OF_RANGE );
         // no break!
      case Tensor::Shape::SYMMETRIC_MATRIX:
         if( i != j ) {
            // |0 4 5 6|     |0 1 2|
            // |x 1 7 8| --\ |x 3 4| (index + 4)
            // |x x 2 9| --/ |x x 5|
            // |x x x 3|
            if( i > j ) std::swap( i, j );
            // we know: j >= 1
            dip::uint k = 0;
            for( dip::uint ii = 0; ii<i; ++ii ) {
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
   Image out = *this;
   out.tensor.SetScalar();
   out.origin = Pointer( i * tstride );
   return out;
}

Image Image::operator[]( dip::uint index ) const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip_ThrowIf( index >= tensor.Elements() , E::INDEX_OUT_OF_RANGE );
   Image out = *this;
   out.tensor.SetScalar();
   out.origin = Pointer( index * tstride );
   return out;
}

Image Image::Diagonal() const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   if( tensor.IsScalar() || tensor.IsDiagonal() ) {
      return *this;
   } else if( tensor.IsVector() ) {
      Image out = *this;
      out.tensor.SetScalar();                // Keep the first tensor element only
      return out;
   } else if( tensor.IsSymmetric() || tensor.IsTriangular() ) {
      Image out = *this;
      out.tensor.SetVector(tensor.Rows());   // The diagonal elements are the first ones.
      return out;
   } else { // matrix
      Image out = *this;
      dip::uint m = tensor.Rows();
      dip::uint n = tensor.Columns();
      out.tensor.SetVector(std::min(m,n));
      if (tensor.Shape() == Tensor::Shape::COL_MAJOR_MATRIX) {
         out.tstride = (m+1)*tstride;
      } else { // row-major matrix
         out.tstride = (n+1)*tstride;
      }
      return out;
   }
}

Image Image::At( const UnsignedArray& coords ) const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip_ThrowIf( coords.size() != dims.size(), E::ARRAY_ILLEGAL_SIZE );
   Image out = *this;
   out.dims.resize( 0 );
   out.strides.resize( 0 );
   out.origin = Pointer( coords );
   return out;
}

Image Image::At( dip::uint index ) const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   if( dims.size() < 2 )
   {
      dip::uint n = dims.size()==0 ? 1 : dims[0];
      dip_ThrowIf( index >= n, E::INDEX_OUT_OF_RANGE );
      Image out = *this;
      out.dims.resize( 0 );
      out.strides.resize( 0 );
      out.origin = Pointer( (dip::sint)index );
      return out;
   }
   else
   {
      return At( IndexToCoordinates( index ) );
   }
}

Image Image::At( Range x_range ) const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip_ThrowIf( dims.size() != 1, E::ILLEGAL_DIMENSIONALITY );
   x_range.Fix( dims[0] );
   Image out = *this;
   out.dims[0] = x_range.Size();
   out.strides[0] *= x_range.Step();
   out.origin = Pointer( x_range.Offset() * strides[0] );
   return out;
}

Image Image::At( Range x_range, Range y_range ) const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip_ThrowIf( dims.size() != 2, E::ILLEGAL_DIMENSIONALITY );
   x_range.Fix( dims[0] );
   y_range.Fix( dims[1] );
   Image out = *this;
   out.dims[0] = x_range.Size();
   out.dims[1] = y_range.Size();
   out.strides[0] *= x_range.Step();
   out.strides[1] *= y_range.Step();
   out.origin = Pointer( x_range.Offset() * strides[0] +
                         y_range.Offset() * strides[1] );
   return out;
}

Image Image::At( Range x_range, Range y_range, Range z_range ) const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip_ThrowIf( dims.size() != 3, E::ILLEGAL_DIMENSIONALITY );
   x_range.Fix( dims[0] );
   y_range.Fix( dims[1] );
   z_range.Fix( dims[2] );
   Image out = *this;
   out.dims[0] = x_range.Size();
   out.dims[1] = y_range.Size();
   out.dims[2] = z_range.Size();
   out.strides[0] *= x_range.Step();
   out.strides[1] *= y_range.Step();
   out.strides[2] *= z_range.Step();
   out.origin = Pointer( x_range.Offset() * strides[0] +
                         y_range.Offset() * strides[1] +
                         z_range.Offset() * strides[2] );
   return out;
}

Image Image::At( RangeArray ranges ) const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip_ThrowIf( dims.size() != ranges.size(), E::ARRAY_ILLEGAL_SIZE );
   for( dip::uint ii = 0; ii < dims.size(); ++ii ) {
      ranges[ii].Fix( dims[ii] );
   }
   Image out = *this;
   dip::sint offset = 0;
   for( dip::uint ii = 0; ii < dims.size(); ++ii ) {
      out.strides[ii] *= ranges[ii].Step();
      out.dims[ii] = ranges[ii].Size();
      offset += ranges[ii].Offset() * strides[ii];
   }
   out.origin = Pointer( offset );
   return out;
}

void DefineROI(
   const Image& src,
   Image& dest,
   const UnsignedArray& origin,
   const UnsignedArray& dims,
   const IntegerArray& spacing
) {
   dip_ThrowIf( !dest.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint n = src.Dimensionality();
   dip_ThrowIf( origin.size()  != n , E::ARRAY_ILLEGAL_SIZE );
   dip_ThrowIf( dims.size()    != n , E::ARRAY_ILLEGAL_SIZE );
   dip_ThrowIf( spacing.size() != n , E::ARRAY_ILLEGAL_SIZE );
   RangeArray ranges( n );
   for( dip::uint ii=0; ii<n; ++ii ) {
      ranges[ii] = Range( origin[ii], dims[ii]+origin[ii]-1, spacing[ii] );
   }
   dest = src.At( ranges );
}

} // namespace dip
