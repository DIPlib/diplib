/*
 * DIPlib 3.0
 * This file contains definitions for the Pixel class and related functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

// This file is included through diplib.h
#ifndef DIPLIB_H
#include "diplib.h"
#endif

#ifndef DIP_PIXEL_H
#define DIP_PIXEL_H


namespace dip {

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

