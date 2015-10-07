/*
 * DIPlib 3.0
 * This file contains definitions for the Tensor class and related functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

// This file is included through diplib.h
#ifndef DIPLIB_H
#include "diplib.h"
#endif

#ifndef DIP_TENSOR_H
#define DIP_TENSOR_H


namespace dip {

//
// The Tensor class
//

/// Describes the shape of a tensor, but doesn't actually contain tensor data.
/// Used internally by the dip::Image and dip::Pixel objects.
class Tensor {

   public:

      /// Possible shapes the tensor can have.
      ///
      /// Shape::COL_MAJOR_MATRIX is stored as follows:
      ///
      ///      |0 3 6|
      ///      |1 4 7|
      ///      |2 5 8|
      ///
      /// Shape::ROW_MAJOR_MATRIX is its transpose. Shape::DIAGONAL_MATRIX
      /// stores only the diagonal elements. Shape::SYMMETRIC_MATRIX and
      /// Shape::UPPTRIANG_MATRIX store the values in the upper triangle only,
      /// as follows:
      ///
      ///      |0 3 4|
      ///      |x 1 5|
      ///      |x x 2|
      ///
      /// Here, `x` indicates values that are not stored.
      /// Shape::LOWTRIANG_MATRIX is the transpose of Shape::UPPTRIANG_MATRIX.
      ///
      /// We use the given ordering for symmetric and triangular matrices
      /// because this makes it easy to extract the diagonal without having
      /// to copy data (it's just a window over the full tensor).
      enum class Shape {
         COL_VECTOR,       ///< a vector (stores n elements)
         ROW_VECTOR,       ///< a row vector (stores n elements)
         COL_MAJOR_MATRIX, ///< a matrix (stores n x m elements)
         ROW_MAJOR_MATRIX, ///< a row-major matrix (stores n x m elements)
         DIAGONAL_MATRIX,  ///< a diagonal matrix (stores n elements)
         SYMMETRIC_MATRIX, ///< a symmetric matrix (stores n(n+1)/2 elements)
         UPPTRIANG_MATRIX, ///< an upper-triangular matrix (stores n(n+1)/2 elements)
         LOWTRIANG_MATRIX, ///< a lower-triangular matrix (stores n(n+1)/2 elements)
      };

      /// Creates a Shape::COL_VECTOR with one element (scalar).
      Tensor() {
         SetScalar();
      }
      /// Creates a Shape::COL_VECTOR.
      Tensor( uint n ) {
         SetVector( n );
      }
      /// Creates a Shape::COL_MAJOR_MATRIX.
      Tensor( uint _rows, uint _cols ) {
         SetMatrix( _rows, _cols );
      }
      /// Constructor for arbitrary shape.
      Tensor( Tensor::Shape _shape, uint _rows, uint _cols ) {
         SetShape( _shape, _rows, _cols );
      }

      /// Tests the tensor shape
      bool IsScalar()    const { return elements==1; }
      /// Tests the tensor shape
      bool IsVector()    const { return (shape==Shape::COL_VECTOR) || (shape==Shape::ROW_VECTOR); }
      /// Tests the tensor shape
      bool IsDiagonal()  const { return shape==Shape::DIAGONAL_MATRIX; }
      /// Tests the tensor shape
      bool IsSymmetric() const { return shape==Shape::SYMMETRIC_MATRIX; }
      /// Returns tensor shape
      Shape GetShape()   const { return shape; }

      /// Gets number of tensor elements.
      uint Elements() const { return elements; }
      /// Gets number of tensor rows.
      uint Rows()     const { return rows; }
      /// Gets number of tensor columns.
      uint Columns()  const {
         switch( shape ) {
            case Shape::COL_VECTOR:
               return 1;
            case Shape::ROW_VECTOR:
               return elements;
            case Shape::COL_MAJOR_MATRIX:
            case Shape::ROW_MAJOR_MATRIX:
               return elements/rows;
            case Shape::DIAGONAL_MATRIX:
            case Shape::SYMMETRIC_MATRIX:
            case Shape::UPPTRIANG_MATRIX:
            case Shape::LOWTRIANG_MATRIX:
               return rows;         // these are all square matrices
          }
      }
      /// Gets the tensor size.
      UnsignedArray Dimensions() const {
         if( IsScalar() ) {
            return {};
         } else if( IsVector() ) {
            return { elements };
         } else {
            return { rows, Columns() };
         }
      }

      /// Sest the tensor shape.
      void SetShape( Shape _shape, uint _rows, uint _cols ) {
         shape = _shape;
         ThrowIf( _rows==0, "Number of rows must be non-zero" );
         ThrowIf( _cols==0, "Number of columns must be non-zero" );
         switch( shape ) {
            case Shape::COL_VECTOR:
               ThrowIf( _cols!=1, "A column vector can have only one column" );
               elements = _rows;
               rows = _rows;
               break;
            case Shape::ROW_VECTOR:
               ThrowIf( _rows!=1, "A column vector can have only one column" );
               elements = _cols;
               rows = 1;
               break;
            case Shape::COL_MAJOR_MATRIX:
            case Shape::ROW_MAJOR_MATRIX:
               elements = _rows * _cols;
               rows = _rows;
               break;
            case Shape::DIAGONAL_MATRIX:
               ThrowIf( _rows!=_cols, "A diagonal matrix must be square" );
               elements = _rows;
               rows = _rows;
               break;
            case Shape::SYMMETRIC_MATRIX:
               ThrowIf( _rows!=_cols, "A symmetric matrix must be square" );
               elements = NUpperDiagonalElements( _rows );
               rows = _rows;
               break;
            case Shape::UPPTRIANG_MATRIX:
            case Shape::LOWTRIANG_MATRIX:
               ThrowIf( _rows!=_cols, "A triangular matrix must be square" );
               elements = NUpperDiagonalElements( _rows );
               rows = _rows;
               break;
          }
      }
      /// Sest the tensor shape, results in a Shape::COL_VECTOR with one element (scalar).
      void SetScalar() {
         shape = Shape::COL_VECTOR;
         elements = rows = 1;
      }
      /// Sest the tensor shape, results in a Shape::COL_VECTOR.
      void SetVector( uint n ) {
         shape = Shape::COL_VECTOR;
         elements = rows = n;
      }
      /// Sest the tensor shape, results in a Shape::COL_MAJOR_MATRIX.
      void SetMatrix( uint _rows, uint _cols ) {
         shape = Shape::COL_MAJOR_MATRIX;
         elements = _rows * _cols;
         rows = _rows;
      }
      /// Sest the tensor size, always results in a Shape::COL_VECTOR or Shape::COL_MAJOR_MATRIX.
      void SetDimensions( const UnsignedArray& tdims ) {
         switch( tdims.size() ) {
            case 0:
               SetScalar();
               break;
            case 1:
               SetVector( tdims[0] );
               break;
            case 2:
               SetMatrix( tdims[0], tdims[1] );
               break;
            default:
               Throw( "Tensor dimensons higher than 2 not supported." );
         }
      }

      /// Changes the tensor shape without changing the number of elements, results in a Shape::COL_MAJOR_MATRIX.
      void ChangeShape( uint _rows ) {
         if( rows != _rows ) {
            ThrowIf( elements % _rows, "Cannot reshape tensor to requested size" );
            rows = _rows;
            shape = Shape::COL_MAJOR_MATRIX;
         }
      }
      /// Changes the tensor shape without changing the number of elements, results in a Shape::COL_VECTOR.
      void ChangeShape() {
         shape = Shape::COL_VECTOR;
         elements = rows;
      }
      /// Transposes the tensor, causing a change of shape without a change of number of elements.
      void Transpose() {
         switch( shape ) {
            case Shape::COL_VECTOR:
               shape = Shape::ROW_VECTOR;
               rows = 1;
               break;
            case Shape::ROW_VECTOR:
               shape = Shape::COL_VECTOR;
               rows = elements;
               break;
            case Shape::COL_MAJOR_MATRIX:
               shape = Shape::ROW_MAJOR_MATRIX;
               rows = elements / rows;
               break;
            case Shape::ROW_MAJOR_MATRIX:
               shape = Shape::COL_MAJOR_MATRIX;
               rows = elements / rows;
               break;
            case Shape::DIAGONAL_MATRIX:
            case Shape::SYMMETRIC_MATRIX:
               break;
            case Shape::UPPTRIANG_MATRIX:
               shape = Shape::LOWTRIANG_MATRIX;
               break;
            case Shape::LOWTRIANG_MATRIX:
               shape = Shape::UPPTRIANG_MATRIX;
               break;
          }
      }

   private:

      Shape shape = Shape::COL_VECTOR;
      uint elements = 1;
      uint rows = 1;

      static inline uint NUpperDiagonalElements( uint rows ) {
         return ( rows * ( rows+1 ) ) / 2;
      }
};

} // namespace dip

#endif // DIP_TENSOR_H
