/*
 * New DIPlib include file
 * This file contains definitions for the Tensor and Pixel classes and related functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

// This file is included through diplib.h

#ifndef DIP_PIXEL_H
#define DIP_PIXEL_H


namespace dip {

//
// The Tensor class
// Describes the shape of a tensor (doesn't contain any data)
//

class Tensor {

   public:

      enum class Shape {
         COL_VECTOR,       // a vector
         ROW_VECTOR,       // a row vector
         COL_MAJOR_MATRIX, // a matrix
         ROw_MAJOR_MATRIX, // a row-major matrix
         DIAGONAL_MATRIX,  // a diagonal matrix
         SYMMETRIC_MATRIX, // a symmetric matrix
         UPPTRIANG_MATRIX, // an upper-triangular matrix
         LOWTRIANG_MATRIX, // a lower-triangular matrix
      };

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

      bool IsScalar()    const { return elements==1; }
      bool IsVector()    const { return (shape==Shape::COL_VECTOR) || (shape==Shape::ROW_VECTOR); }
      bool IsDiagonal()  const { return shape==Shape::DIAGONAL_MATRIX; }
      bool IsSymmetric() const { return shape==Shape::SYMMETRIC_MATRIX; }
      // Feel free to add more here: IsMatrix, IsTriangular, IsRowMajor, IsColumnMajor, ...
      Shape GetShape()   const { return shape; }

      uint Elements() const { return elements; }
      uint Rows()     const { return rows; }
      uint Columns()  const {
         switch( shape ) {
            case Shape::COL_VECTOR:
               return 1;
            case Shape::ROW_VECTOR:
               return elements;
            case Shape::COL_MAJOR_MATRIX:
            case Shape::ROw_MAJOR_MATRIX:
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

      void SetShape( Shape _shape, uint _rows, uint _cols ) {
         shape = _shape;
         DIPASSERT( _rows>0 , "Number of rows must be non-zero" );
         DIPASSERT( _cols>0 , "Number of columns must be non-zero" );
         switch( shape ) {
            case Shape::COL_VECTOR:
               DIPASSERT( _cols==1 , "A column vector can have only one column" );
               elements = _rows;
               rows = _rows;
               break;
            case Shape::ROW_VECTOR:
               DIPASSERT( _rows==1 , "A column vector can have only one column" );
               elements = _cols;
               rows = 1;
               break;
            case Shape::COL_MAJOR_MATRIX:
            case Shape::ROw_MAJOR_MATRIX:
               elements = _rows * _cols;
               rows = _rows;
               break;
            case Shape::DIAGONAL_MATRIX:
               DIPASSERT( _rows==_cols , "A diagonal matrix must be square" );
               elements = _rows;
               rows = _rows;
               break;
            case Shape::SYMMETRIC_MATRIX:
               DIPASSERT( _rows==_cols , "A symmetric matrix must be square" );
               elements = NUpperDiagonalElements( _rows );
               rows = _rows;
               break;
            case Shape::UPPTRIANG_MATRIX:
            case Shape::LOWTRIANG_MATRIX:
               DIPASSERT( _rows==_cols , "A triangular matrix must be square" );
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
               DIPSJ( "Tensor dimensons higher than 2 not supported." );
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

//
// The Pixel class
// Links to a pixel in an image object, and can be used to modify it
//

class Pixel {

   public:

      //
      // Default constructor
      //

      Pixel() {}
      // NOTE: destructor, move constructor, copy assignment operator, and move assignment operator
      // should all be the default ones (Google for rule of zero).

      //
      // Other constructors
      //

      // Construct a Pixel with all its data.
      //explicit Pixel( ... ) { ... } // TODO

      //
      // Tensor
      //

      UnsignedArray GetTensorDimensions() const {
         return tensor.Dimensions();
      }
      uint GetTensorElements() const {
         return tensor.Elements();
      }
      uint GetTensorColumns() const {
         return tensor.Columns();
      }
      uint GetTensorRows() const {
         return tensor.Rows();
      }
      bool IsScalar() const {
         return tensor.IsScalar();
      }
      bool IsVector() const {
         return tensor.IsVector();
      }

      //
      // Stide
      //

      sint GetTensorStride() const {
         return tstride;
      }

      //
      //
      // Data Type
      //

      DataType GetDataType() const {
         return datatype;
      }

      //
      // Color space
      //

      const ColorSpace& GetColorSpace() const;

      bool IsColor() const;

      //
      // Data
      //

      void* GetData() const {
         return origin;
      }

      //
      // Data manipulation
      //

      Pixel operator[]( const UnsignedArray& );       // Indexing in tensor dimensions

      Pixel operator[]( uint );                       // Indexing in tensor dimensions (linear indexing)

      //
      // Operators (modify the pixel value in the image)
      //

      Pixel& operator+=( const Pixel& rhs );
      Pixel& operator-=( const Pixel& rhs );
      Pixel& operator*=( const Pixel& rhs );
      Pixel& operator/=( const Pixel& rhs );
      Pixel& operator%=( const Pixel& rhs );
      Pixel& operator&=( const Pixel& rhs ); // only for binary images?
      Pixel& operator|=( const Pixel& rhs ); // only for binary images?
      Pixel& operator^=( const Pixel& rhs ); // only for binary images?

      Pixel& operator+=( double rhs );
      Pixel& operator-=( double rhs );
      Pixel& operator*=( double rhs );
      Pixel& operator/=( double rhs );
      Pixel& operator%=( double rhs );
      Pixel& operator&=( double rhs ); // only for binary images?
      Pixel& operator|=( double rhs ); // only for binary images?
      Pixel& operator^=( double rhs ); // only for binary images?

      Pixel& operator+=( int rhs );
      Pixel& operator-=( int rhs );
      Pixel& operator*=( int rhs );
      Pixel& operator/=( int rhs );
      Pixel& operator%=( int rhs );
      Pixel& operator&=( int rhs ); // only for binary images?
      Pixel& operator|=( int rhs ); // only for binary images?
      Pixel& operator^=( int rhs ); // only for binary images?

   private:

      //
      // Implementation
      //

      DataType datatype;
      Tensor tensor;                      // Once the constructor is written, this will be a reference
      sint tstride;
      ColorSpace color_space;             // Once the constructor is written, this will be a reference
      void* origin = nullptr;             // Points to the data inside the image object.

}; // class Pixel

//
// Overloaded operators
//

// Unary
Pixel operator-( const Pixel& );
Pixel operator~( const Pixel& ); // maybe not this one?
Pixel operator!( const Pixel& ); // only for binary images?
         // -> How to implement this??? We can only modify data inside an image!

// Comparison
bool operator==( const Pixel& lhs, const Pixel& rhs );
bool operator!=( const Pixel& lhs, const Pixel& rhs );
bool operator< ( const Pixel& lhs, const Pixel& rhs );
bool operator> ( const Pixel& lhs, const Pixel& rhs );
bool operator<=( const Pixel& lhs, const Pixel& rhs );
bool operator>=( const Pixel& lhs, const Pixel& rhs );

bool operator==( const Pixel& lhs, double rhs );
bool operator!=( const Pixel& lhs, double rhs );
bool operator< ( const Pixel& lhs, double rhs );
bool operator> ( const Pixel& lhs, double rhs );
bool operator<=( const Pixel& lhs, double rhs );
bool operator>=( const Pixel& lhs, double rhs );

bool operator==( const Pixel& lhs, int rhs );
bool operator!=( const Pixel& lhs, int rhs );
bool operator< ( const Pixel& lhs, int rhs );
bool operator> ( const Pixel& lhs, int rhs );
bool operator<=( const Pixel& lhs, int rhs );
bool operator>=( const Pixel& lhs, int rhs );

} // namespace dip

#endif // DIP_PIXEL_H

