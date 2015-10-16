/*
 * DIPlib 3.0
 * This file contains definitions for the Pixel class and related functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

// This file is included through diplib.h
#ifndef DIPLIB_H
#error "Please don't include this file directly, include diplib.h instead."
#endif

#ifndef DIP_PIXEL_H
#define DIP_PIXEL_H


namespace dip {

//
// The Pixel class
//

/// Links to a pixel in a dip::Image object, and can be used to modify it.
///
/// Indexing a single pixel in an image returns an object of type dip::Pixel.
/// It contains information on how to interpret the tensor data for that
/// pixel, and allows arithmetic and logical operations, and assignment.
///
/// It is not default-constructible, but it is movable and copiable.
///
/// A dip::Pixel can be cast to a dip::dfloat, a dip::sint or a
/// dip::dcomplex value. The first tensor value is taken. It is possible
/// to index into the pixel to obtain other tensor elements:
///
///     double x = static_cast<dfloat>( pixel[ 3 ] );
class Pixel {

   public:

      //
      // Constructors
      //

      //Pixel() = delete; // Is deleted by default, because we have reference members.
      // NOTE: destructor, move constructor, copy assignment operator, and move assignment operator
      // should all be the default ones (Google for rule of zero).

      /// Construct a Pixel with all its data; used by dip::Image.At() etc.
      Pixel( DataType dt, const Tensor& tens, sint tstr, const ColorSpace& colsp, void* orig ) :
         datatype(dt),
         tensor(tens),
         tstride(tstr),
         color_space(colsp),
         origin(orig) {}

      //
      // Tensor
      //

      /// Gets the tensor size.
      UnsignedArray TensorDimensions() const {
         return tensor.Dimensions();
      }
      /// Gets number of tensor elements.
      uint TensorElements() const {
         return tensor.Elements();
      }
      /// Gets number of tensor columns.
      uint TensorColumns() const {
         return tensor.Columns();
      }
      /// Gets number of tensor rows.
      uint TensorRows() const {
         return tensor.Rows();
      }
      /// Tests the tensor shape.
      bool IsScalar() const {
         return tensor.IsScalar();
      }
      /// Tests the tensor shape.
      bool IsVector() const {
         return tensor.IsVector();
      }
      /// Tests the tensor shape.
      bool IsDiagonal() const {
         return tensor.IsDiagonal();
      }
      /// Tests the tensor shape.
      bool IsSymmetric() const {
         return tensor.IsSymmetric();
      }
      /// Returns tensor shape.
      Tensor::Shape TensorShape() const {
         return tensor.GetShape();
      }

      /// Gets the tensor stride.
      sint TensorStride() const {
         return tstride;
      }

      //
      // Data type
      //

      /// Gets the data type.
      DataType GetDataType() const {
         return datatype;
      }

      //
      // Color space
      //

      /// Gets the color space information.
      const ColorSpace& GetColorSpace() const {
         return color_space;
      }

      /// Tests for color pixel.
      bool IsColor() const {
         return color_space.IsColor();
      }

      //
      // Data
      //

      /// Gets the data pointer.
      void* Data() const {
         return origin;
      }

      /// Returns a single tensor element using linear indexing.
      Pixel operator[]( uint n ) const {
         ThrowIf( n >= tensor.Elements(), E::PARAMETER_OUT_OF_RANGE );
         void* o = (uint8*)origin + n * tstride * datatype.SizeOf();
         return Pixel( datatype, Tensor(), tstride, color_space, o );
      }

      /// Extracts the fist value in the pixel, for complex values
      /// returns the absolute value.
      explicit operator sint() const;
      /// Extracts the fist value in the pixel, for complex values
      /// returns the absolute value.
      explicit operator dfloat() const;
      /// Extracts the fist value in the pixel.
      explicit operator dcomplex() const;

      /// Overloaded operator.
      Pixel& operator+=( const Pixel& rhs );
      /// Overloaded operator.
      Pixel& operator-=( const Pixel& rhs );
      /// Overloaded operator (matrix multiplication).
      Pixel& operator*=( const Pixel& rhs );
      /// Overloaded operator (`rhs` must be scalar).
      Pixel& operator/=( const Pixel& rhs );
      /// Overloaded operator (`rhs` must be scalar).
      Pixel& operator%=( const Pixel& rhs );
      /// Overloaded operator for binary-valued pixels.
      Pixel& operator&=( const Pixel& rhs );
      /// Overloaded operator for binary-valued pixels.
      Pixel& operator|=( const Pixel& rhs );
      /// Overloaded operator for binary-valued pixels.
      Pixel& operator^=( const Pixel& rhs );

      /// Overloaded operator.
      Pixel& operator+=( double rhs );
      /// Overloaded operator.
      Pixel& operator-=( double rhs );
      /// Overloaded operator.
      Pixel& operator*=( double rhs );
      /// Overloaded operator.
      Pixel& operator/=( double rhs );
      /// Overloaded operator.
      Pixel& operator%=( double rhs );
      /// Overloaded operator for binary-valued pixels.
      Pixel& operator&=( double rhs );
      /// Overloaded operator for binary-valued pixels.
      Pixel& operator|=( double rhs );
      /// Overloaded operator for binary-valued pixels.
      Pixel& operator^=( double rhs );

      /// Overloaded operator.
      Pixel& operator+=( int rhs );
      /// Overloaded operator.
      Pixel& operator-=( int rhs );
      /// Overloaded operator.
      Pixel& operator*=( int rhs );
      /// Overloaded operator.
      Pixel& operator/=( int rhs );
      /// Overloaded operator.
      Pixel& operator%=( int rhs );
      /// Overloaded operator for binary-valued pixels.
      Pixel& operator&=( int rhs );
      /// Overloaded operator for binary-valued pixels.
      Pixel& operator|=( int rhs );
      /// Overloaded operator for binary-valued pixels.
      Pixel& operator^=( int rhs );

   private:

      DataType datatype;
      const Tensor tensor;                // We keep a copy so we can modify it.
      sint tstride;
      const ColorSpace& color_space;
      void * origin;                      // Points to the data inside the image object.

}; // class Pixel

//
// Overloaded operators
//

/// Overloaded operator, both pixels must have the same number of elements.
bool operator==( const Pixel& lhs, const Pixel& rhs );
/// Overloaded operator, both pixels must have the same number of elements.
bool operator!=( const Pixel& lhs, const Pixel& rhs );

} // namespace dip

#endif // DIP_PIXEL_H
