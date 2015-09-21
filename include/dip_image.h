/*
 * New DIPlib include file
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIP_IMAGE_H
#define DIP_IMAGE_H

#include "dip_support.h"
#include <memory>

namespace dip {

class Image {

   public:

      // Default constructor
      Image() {}
      // NOTE: destructor, move constructor, copy assignment operator, and move assignment operator
      // should all be the default ones (Google for rule of zero).

      // Other constructors
      explicit Image( UnsignedArray dims, DataType dt = DataType::SFLOAT );
                                       // Empty image of given sizes.
      explicit Image( const Image & src, DataType dt );
                                       // Empty image of same size as 'src' image.
      Image( const Image & src,
             const UnsignedArray & origin,
             const UnsignedArray & dims,
             const IntegerArray & spacing );
                                       // Creates a new image pointing to data of 'src'.
      Image( double p, DataType dt = DataType::SFLOAT );
                                       // Creates a 0-D image with the value of 'p'.
      Image( std::shared_ptr<void> data, DataType datatype,
             const UnsignedArray & dims, const IntegerArray & strides,
             const UnsignedArray & tensor_dims, const IntegerArray & tensor_strides,
             void * interface );       // Creates an image around existing data.

      // Getters (dimensions and strides arrays are copied, other properties are passed by const reference)

      void * GetData() const;
      DataType GetDataType() const;
      uint GetDimensionality() const;
      UnsignedArray GetDimensions() const;
      uint GetNumberOfPixels() const;
      IntegerArray GetStrides() const;
      uint GetTensorDimensionality() const;
      UnsignedArray GetTensorDimensions() const;
      uint GetNumberOfTensorComponents() const;
      IntegerArray GetTensorStrides() const;
      const ColorSpace & GetColorSpace() const;
      const PhysicalDimensions & GetPhysicalDimensions() const;

      bool IsForged() const;
      bool HasContiguousData() const;
      bool HasNormalStrides() const;
      bool IsScalar() const;
      bool IsVector() const;
      bool IsColor() const;

      bool Compare( const Image &, bool error=true ) const;
                                       // Compares properties of an image against a template,
                                       // either returns true/false or throws an error.
      bool Check( const uint ndims, const DataType dt, bool error=true ) const;
                                       // Checks image properties, either returns true/false
                                       // or throws an error.

      // Setters

      void SetDataType( DataType );
      void SetDimensions( const UnsignedArray & );
      void SetStrides( const IntegerArray & );
      void SetTensorDimensions( const UnsignedArray & );
      void SetTensorStrides( const IntegerArray & );
      void SetColorSpace( const ColorSpace & );
      void SetColorSpace( const String & );
      void SetPhysicalDimensions( const PhysicalDimensions & );

      void Assimilate( const Image & );   // Same as Strip(); CopyProperties(); Forge();
      void CopyProperties( const Image & );

      void SetExternalInterface( ExternalInterface* );

      // manipulation

      void Forge();              // Allocate pixels
      void Strip();              // Deallocate pixels
      void PermuteDimensions( const UnsignedArray & );
                                 // {3,1} -> 3rd dimension becomes 1st, 1st dimension becomse 2nd,
                                 //          2nd dimension is removed (only possible if dims[1]==1).
      void Flatten();            // Make image 1D, if !HasContiguousData(), data block will
                                 //    be copied.
      void Squeeze();            // Removes singleton dimensions.
      Image & operator[]( const UnsignedArray & );    // Indexing in tensor dimensions
      Image & operator[]( uint );                     // Indexing in tensor dimensions (linear indexing)
      void Copy( Image & img ) const;  // Deep copy. 'this' will become a copy of 'img' with its own data.
      void ConvertDataType( Image &, DataType );      // Deep copy with data type conversion.

      // operators

      Image & operator+=( const Image & );
      Image & operator-=( const Image & );
      Image & operator*=( const Image & );
      Image & operator/=( const Image & );
      Image & operator%=( const Image & );
      Image & operator&=( const Image & );
      Image & operator|=( const Image & );
      Image & operator^=( const Image & );
               // Implemented as calls to dip::Arithmetic(*this,in,*this,dip::Operation,dip::DataType);
               // Note that a+=b should set the dip::DataType argument to a.DataType().
               // dip::Arithmetic should be then able to do this operation in place if 'a' has the right
               // size to hold the result. Singleton expansion could cause this to not be the case.
               // Should there be an error if in-place operation is not possible?
               // a=a+b would resize 'a' and change its data type if necessary.

      // other

      uint CoordinateToIndex( UnsignedArray & );
      UnsignedArray IndexToCoordinate( uint );

   private:
      DataType datatype = DataType::SFLOAT;
      UnsignedArray dims;                 // dims.size == ndims
      IntegerArray strides;               // strides.size == ndims
      UnsignedArray tensor_dims;          // tensor_dims.size == tensor_ndims
      IntegerArray tensor_strides;        // tensor_strides.size == tensor_ndims
      ColorSpace color_space;
      PhysicalDimensions physdims;
      std::shared_ptr<void> datablock;    // Holds the pixel data. Data block will be freed when last image
                                          //    that uses it is destroyed.
      void* origin = nullptr;             // Points to the origin ( pixel (0,0) ), not necessarily the first
                                          //    pixel of the data block.
      ExternalInterface* external_interface = nullptr;
                                          // A function that will be called instead of the default forge function.

      bool HasValidStrides() const;       // Are the two strides arrays of the same size as the dims arrays?
      void ComputeStrides();              // Fill in both strides arrays.
      void GetDataBlockSizeAndStart( uint & size, sint & start ) const;
                                          // size is the distance between top left and bottom right corners.
                                          // start is the distance between top left corner and origin
                                          // (will be <0 if any strides[ii] < 0). All measured in pixels.


      friend std::ostream & operator<<( std::ostream &, const Image & );
}; // class Image

typedef std::vector<Image> ImageArray;
typedef std::vector<Image&> ImageRefArray;


/*
 * Functions to work with image properties
 */

bool ImagesCompare( const ImageArray &, bool throw_exception = true );
                                       // Compares properties of all images in array,
                                       // either returns true/false or throws an exception.
bool ImagesCheck( const ImageArray &, const uint ndims, const DataType dt, bool throw_exception = true );
                                       // Checks properties of all images in array,
                                       // either returns true/false or throws an exception.
void ImagesSeparate( const ImageArray & input, ImageArray & output );
                                       // Makes sure none of the 'output' images are in the
                                       // 'input' list, and copies images if needed.
                                       // The idea is that, at function end, the modified
                                       // copies are the ones that are passed back to the
                                       // caller. We'll have to think about how exactly
                                       // this works. This is useful when a function is called
                                       // with the same image as input and output:
                                       // MyFunc(img,img); We want to do the processing in-place.
                                       // Some functions cannot work in-place, we need to make
                                       // a temporary output image tmp, and after the processing,
                                       // set img=tmp;

void ImageChangeDataType( const Image & src, Image & dst );

/*
 * Overloaded operators
 */

Image operator-( const Image & );
Image operator~( const Image & );
Image operator!( const Image & );
         // Implemented as call to dip::Invert(in,out);
Image operator+( const Image &, const Image & );
Image operator-( const Image &, const Image & );
Image operator*( const Image &, const Image & );
Image operator/( const Image &, const Image & );
Image operator%( const Image &, const Image & );
Image operator&( const Image &, const Image & );
Image operator|( const Image &, const Image & );
Image operator^( const Image &, const Image & );
         // Implemented as calls to dip::Arithmetic(in1,in2,out,dip::ArithmeticOperation,dip::DataType);
Image operator==( const Image &, const Image & );
Image operator!=( const Image &, const Image & );
Image operator< ( const Image &, const Image & );
Image operator> ( const Image &, const Image & );
Image operator<=( const Image &, const Image & );
Image operator>=( const Image &, const Image & );
         // -> Implemented as call to dip::Compare(in1,in2,out,dip::ComparisonOperation);

/*
 * Other image manipulation functions
 */

// Makes a new image object pointing to same pixel data as 'src', but with different origin, strides and size.
void DefineROI( Image & dest, const Image & src, const UnsignedArray & origin, const UnsignedArray & dims, const IntegerArray & spacing );

} // namespace dip

#endif // DIP_IMAGE_H

