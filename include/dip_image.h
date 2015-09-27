/*
 * New DIPlib include file
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

// This file is included through diplib.h

#ifndef DIP_IMAGE_H
#define DIP_IMAGE_H

#include <memory>

namespace dip {


//
// Support functions that are needed in class Image
//

// Makes a new image object pointing to same pixel data as 'src', but with different origin, strides and size.
void DefineROI( Image& dest, const Image& src, const UnsignedArray& origin, const UnsignedArray& dims, const IntegerArray& spacing );

// Gateway to all the arithmetic functionality
inline void Arithmetic( const Image& lhs, const Image& rhs, Image& out, String op, DataType dt ) {}; // Should be defined elsewhere, the "inline" and "{}" here is to avoid a linker warning for now.



//
// Support for external interfaces:
// Software using DIPlib might want to control how pixel data is allocated.
//

// A class derived from this one will do all we need it to do. Assign into
// the image object through dip::Image::SetExternalInterface().
// The caller will maintain ownership of the interface!
class ExternalInterface {
   public:
      virtual std::shared_ptr<void> AllocateData(const UnsignedArray& dims,
                                                 IntegerArray& strides,
                                                 Tensor& tensor,
                                                 sint tstride,
                                                 DataType datatype) = 0;
};



//
// The Image class
//

class Image {

   public:

      //
      // Default constructor
      //

      Image() {}
      // NOTE: destructor, move constructor, copy assignment operator, and move assignment operator
      // should all be the default ones (Google for rule of zero).

      //
      // Other constructors
      //

      // Empty (forged) image of given sizes.
      explicit Image( UnsignedArray d, uint nchan = 1, DataType dt = DT_SFLOAT ) :
         datatype(dt),
         dims(d),
         tensor({nchan})
      {
         Forge();
      }

      // Empty (forged) image similar to src.
      explicit Image( const Image& src, DataType dt ) :
         datatype(dt),
         dims(src.dims),
         strides(src.strides),
         tensor(src.tensor),
         tstride(src.tstride),
         color_space(src.color_space),
         physdims(src.physdims),
         external_interface(src.external_interface)
      {
         Forge();
      }

      // Creates a new image pointing to data of 'src'.
      Image( const Image& src,
             const UnsignedArray& origin,
             const UnsignedArray& dims,
             const IntegerArray& spacing );

      // Creates a 0-D image with the value of 'p'.
      Image( double p, DataType dt = DT_SFLOAT ) :
         datatype(dt)
      {
         Forge();       // dims is empty by default
         // TODO: write data. There's sure to be a good way to do so later on.
      }

      // Creates an image around existing data.
      Image( std::shared_ptr<void> data,
             DataType dt,
             const UnsignedArray& d,            // dimensions
             const IntegerArray& s,             // strides
             const Tensor& t,                   // tensor properties
             sint ts ,                          // tensor stride
             ExternalInterface* ei ) :
         datatype(dt),
         dims(d),
         strides(s),
         tensor(t),
         tstride(ts),
         datablock(data),
         external_interface(ei)
      {
         origin = datablock.get();
         // TODO: call GetDataBlockSizeAndStart() to figure out where the origin should be.
      }

      //
      // Dimensions
      //

      uint GetDimensionality() const {
         return dims.size();
      }

      UnsignedArray GetDimensions() const {
         return dims;
      }

      uint GetNumberOfPixels() const {
         dip::uint n = 1;
         for( dip::uint ii=0; ii<dims.size(); ++ii ) {
            n *= dims[ii];
            // We should add a test here to make sure we don't get overflow in the computation.
         }
         return n;
      }

      void SetDimensions( const dip::UnsignedArray& d ) {
         DIPTS( IsForged(), dip::E::IMAGE_NOT_RAW );
         dims = d;
      }

      //
      // Strides
      //

      IntegerArray GetStrides() const {
         return strides;
      }
      uint GetTensorStride() const {
         return tstride;
      }

      bool HasContiguousData() const {
         DIPASSERT( IsForged(), dip::E::IMAGE_NOT_FORGED );
         dip::uint size = GetNumberOfPixels() * GetTensorElements();
         dip::sint start;
         dip::uint sz;
         GetDataBlockSizeAndStart( sz, start );
         return sz == size;
      }

      bool HasNormalStrides() const;

      void SetStrides( const IntegerArray& s ) {
         DIPTS( IsForged(), dip::E::IMAGE_NOT_RAW );
         strides = s;
      }
      void SetTensorStride( sint ts ) {
         DIPTS( IsForged(), dip::E::IMAGE_NOT_RAW );
         tstride = ts;
      }

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

      void SetTensorDimensions( const UnsignedArray& tdims ) {
         tensor.SetDimensions( tdims );
      }

      //
      // Data Type
      //

      DataType GetDataType() const {
         return datatype;
      }

      void SetDataType( DataType dt ) {
         DIPTS( IsForged(), dip::E::IMAGE_NOT_RAW );
         datatype = dt;
      }

      //
      // Color space
      //

      const ColorSpace& GetColorSpace() const;

      bool IsColor() const;

      void SetColorSpace( const ColorSpace& );

      void SetColorSpace( const String& );

      //
      // Physical dimensions
      //

      const PhysicalDimensions& GetPhysicalDimensions() const;

      void SetPhysicalDimensions( const PhysicalDimensions& );

      //
      // Utility functions
      //

      bool Compare( const Image&, bool error=true ) const;
                                       // Compares properties of an image against a template,
                                       // either returns true/false or throws an error.

      bool Check( const uint ndims, const DataType dt, bool error=true ) const;
                                       // Checks image properties, either returns true/false
                                       // or throws an error.

      // Copy all image properties except for the external interface
      void CopyProperties( const Image& img ) {
         DIPTS( IsForged(), dip::E::IMAGE_NOT_RAW );
         datatype       = img.datatype;
         dims           = img.dims;
         strides        = img.strides;
         tensor         = img.tensor;
         color_space    = img.color_space;
         physdims       = img.physdims;
      }

      // Make this image similar to the template (except for the extenal interface)
      void Assimilate( const Image& img ) {
         Strip();
         CopyProperties( img );
         Forge();
      }

      // Add functions to see if two image handles are the same, or if two images point to the same data

      //
      // Data
      //

      void* GetData() const {
         DIPASSERT( IsForged(), dip::E::IMAGE_NOT_FORGED );
         return origin;
      }

      // Allocate pixels
      void Forge();

      // Deallocate pixels
      void Strip() {
         if( IsForged() ) {
            datablock = nullptr; // Automatically frees old memory if no other pointers to it exist.
            origin = nullptr;    // Keep this one in sync!
         }
      }

      bool IsForged() const {
         if( origin )
            return true;
         else
            return false;
      }

      void SetExternalInterface( ExternalInterface* ei ) {
         DIPTS( IsForged(), dip::E::IMAGE_NOT_RAW );
         external_interface = ei;
      }

      //
      // Dimensionality manipulation
      //

      void PermuteDimensions( const UnsignedArray& );
                                 // {3,1} -> 3rd dimension becomes 1st, 1st dimension becomse 2nd,
                                 //          2nd dimension is removed (only possible if dims[1]==1).

      void Flatten();            // Make image 1D, if !HasContiguousData(), data block will
                                 //    be copied.

      void Squeeze();            // Removes singleton dimensions.

      //
      // Data manipulation
      //

      Image operator[]( const UnsignedArray& );       // Indexing in tensor dimensions

      Image operator[]( uint );                       // Indexing in tensor dimensions (linear indexing)

      Pixel at( const UnsignedArray& );               // Indexing in spatial dimensions

      Pixel at( uint );                               // Indexing in spatial dimensions (linear indexing)

      void Copy( Image& img );                        // Deep copy. 'this' will become a copy of 'img' with its own data.

      void ConvertDataType( Image&, DataType );       // Deep copy with data type conversion.

      uint CoordinateToIndex( UnsignedArray& );

      UnsignedArray IndexToCoordinate( uint );

      //
      // Operators
      //

      Image& operator+=( const Image& rhs ) {
         Arithmetic( *this, rhs, *this, "+", datatype );
         return *this;
      }
      Image& operator-=( const Image& rhs ) {
         Arithmetic( *this, rhs, *this, "-", datatype );
         return *this;
      }
      Image& operator*=( const Image& rhs ) {
         Arithmetic( *this, rhs, *this, "*", datatype );
         return *this;
      }
      Image& operator/=( const Image& rhs ) {
         Arithmetic( *this, rhs, *this, "/", datatype );
         return *this;
      }
      Image& operator%=( const Image& rhs ) {
         Arithmetic( *this, rhs, *this, "%", datatype );
         return *this;
      }
      Image& operator&=( const Image& rhs ) { // only for binary images?
         Arithmetic( *this, rhs, *this, "&", datatype );
         return *this;
      }
      Image& operator|=( const Image& rhs ) { // only for binary images?
         Arithmetic( *this, rhs, *this, "|", datatype );
         return *this;
      }
      Image& operator^=( const Image& rhs ) { // only for binary images?
         Arithmetic( *this, rhs, *this, "^", datatype );
         return *this;
      }
               // Implemented as calls to dip::Arithmetic(*this,in,*this,dip::Operation,dip::DataType);
               // dip::Arithmetic should be then able to do this operation in place if 'a' has the right
               // size to hold the result. Singleton expansion could cause this to not be the case.
               // Should there be an error if in-place operation is not possible?
               // a=a+b will resize 'a' and change its data type as necessary.

      friend std::ostream& operator<<( std::ostream&, const Image& );

   private:

      //
      // Implementation
      //

      DataType datatype = DT_SFLOAT;
      UnsignedArray dims;                 // dims.size == ndims
      IntegerArray strides;               // strides.size == ndims
      Tensor tensor;
      sint tstride;
      ColorSpace color_space;
      PhysicalDimensions physdims;
      std::shared_ptr<void> datablock;    // Holds the pixel data. Data block will be freed when last image
                                          //    that uses it is destroyed.
      void* origin = nullptr;             // Points to the origin ( pixel (0,0) ), not necessarily the first
                                          //    pixel of the data block.
      ExternalInterface* external_interface = nullptr;
                                          // A function that will be called instead of the default forge function.

      //
      // Some private functions
      //

      bool HasValidStrides() const {      // Are the two strides arrays of the same size as the dims arrays?
         if( dims.size() != strides.size() ) {
            return false;
         }
         return true;
      }

      void ComputeStrides();              // Fill in both strides arrays.

      void GetDataBlockSizeAndStart( uint& size, sint& start ) const;
                                          // size is the distance between top left and bottom right corners.
                                          // start is the distance between top left corner and origin
                                          // (will be <0 if any strides[ii] < 0). All measured in pixels.

}; // class Image

typedef std::vector<Image>  ImageArray;
typedef std::vector<Image&> ImageRefArray;


//
//Functions to work with image properties
//

bool ImagesCompare( const ImageArray&, bool throw_exception = true );
                                       // Compares properties of all images in array,
                                       // either returns true/false or throws an exception.
bool ImagesCheck( const ImageArray&, const uint ndims, const DataType dt, bool throw_exception = true );
                                       // Checks properties of all images in array,
                                       // either returns true/false or throws an exception.
void ImagesSeparate( const ImageArray& input, ImageArray& output );
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

void ImageChangeDataType( const Image& src, Image& dst );

//
// Overloaded operators
//

// Unary
Image operator-( const Image& );
Image operator~( const Image& ); // maybe not this one?
Image operator!( const Image& ); // only for binary images?
         // -> Implemented as call to dip::Invert(in,out);

// Arithmetic
inline Image operator+( const Image& lhs, const Image& rhs ) {
   Image out;
   Arithmetic( lhs, rhs, out, "+", DataTypeSuggest_Arithmetic( lhs, rhs ) );
   return out;
}
inline Image operator-( const Image& lhs, const Image& rhs ) {
   Image out;
   Arithmetic( lhs, rhs, out, "-", DataTypeSuggest_Arithmetic( lhs, rhs ) );
   return out;
}
inline Image operator*( const Image& lhs, const Image& rhs ) {
   Image out;
   Arithmetic( lhs, rhs, out, "*", DataTypeSuggest_Arithmetic( lhs, rhs ) );
   return out;
}
inline Image operator/( const Image& lhs, const Image& rhs ) {
   Image out;
   Arithmetic( lhs, rhs, out, "/", DataTypeSuggest_Arithmetic( lhs, rhs ) );
   return out;
}
inline Image operator%( const Image& lhs, const Image& rhs ) {
   Image out;
   Arithmetic( lhs, rhs, out, "%", DataTypeSuggest_Arithmetic( lhs, rhs ) );
   return out;
}
inline Image operator&( const Image& lhs, const Image& rhs ) { // only for binary images?
   Image out;
   Arithmetic( lhs, rhs, out, "&", DataTypeSuggest_Arithmetic( lhs, rhs ) );
   return out;
}
inline Image operator|( const Image& lhs, const Image& rhs ) { // only for binary images?
   Image out;
   Arithmetic( lhs, rhs, out, "|", DataTypeSuggest_Arithmetic( lhs, rhs ) );
   return out;
}
inline Image operator^( const Image& lhs, const Image& rhs ) { // only for binary images?
   Image out;
   Arithmetic( lhs, rhs, out, "^", DataTypeSuggest_Arithmetic( lhs, rhs ) );
   return out;
}

// Comparison
Image operator==( const Image&, const Image& );
Image operator!=( const Image&, const Image& );
Image operator< ( const Image&, const Image& );
Image operator> ( const Image&, const Image& );
Image operator<=( const Image&, const Image& );
Image operator>=( const Image&, const Image& );
         // -> Implemented as call to dip::Compare(in1,in2,out,"==");

} // namespace dip

#endif // DIP_IMAGE_H

