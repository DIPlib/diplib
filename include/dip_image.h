/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

// This file is included through diplib.h
#ifndef DIPLIB_H
#include "diplib.h"
#endif

#ifndef DIP_IMAGE_H
#define DIP_IMAGE_H

#include <memory>
#include <limits>

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
// the image object through Image::SetExternalInterface().
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

      // Empty (forged) image similar to src, but with different data type.
      Image( const Image& src, DataType dt ) :
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
         uint size;
         sint start;
         GetDataBlockSizeAndStart( size, start );
         origin = (uint8*)datablock.get() + start * dt.SizeOf();
      }

      // Creates an image with the external_interface set.
      explicit Image( ExternalInterface* ei ) : external_interface(ei) {}

      //
      // Dimensions
      //

      // TODO: We use the old DIPlib names here, I would prefer the
      // getters to have a name without "Get": img.Dimensionality(), img.Sizes(), img.Strides(), etc.

      // Get the number of spatial dimensions
      uint GetDimensionality() const {
         return dims.size();
      }

      // Get the spatial dimensions (image size)
      UnsignedArray GetDimensions() const {
         return dims;
      }

      // Get the number of pixels
      uint GetNumberOfPixels() const {
         uint n = 1;
         for( uint ii=0; ii<dims.size(); ++ii ) {
            ThrowIf( ( dims[ii] != 0 ) && ( n > std::numeric_limits<uint>::max() / dims[ii] ),
               E::DIMENSIONALITY_EXCEEDS_LIMIT );
            n *= dims[ii];
         }
         return n;
      }

      // Set the spatial dimensions (image size)
      void SetDimensions( const UnsignedArray& d ) {
         ThrowIf( IsForged(), E::IMAGE_NOT_RAW );
         dims = d;
      }

      // Permute dimensions
      // Example: {3,1} -> 3rd dimension becomes 1st, 1st dimension becomes 2nd,
      //                   2nd dimension is removed (only possible if dims[1]==1).
      Image& PermuteDimensions( const UnsignedArray& order );

      // Swap dimensions d1 and d2
      Image& SwapDimensions( uint d1, uint d2 );

      // Make image 1D, if !HasContiguousData(), data block will be copied
      Image& Flatten();

      // Removes singleton dimensions (dimensions with size==1)
      Image& Squeeze();

      // Adds a singleton dimension (with size==1), dimensions dim to
      // last are shifted up.
      // Example: an image with dims {4,5,6}, we add singleton dimension
      // dim=1, leaves the image with dims {4,1,5,6}.
      Image& AddSingleton( uint dim );

      // Appends singleton dimensions to increase the image dimensionality
      // to n. If the image already has n or more dimensions, nothing happens.
      Image& ExpandDimensionality( uint n );

      // Mirror de image about selected axes
      Image& Mirror( BooleanArray& process );

      //
      // Strides
      //

      IntegerArray GetStrides() const {
         return strides;
      }

      uint GetTensorStride() const {
         return tstride;
      }

      void SetStrides( const IntegerArray& s ) {
         ThrowIf( IsForged(), E::IMAGE_NOT_RAW );
         strides = s;
      }

      void SetTensorStride( sint ts ) {
         ThrowIf( IsForged(), E::IMAGE_NOT_RAW );
         tstride = ts;
      }

      // Test if all the pixels are contiguous (i.e. you can traverse
      // the whole image using a single stride==1.
      bool HasContiguousData() const {
         ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
         uint size = GetNumberOfPixels() * GetTensorElements();
         sint start;
         uint sz;
         GetDataBlockSizeAndStart( sz, start );
         return sz == size;
      }

      // Test if strides are as by default
      bool HasNormalStrides() const;

      // Test if the whole image can be travesed with a single stride value
      bool HasSimpleStride() const {
         void* p;
         uint s;
         GetSimpleStrideAndOrigin( s, p );
         return s>0;
      }

      // Return a pointer to the start of the data and a single stride to
      // walk through all pixels. If this is not possible, stride==0 and
      // porigin==nullptr.
      void GetSimpleStrideAndOrigin( uint& stride, void*& origin ) const;

      // Compute linear index given coordinates
      uint CoordinateToIndex( UnsignedArray& );

      // Compute coordinates given a linear index
      UnsignedArray IndexToCoordinate( uint );

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

      void ReshapeTensor( uint rows, uint cols ) { // Make into a matrix
         ThrowIf( tensor.Elements() != rows*cols, "Cannot reshape tensor to requested dimensions." );
         tensor.ChangeShape( rows );
      }
      void ReshapeTensorAsVector() {               // Make into a vector
         tensor.ChangeShape();
      }
      void Transpose() {                           // Transpose the tensor
         tensor.Transpose();
      }

      //
      // Data Type
      //

      DataType GetDataType() const {
         return datatype;
      }

      void SetDataType( DataType dt ) {
         ThrowIf( IsForged(), E::IMAGE_NOT_RAW );
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

      FloatArray PixelsToPhysicalDims( const FloatArray& ) const;
      FloatArray PhysicalDimsToPixels( const FloatArray& ) const;

      //
      // Utility functions
      //

      // Compares properties of an image against a template, either
      // returns true/false or throws an error.
      bool Compare( const Image&, bool error=true ) const;

      // Checks image properties, either returns true/false or throws an error.
      bool Check( const uint ndims, const DataType dt, bool error=true ) const;

      // Copy all image properties
      void CopyProperties( const Image& img ) {
         ThrowIf( IsForged(), E::IMAGE_NOT_RAW );
         datatype       = img.datatype;
         dims           = img.dims;
         strides        = img.strides;
         tensor         = img.tensor;
         color_space    = img.color_space;
         physdims       = img.physdims;
         if( !external_interface )
            external_interface = img.external_interface;
      }

      // Make this image similar to the template (except for the extenal interface)
      void Assimilate( const Image& img ) {
         Strip();
         CopyProperties( img );
         Forge();
      }

      // Does this image share its data pointer with another image?
      bool SharesData( const Image& other ) const {
         ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
         ThrowIf( !other.IsForged(), E::IMAGE_NOT_FORGED );
         return datablock == other.datablock;
      }
      bool SharesData() const {
         return !datablock.unique();
      }

      // Does writing in this image change the data of the other image?
      bool Aliases( const Image& other ) const;

      //
      // Data
      //

      // Get data pointer
      void* GetData() const {
         ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
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

      // Test if forged
      bool IsForged() const {
         if( origin )
            return true;
         else
            return false;
      }

      // Set external interface pointer
      void SetExternalInterface( ExternalInterface* ei ) {
         ThrowIf( IsForged(), E::IMAGE_NOT_RAW );
         external_interface = ei;
      }

      Image operator[]( const UnsignedArray& );       // Indexing in tensor dimensions

      Image operator[]( uint );                       // Indexing in tensor dimensions (linear indexing)

      Pixel at( const UnsignedArray& );               // Indexing in spatial dimensions

      Pixel at( uint );                               // Indexing in spatial dimensions (linear indexing)

      void Copy( Image& img );                        // Deep copy. 'this' will become a copy of 'img' with its own data.

      void ConvertDataType( Image&, DataType );       // Deep copy with data type conversion.

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
               // Arithmetic should be then able to do this operation in place if 'a' has the right
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
         // -> Implemented as call to Invert(in,out);

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
         // -> Implemented as call to Compare(in1,in2,out,"==");

//
// Utility functions
//

inline bool Alias( const Image& img1, const Image& img2 ) {
   return img1.Aliases( img2 );
}

} // namespace dip

#endif // DIP_IMAGE_H

