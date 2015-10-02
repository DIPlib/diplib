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
// Describes the shape of a tensor (doesn't contain any data)
//

class Tensor {

   public:

      // Possible shapes the tensor can have
      // We use the given ordering for symmetric and triangular matrices
      // because this makes it easy to extract the diagonal without having
      // to copy data (it's just a window over the full tensor).
      enum class Shape {
         COL_VECTOR,       // a vector                      // 0        (stores n elements)
                                                            // 1
                                                            // 2

         ROW_VECTOR,       // a row vector                  // 0 1 2    (stores n elements)

         COL_MAJOR_MATRIX, // a matrix                      // 0 3 6    (stores n x m elements)
                                                            // 1 4 7
                                                            // 2 5 8

         ROW_MAJOR_MATRIX, // a row-major matrix            // 0 1 2    (stores n x m elements)
                                                            // 3 4 5
                                                            // 6 7 8

         DIAGONAL_MATRIX,  // a diagonal matrix             // 0 x x    (stores n elements)
                                                            // x 1 x    (x = not stored, value = 0)
                                                            // x x 2

         SYMMETRIC_MATRIX, // a symmetric matrix            // 0 3 4    (stores n(n+1)/2 elements)
                                                            // 3 1 5
                                                            // 4 5 2

         UPPTRIANG_MATRIX, // an upper-triangular matrix    // 0 3 4    (stores n(n+1)/2 elements)
                                                            // x 1 5    (x = not stored, value = 0)
                                                            // x x 2

         LOWTRIANG_MATRIX, // a lower-triangular matrix     // 0 x x    (stores n(n+1)/2 elements)
                                                            // 3 1 x    (x = not stored, value = 0)
                                                            // 4 5 2
      };

      // Constructors
      Tensor() {
         SetScalar();
      }
      Tensor( uint n ) {
         SetVector( n );
      }
      Tensor( uint _rows, uint _cols ) {
         SetMatrix( _rows, _cols );
      }
      Tensor( Tensor::Shape _shape, uint _rows, uint _cols ) {
         SetShape( _shape, _rows, _cols );
      }

      // Test for shape
      // Feel free to add more here: IsMatrix, IsTriangular, IsRowMajor, IsColumnMajor, ...
      bool IsScalar()    const { return elements==1; }
      bool IsVector()    const { return (shape==Shape::COL_VECTOR) || (shape==Shape::ROW_VECTOR); }
      bool IsDiagonal()  const { return shape==Shape::DIAGONAL_MATRIX; }
      bool IsSymmetric() const { return shape==Shape::SYMMETRIC_MATRIX; }
      Shape GetShape()   const { return shape; }

      // Get tensor sizes
      uint Elements() const { return elements; }
      uint Rows()     const { return rows; }
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
      UnsignedArray Dimensions() const {
         if( IsScalar() ) {
            return {};
         } else if( IsVector() ) {
            return { elements };
         } else {
            return { rows, Columns() };
         }
      }

      // Set the tensor shape
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
      void SetScalar() {
         shape = Shape::COL_VECTOR;
         elements = rows = 1;
      }
      void SetVector( uint n ) {
         shape = Shape::COL_VECTOR;
         elements = rows = n;
      }
      void SetMatrix( uint _rows, uint _cols ) {
         shape = Shape::COL_MAJOR_MATRIX;
         elements = _rows * _cols;
         rows = _rows;
      }
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

      // Change the shape without changing the number of elements
      // (can be used after forging the image)
      void ChangeShape( uint _rows ) {
         if( rows != _rows ) {
            ThrowIf( elements % _rows, "Cannot reshape tensor to requested size" );
            rows = _rows;
            shape = Shape::COL_MAJOR_MATRIX;
         }
      }
      void ChangeShape() {
         shape = Shape::COL_VECTOR;
         elements = rows;
      }
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
