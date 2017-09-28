/*
 * DIPlib 3.0
 * This file contains definitions for the Tensor class and related functions.
 *
 * (c)2014-2016, Cris Luengo.
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


//
// NOTE!
// This file is included through diplib.h -- no need to include directly
//


#ifndef DIP_TENSOR_H
#define DIP_TENSOR_H

#include "diplib/library/types.h"


/// \file
/// \brief The dip::Tensor class. This file is always included through `diplib.h`.
/// \see infrastructure


namespace dip {


/// \addtogroup infrastructure
/// \{


//
// The Tensor class
//

/// \brief Describes the shape of a tensor, but doesn't actually contain tensor data.
///
/// Used internally by the `dip::Image` objects.
/// It is default-constructible, movable and copiable.
class DIP_NO_EXPORT Tensor {

   public:

      /// \brief Possible shapes the tensor can have.
      ///
      /// `Shape::COL_MAJOR_MATRIX` is stored as follows:
      ///
      ///      |0 3 6|
      ///      |1 4 7|
      ///      |2 5 8|
      ///
      /// `Shape::ROW_MAJOR_MATRIX` is its transpose. These two shapes always have
      /// more than one column and row. A tensor with only one row or one column
      /// is a vector (`Shape::COL_VECTOR` or `Shape::ROW_VECTOR`).
      ///
      /// `Shape::DIAGONAL_MATRIX` stores only the diagonal elements.
      ///
      /// `Shape::SYMMETRIC_MATRIX` and `Shape::UPPTRIANG_MATRIX` store the
      /// values in the upper triangle only, as follows:
      ///
      ///      |0 4 5 7|
      ///      |x 1 6 8|
      ///      |x x 2 9|
      ///      |x x x 3|
      ///
      /// Here, `x` indicates values that are not stored.
      ///
      /// `Shape::LOWTRIANG_MATRIX` is the transpose of `Shape::UPPTRIANG_MATRIX`.
      ///
      /// We use the given ordering for symmetric and triangular matrices
      /// because this makes it easy to extract the diagonal without having
      /// to copy data (it's just a window over the full tensor). Because it
      /// is a little awkward finding the right elements given this ordering,
      /// the function `dip::Tensor::LookUpTable` prepares a table that can be used to access
      /// any tensor element given the row and column number. This function
      /// should help make more generic functions that can access tensor elements
      /// without paying attention to the tensor's Shape value.
      ///
      /// To access each of the elements of a symmetric or triangular matrix, use the
      /// following code:
      /// ```cpp
      ///     dip::uint index = 0;
      ///     for( dip::uint ii = 0; ii < nDims; ++ii ) { // Symmetric matrix stores diagonal elements first
      ///        // value at index * tensorStride is tensor element ( ii, ii ).
      ///        ++index;
      ///     }
      ///     for( dip::uint jj = 1; jj < nDims; ++jj ) { // Elements above diagonal stored column-wise
      ///        for( dip::uint ii = 0; ii < jj; ++ii ) {
      ///           // value at index * tensorStride is tensor element ( ii, jj ).
      ///           ++index;
      ///        }
      ///     }
      /// ```
      enum class Shape {
            COL_VECTOR = 0,   ///< a vector (stores n elements), default vector shape
            ROW_VECTOR,       ///< a row vector (stores n elements)
            COL_MAJOR_MATRIX, ///< a matrix (stores n x m elements), default matrix shape
            ROW_MAJOR_MATRIX, ///< a row-major matrix (stores n x m elements)
            DIAGONAL_MATRIX,  ///< a diagonal matrix (stores n elements)
            SYMMETRIC_MATRIX, ///< a symmetric matrix (stores n(n+1)/2 elements)
            UPPTRIANG_MATRIX, ///< an upper-triangular matrix (stores n(n+1)/2 elements)
            LOWTRIANG_MATRIX, ///< a lower-triangular matrix (stores n(n+1)/2 elements)
      };

      /// Creates a `Shape::COL_VECTOR` with one element (scalar).
      Tensor() {
         SetScalar();
      }
      /// Creates a `Shape::COL_VECTOR`.
      explicit Tensor( dip::uint n ) {
         SetVector( n );
      }
      /// Creates a `Shape::COL_MAJOR_MATRIX`.
      Tensor( dip::uint rows, dip::uint cols ) {
         SetMatrix( rows, cols );
      }
      /// Constructor for arbitrary shape.
      Tensor( Shape shape, dip::uint rows, dip::uint cols ) {
         SetShape( shape, rows, cols );
      }
      /// Constructor for arbitrary shape.
      Tensor( String const& shape, dip::uint rows, dip::uint cols ) {
         SetShape( ShapeFromString( shape ), rows, cols );
      }

      /// Tests the tensor shape.
      bool IsScalar() const {
         return elements_ == 1;
      }
      /// Tests the tensor shape.
      bool IsVector() const {
         return (( shape_ == Shape::COL_VECTOR ) || ( shape_ == Shape::ROW_VECTOR )) && ( elements_ > 1 );
      }
      /// Tests the tensor shape.
      bool IsDiagonal() const {
         return shape_ == Shape::DIAGONAL_MATRIX;
      }
      /// Tests the tensor shape.
      bool IsSymmetric() const {
         return shape_ == Shape::SYMMETRIC_MATRIX;
      }
      /// Tests the tensor shape.
      bool IsTriangular() const {
         return ( shape_ == Shape::UPPTRIANG_MATRIX ) || ( shape_ == Shape::LOWTRIANG_MATRIX );
      }
      /// True if the matrix is square, independently from how it is stored.
      bool IsSquare() const {
         switch( shape_ ) {
            case Shape::COL_VECTOR:
            case Shape::ROW_VECTOR:
               return elements_ == 1;
            case Shape::COL_MAJOR_MATRIX:
            case Shape::ROW_MAJOR_MATRIX:
               return elements_ == rows_ * rows_;
            case Shape::DIAGONAL_MATRIX:
            case Shape::SYMMETRIC_MATRIX:
            case Shape::UPPTRIANG_MATRIX:
            case Shape::LOWTRIANG_MATRIX:
               return true;        // these are all square matrices
         }
         // This should never happen:
         DIP_THROW( "Unknown tensor shape" );
      }
      /// Returns tensor shape.
      Shape TensorShape() const {
         return shape_;
      }
      /// Return a string representation of the tensor shape.
      String TensorShapeAsString() const {
         return ShapeToString( shape_ );
      }
      /// Gets number of tensor elements.
      dip::uint Elements() const {
         return elements_;
      }
      /// Gets number of tensor rows.
      dip::uint Rows() const {
         return rows_;
      }
      /// Gets number of tensor columns.
      dip::uint Columns() const {
         switch( shape_ ) {
            case Shape::COL_VECTOR:
               return 1;
            case Shape::ROW_VECTOR:
               return elements_;
            case Shape::COL_MAJOR_MATRIX:
            case Shape::ROW_MAJOR_MATRIX:
               return elements_ / rows_;
            case Shape::DIAGONAL_MATRIX:
            case Shape::SYMMETRIC_MATRIX:
            case Shape::UPPTRIANG_MATRIX:
            case Shape::LOWTRIANG_MATRIX:
               return rows_;        // these are all square matrices
         }
         // This should never happen:
         DIP_THROW( "Unknown tensor shape" );
      }
      /// Gets the tensor size.
      UnsignedArray Sizes() const {
         if( IsScalar() ) {
            return {};
         } else if( IsVector() ) {
            return { elements_ };
         } else {
            return { rows_, Columns() };
         }
      }

      /// Compares tensor size and shape.
      bool operator==( Tensor const& rhs )  const {
         return ( shape_ == rhs.shape_ ) &&
                ( elements_ == rhs.elements_ ) &&
                ( rows_ == rhs.rows_ );
      }

      /// Compares tensor size and shape.
      bool operator!=( Tensor const& rhs ) const {
         return !( *this == rhs );
      }

      /// Sets the tensor shape.
      void SetShape( Shape shape, dip::uint rows, dip::uint cols ) {
         shape_ = shape;
         DIP_THROW_IF( rows == 0, "Number of rows must be non-zero" );
         DIP_THROW_IF( cols == 0, "Number of columns must be non-zero" );
         switch( shape_ ) {
            case Shape::COL_VECTOR: DIP_THROW_IF( cols != 1, "A column vector can have only one column" );
               elements_ = rows;
               rows_ = rows;
               break;
            case Shape::ROW_VECTOR: DIP_THROW_IF( rows != 1, "A column vector can have only one column" );
               elements_ = cols;
               rows_ = 1;
               break;
            case Shape::COL_MAJOR_MATRIX:
            case Shape::ROW_MAJOR_MATRIX:
               elements_ = rows * cols;
               rows_ = rows;
               CorrectShape();
               break;
            case Shape::DIAGONAL_MATRIX: DIP_THROW_IF( rows != cols, "A diagonal matrix must be square" );
               elements_ = rows;
               rows_ = rows;
               break;
            case Shape::SYMMETRIC_MATRIX: DIP_THROW_IF( rows != cols, "A symmetric matrix must be square" );
               elements_ = NUpperDiagonalElements( rows );
               rows_ = rows;
               break;
            case Shape::UPPTRIANG_MATRIX:
            case Shape::LOWTRIANG_MATRIX: DIP_THROW_IF( rows != cols, "A triangular matrix must be square" );
               elements_ = NUpperDiagonalElements( rows );
               rows_ = rows;
               break;
         }
      }
      /// Sets the tensor shape, results in a `Shape::COL_VECTOR` with one element (scalar).
      void SetScalar() {
         shape_ = Shape::COL_VECTOR;
         elements_ = rows_ = 1;
      }
      /// Sets the tensor shape, results in a Shape::COL_VECTOR.
      void SetVector( dip::uint n ) {
         DIP_THROW_IF(( n == 0 ) || ( n > maxint ), "Sizes must be non-zero and no larger than " + std::to_string( maxint ));
         shape_ = Shape::COL_VECTOR;
         elements_ = rows_ = n;
      }
      /// Sets the tensor shape, results in a Shape::COL_MAJOR_MATRIX.
      void SetMatrix( dip::uint rows, dip::uint cols ) {
         DIP_THROW_IF( rows == 0, "Number of rows must be non-zero" );
         DIP_THROW_IF( cols == 0, "Number of columns must be non-zero" );
         shape_ = Shape::COL_MAJOR_MATRIX;
         elements_ = rows * cols;
         rows_ = rows;
         CorrectShape();
      }
      /// Sets the tensor size, always results in a `Shape::COL_VECTOR` or `Shape::COL_MAJOR_MATRIX`.
      void SetSizes( UnsignedArray const& sizes ) {
         switch( sizes.size() ) {
            case 0:
               SetScalar();
               break;
            case 1:
               SetVector( sizes[ 0 ] );
               break;
            case 2:
               SetMatrix( sizes[ 0 ], sizes[ 1 ] );
               break;
            default:
               DIP_THROW( "Tensor dimensionalities higher than 2 not supported" );
         }
      }

      /// Changes the tensor shape without changing the number of elements, results in a `Shape::COL_MAJOR_MATRIX`.
      void ChangeShape( dip::uint rows ) {
         if( rows_ != rows ) {
            DIP_THROW_IF( elements_ % rows, "Cannot reshape tensor to requested size" );
            rows_ = rows;
            shape_ = Shape::COL_MAJOR_MATRIX;
            CorrectShape();
         }
      }
      /// Changes the tensor shape without changing the number of elements, results in a `Shape::COL_VECTOR`.
      void ChangeShape() {
         shape_ = Shape::COL_VECTOR;
         rows_ = elements_;
      }
      /// Changes the tensor shape without changing the number of elements, resulting in the shape described by `example`.
      void ChangeShape( Tensor const& example ) {
         DIP_THROW_IF( elements_ != example.elements_, "Cannot reshape tensor to requested form" );
         shape_ = example.shape_;
         rows_ = example.rows_;
      }
      /// Transposes the tensor, causing a change of shape without a change of number of elements.
      void Transpose() {
         switch( shape_ ) {
            case Shape::COL_VECTOR:
               shape_ = Shape::ROW_VECTOR;
               rows_ = 1;
               break;
            case Shape::ROW_VECTOR:
               shape_ = Shape::COL_VECTOR;
               rows_ = elements_;
               break;
            case Shape::COL_MAJOR_MATRIX:
               shape_ = Shape::ROW_MAJOR_MATRIX;
               rows_ = elements_ / rows_;
               break;
            case Shape::ROW_MAJOR_MATRIX:
               shape_ = Shape::COL_MAJOR_MATRIX;
               rows_ = elements_ / rows_;
               break;
            case Shape::DIAGONAL_MATRIX:
            case Shape::SYMMETRIC_MATRIX:
               break;
            case Shape::UPPTRIANG_MATRIX:
               shape_ = Shape::LOWTRIANG_MATRIX;
               break;
            case Shape::LOWTRIANG_MATRIX:
               shape_ = Shape::UPPTRIANG_MATRIX;
               break;
         }
      }
      /// \brief Transforms the tensor such that it becomes a vector referencing the elements along the diagonal.
      /// The value of `stride` is adjusted.
      void ExtractDiagonal( dip::sint& stride ) {
         if( IsScalar() || IsDiagonal() ) {
            ChangeShape();                // The diagonal is all elements.
         } else if( IsVector() ) {
            SetScalar();                  // Keep the first tensor element only.
         } else if( IsSymmetric() || IsTriangular() ) {
            SetVector( rows_ );           // The diagonal elements are the first ones.
         } else { // matrix
            dip::uint m = rows_;
            dip::uint n = Columns();
            SetVector( std::min( m, n ) );
            if( shape_ == Tensor::Shape::COL_MAJOR_MATRIX ) {
               stride *= static_cast< dip::sint >( m + 1 );
            } else { // row-major matrix
               stride *= static_cast< dip::sint >( n + 1 );
            }
         }
      }
      /// \brief Transforms the tensor such that it becomes a vector referencing the elements along the given
      /// row. The value of `stride` is adjusted. The tensor representation must be full (i.e. no symmetric or
      /// triangular matrices).
      dip::sint ExtractRow( dip::uint index, dip::sint& stride ) {
         DIP_THROW_IF( index >= rows_, E::INDEX_OUT_OF_RANGE );
         dip::uint N = Columns();
         dip::sint offset = 0;
         switch( TensorShape() ) {
            case Tensor::Shape::COL_VECTOR:
            case Tensor::Shape::COL_MAJOR_MATRIX:
               offset = static_cast< dip::sint >( index ) * stride;
               stride *= static_cast< dip::sint >( rows_ );
               break;
            case Tensor::Shape::ROW_VECTOR:
            case Tensor::Shape::ROW_MAJOR_MATRIX:
               offset = static_cast< dip::sint >( index * N ) * stride;
               // stride doesn't change
               break;
            default:
               DIP_THROW( "Cannot obtain row for non-full tensor representation." );
         }
         SetShape( Tensor::Shape::ROW_VECTOR, 1, N );
         return offset;
      }
      /// \brief Transforms the tensor such that it becomes a vector referencing the elements along the given
      /// column. The value of `stride` is adjusted. The tensor representation must be full (i.e. no symmetric or
      /// triangular matrices).
      dip::sint ExtractColumn( dip::uint index, dip::sint& stride ) {
         dip::uint N = Columns();
         DIP_THROW_IF( index >= N, E::INDEX_OUT_OF_RANGE );
         dip::sint offset = 0;
         switch( TensorShape() ) {
            case Tensor::Shape::COL_VECTOR:
            case Tensor::Shape::COL_MAJOR_MATRIX:
               offset = static_cast< dip::sint >( index * rows_ ) * stride;
               // stride doesn't change
               break;
            case Tensor::Shape::ROW_VECTOR:
            case Tensor::Shape::ROW_MAJOR_MATRIX:
               offset = static_cast< dip::sint >( index ) * stride;
               stride *= static_cast< dip::sint >( N );
               break;
            default:
               DIP_THROW( "Cannot obtain row for non-full tensor representation." );
         }
         SetShape( Tensor::Shape::COL_VECTOR, rows_, 1 );
         return offset;
      }

      /// \brief Returns true for tensors that are stored in column-major order (all
      /// vectors and non-transposed full tensors).
      bool HasNormalOrder() const {
         switch( shape_ ) {
            case Shape::COL_VECTOR:
            case Shape::ROW_VECTOR:
            case Shape::COL_MAJOR_MATRIX:
               return true;
            default:
               return false;
         }
      }

      /// \brief Returns the linear index corresponding to the given tensor indices.
      ///
      /// Tensor element `(m,n)` can be found by adding `Tensor::Index({m,m}) * tstride`
      /// to the pixel's pointer. Throws if the indices do not point to a stored tensor
      /// element (for example, in a diagonal matrix, only the diagonal elements are
      /// stored; trying to access an off-diagonal element through `%Index` causes an
      /// exception to be thrown).
      ///
      /// \see dip::Tensor::LookUpTable
      dip::uint Index( UnsignedArray const& indices ) const {
         dip::uint i = 0;
         dip::uint j = 0;
         switch( indices.size() ) {
            case 2:
               j = indices[ 1 ];
               // no break!
            case 1:
               i = indices[ 0 ];
               break;
            default:
               DIP_THROW( E::ARRAY_ILLEGAL_SIZE );
         }
         dip::uint m = rows_;
         dip::uint n = Columns();
         DIP_THROW_IF(( i >= m ) || ( j >= n ), E::INDEX_OUT_OF_RANGE );
         switch( shape_ ) {
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
               DIP_THROW_IF( i != j, E::INDEX_OUT_OF_RANGE );
               break;
            case Tensor::Shape::LOWTRIANG_MATRIX:
               std::swap( i, j );
               // no break!
            case Tensor::Shape::UPPTRIANG_MATRIX:
               DIP_THROW_IF( i > j, E::INDEX_OUT_OF_RANGE );
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
         return i;
      }

      /// \brief Returns a look-up table that you can use to find specific tensor elements.
      ///
      /// Given a tensor with `M` rows and `N` columns, tensor element `(m,n)` can
      /// be found by adding `Tensor::LookUpTable()[n*M+m] * tstride` to the pixel's
      /// pointer. If the value in the look-up table is -1, the tensor element is
      /// not stored, and presumed to be 0 (happens with triangular and diagonal
      /// matrices only).
      ///
      std::vector< dip::sint > LookUpTable() const {
         dip::uint M = rows_;
         dip::uint N = Columns();
         std::vector< dip::sint > LUT( N * M, -1 );
         dip::sint index = 0;
         switch( shape_ ) {
            case Shape::COL_VECTOR:
            case Shape::ROW_VECTOR:
            case Shape::COL_MAJOR_MATRIX:
               for( dip::uint n = 0; n < N; ++n ) {
                  for( dip::uint m = 0; m < M; ++m ) {
                     LUT[ n * M + m ] = index;
                     ++index;
                  }
               }
               break;
            case Shape::ROW_MAJOR_MATRIX:
               for( dip::uint m = 0; m < M; ++m ) {
                  for( dip::uint n = 0; n < N; ++n ) {
                     LUT[ n * M + m ] = index;
                     ++index;
                  }
               }
               break;
            case Shape::DIAGONAL_MATRIX:
               for( dip::uint m = 0; m < M; ++m ) {
                  LUT[ m * M + m ] = index;
                  ++index;
               }
               break;
            case Shape::SYMMETRIC_MATRIX:
               for( dip::uint m = 0; m < M; ++m ) {
                  LUT[ m * M + m ] = index;
                  ++index;
               }
               for( dip::uint n = 1; n < N; ++n ) {
                  for( dip::uint m = 0; m < n; ++m ) {
                     LUT[ n * M + m ] = index;
                     LUT[ m * M + n ] = index;
                     ++index;
                  }
               }
               break;
            case Shape::UPPTRIANG_MATRIX:
               for( dip::uint m = 0; m < M; ++m ) {
                  LUT[ m * M + m ] = index;
                  ++index;
               }
               for( dip::uint n = 1; n < N; ++n ) {
                  for( dip::uint m = 0; m < n; ++m ) {
                     LUT[ n * M + m ] = index;
                     ++index;
                  }
               }
               break;
            case Shape::LOWTRIANG_MATRIX:
               for( dip::uint m = 0; m < M; ++m ) {
                  LUT[ m * M + m ] = index;
                  ++index;
               }
               for( dip::uint n = 1; n < N; ++n ) {
                  for( dip::uint m = 0; m < n; ++m ) {
                     LUT[ m * M + n ] = index;
                     ++index;
                  }
               }
               break;
         }
         return LUT;
      }

      /// Swaps the contents of `this` and `other`.
      void swap( Tensor& other ) {
         using std::swap;
         swap( shape_, other.shape_ );
         swap( elements_, other.elements_ );
         swap( rows_, other.rows_ );
      }

      /// Return a string representation of a tensor shape
      static String ShapeToString( Shape shape ) {
         switch( shape ) {
            default:
               //case Shape::COL_VECTOR:
               return "column vector";
            case Shape::ROW_VECTOR:
               return "row vector";
            case Shape::COL_MAJOR_MATRIX:
               return "column-major matrix";
            case Shape::ROW_MAJOR_MATRIX:
               return "row-major matrix";
            case Shape::DIAGONAL_MATRIX:
               return "diagonal matrix";
            case Shape::SYMMETRIC_MATRIX:
               return "symmetric matrix";
            case Shape::UPPTRIANG_MATRIX:
               return "upper triangular matrix";
            case Shape::LOWTRIANG_MATRIX:
               return "lower triangular matrix";
         }
      }

      /// Retrieve a tensor shape from a string representation of the tensor shape
      static Shape ShapeFromString( String const& string ) {
         if( string == "column vector" ) {
            return dip::Tensor::Shape::COL_VECTOR;
         } else if( string == "row vector" ) {
            return dip::Tensor::Shape::ROW_VECTOR;
         } else if( string == "column-major matrix" ) {
            return dip::Tensor::Shape::COL_MAJOR_MATRIX;
         } else if( string == "row-major matrix" ) {
            return dip::Tensor::Shape::ROW_MAJOR_MATRIX;
         } else if( string == "diagonal matrix" ) {
            return dip::Tensor::Shape::DIAGONAL_MATRIX;
         } else if( string == "symmetric matrix" ) {
            return dip::Tensor::Shape::SYMMETRIC_MATRIX;
         } else if( string == "upper triangular matrix" ) {
            return dip::Tensor::Shape::UPPTRIANG_MATRIX;
         } else if( string == "lower triangular matrix" ) {
            return dip::Tensor::Shape::LOWTRIANG_MATRIX;
         } else {
            DIP_THROW( String{ "TensorShape string not recognized: " } + string );
         }
      }

   private:

      Shape shape_ = Shape::COL_VECTOR;
      dip::uint elements_ = 1;
      dip::uint rows_ = 1;

      static inline dip::uint NUpperDiagonalElements( dip::uint rows ) {
         return ( rows * ( rows + 1 ) ) / 2;
      }

      // Only to be called if shape == COL_MAJOR_MATRIX or ROW_MAJOR_MATRIX.
      void CorrectShape() {
         if( rows_ == 1 ) {
            shape_ = Shape::ROW_VECTOR;
         } else if( rows_ == elements_ ) {
            shape_ = Shape::COL_VECTOR;
         }
      }
};

inline void swap( Tensor& v1, Tensor& v2 ) {
   v1.swap( v2 );
}

inline std::ostream& operator<<(
      std::ostream& os,
      Tensor const& tensor
) {
   if( tensor.IsScalar() ) {
      os << "scalar, 1 element";
   } else {
      os << tensor.Rows() << "x" << tensor.Columns() << " " << tensor.TensorShapeAsString() << ", " << tensor.Elements() << " elements";
   }
   return os;
}

/// \}

} // namespace dip

#endif // DIP_TENSOR_H
