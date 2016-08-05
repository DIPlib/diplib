/*
 * DIPlib 3.0
 * This file contains definitions for the Tensor class and related functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


//
// NOTE!
// This file is included through diplib.h -- no need to include directly
//


#ifndef DIP_TENSOR_H
#define DIP_TENSOR_H

#include "dip_support.h"


/// \file
/// Defines the dip::Tensor class. This file is always included through diplib.h.


namespace dip {

//
// The Tensor class
//

/// Describes the shape of a tensor, but doesn't actually contain tensor
/// data.
/// Used internally by the dip::Image and dip::Pixel objects.
/// It is default-constructible, movable and copiable.
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
      /// Shape::ROW_MAJOR_MATRIX is its transpose.
      ///
      /// Shape::DIAGONAL_MATRIX stores only the diagonal elements.
      ///
      /// Shape::SYMMETRIC_MATRIX and Shape::UPPTRIANG_MATRIX store the
      /// values in the upper triangle only, as follows:
      ///
      ///      |0 4 5 6|
      ///      |x 1 7 8|
      ///      |x x 2 9|
      ///      |x x x 3|
      ///
      /// Here, `x` indicates values that are not stored.
      ///
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
      Tensor( dip::uint n ) {
         SetVector( n );
      }
      /// Creates a Shape::COL_MAJOR_MATRIX.
      Tensor( dip::uint rows, dip::uint cols ) {
         SetMatrix( rows, cols );
      }
      /// Constructor for arbitrary shape.
      Tensor( enum Shape shape, dip::uint rows, dip::uint cols ) {
         SetShape( shape, rows, cols );
      }

      /// Tests the tensor shape.
      bool IsScalar() const {
         return elements_==1;
      }
      /// Tests the tensor shape.
      bool IsVector() const {
         return (shape_==Shape::COL_VECTOR) || (shape_==Shape::ROW_VECTOR);
      }
      /// Tests the tensor shape.
      bool IsDiagonal() const {
         return shape_==Shape::DIAGONAL_MATRIX;
      }
      /// Tests the tensor shape.
      bool IsSymmetric() const {
         return shape_==Shape::SYMMETRIC_MATRIX;
      }
      /// Tests the tensor shape.
      bool IsTriangular() const {
         return (shape_==Shape::UPPTRIANG_MATRIX) || (shape_==Shape::LOWTRIANG_MATRIX);
      }
      /// Returns tensor shape.
      enum Shape Shape() const {
         return shape_;
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
               return elements_/rows_;
            case Shape::DIAGONAL_MATRIX:
            case Shape::SYMMETRIC_MATRIX:
            case Shape::UPPTRIANG_MATRIX:
            case Shape::LOWTRIANG_MATRIX:
               return rows_;        // these are all square matrices
          }
      }
      /// Gets the tensor size.
      UnsignedArray Dimensions() const {
         if( IsScalar() ) {
            return {};
         } else if( IsVector() ) {
            return { elements_ };
         } else {
            return { rows_, Columns() };
         }
      }

      /// Sets the tensor shape.
      void SetShape( enum Shape shape, dip::uint rows, dip::uint cols ) {
         shape_ = shape;
         dip_ThrowIf( rows==0, "Number of rows must be non-zero" );
         dip_ThrowIf( cols==0, "Number of columns must be non-zero" );
         switch( shape_ ) {
            case Shape::COL_VECTOR:
               dip_ThrowIf( cols!=1, "A column vector can have only one column" );
               elements_ = rows;
               rows_ = rows;
               break;
            case Shape::ROW_VECTOR:
               dip_ThrowIf( rows!=1, "A column vector can have only one column" );
               elements_ = cols;
               rows_ = 1;
               break;
            case Shape::COL_MAJOR_MATRIX:
            case Shape::ROW_MAJOR_MATRIX:
               elements_ = rows * cols;
               rows_ = rows;
               break;
            case Shape::DIAGONAL_MATRIX:
               dip_ThrowIf( rows!=cols, "A diagonal matrix must be square" );
               elements_ = rows;
               rows_ = rows;
               break;
            case Shape::SYMMETRIC_MATRIX:
               dip_ThrowIf( rows!=cols, "A symmetric matrix must be square" );
               elements_ = NUpperDiagonalElements( rows );
               rows_ = rows;
               break;
            case Shape::UPPTRIANG_MATRIX:
            case Shape::LOWTRIANG_MATRIX:
               dip_ThrowIf( rows!=cols, "A triangular matrix must be square" );
               elements_ = NUpperDiagonalElements( rows );
               rows_ = rows;
               break;
          }
      }
      /// Sets the tensor shape, results in a Shape::COL_VECTOR with one element (scalar).
      void SetScalar() {
         shape_ = Shape::COL_VECTOR;
         elements_ = rows_ = 1;
      }
      /// Sets the tensor shape, results in a Shape::COL_VECTOR.
      void SetVector( dip::uint n ) {
         shape_ = Shape::COL_VECTOR;
         elements_ = rows_ = n;
      }
      /// Sets the tensor shape, results in a Shape::COL_MAJOR_MATRIX.
      void SetMatrix( dip::uint rows, dip::uint cols ) {
         shape_ = Shape::COL_MAJOR_MATRIX;
         elements_ = rows * cols;
         rows_ = rows;
      }
      /// Sets the tensor size, always results in a Shape::COL_VECTOR or Shape::COL_MAJOR_MATRIX.
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
               dip_Throw( "Tensor dimensons higher than 2 not supported." );
         }
      }

      /// Changes the tensor shape without changing the number of elements, results in a Shape::COL_MAJOR_MATRIX.
      void ChangeShape( dip::uint rows ) {
         if( rows_ != rows ) {
            dip_ThrowIf( elements_ % rows, "Cannot reshape tensor to requested size" );
            rows_ = rows;
            shape_ = Shape::COL_MAJOR_MATRIX;
         }
      }
      /// Changes the tensor shape without changing the number of elements, results in a Shape::COL_VECTOR.
      void ChangeShape() {
         shape_ = Shape::COL_VECTOR;
         elements_ = rows_;
      }
      /// Changes the tensor shape without changing the number of elements, resulting in the shape described by `other`.
      void ChangeShape( const Tensor& other ) {
         dip_ThrowIf( elements_ != other.elements_, "Cannot reshape tensor to requested form" );
         shape_ = other.shape_;
         rows_ = other.rows_;
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

   private:

      enum Shape shape_ = Shape::COL_VECTOR;
      dip::uint elements_ = 1;
      dip::uint rows_ = 1;

      static inline dip::uint NUpperDiagonalElements( dip::uint rows ) {
         return ( rows * ( rows+1 ) ) / 2;
      }
};

} // namespace dip

#endif // DIP_TENSOR_H
