/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

// This file is included through diplib.h
#ifndef DIPLIB_H
#error "Please don't include this file directly, include diplib.h instead."
#endif

#ifndef DIP_IMAGE_H
#define DIP_IMAGE_H

#include <memory>
#include <limits>

namespace dip {

//
// Support functions that are needed in class Image
//

// I hope this section can end up elsewhere. I don't like needing the
// forward declaration.

class Image;      // Forward declaration.

// Gateway to all the arithmetic functionality
// Maybe this should be many different functions. The old DIPlib had
// a single function here, but I'm not sure why.
inline void Arithmetic( const Image& lhs, const Image& rhs, Image& out, String op, DataType dt ) {} // Should be defined elsewhere, the "inline" and "{}" here is to avoid a linker warning for now.



//
// Support for external interfaces
//

/// Support for external interfaces. Software using DIPlib might want to
/// control how the image data is allocated. Such software should derive
/// a class from this one, and assign a pointer to it into each of the
/// images that it creates, through Image::SetExternalInterface().
/// The caller will maintain ownership of the interface.
class ExternalInterface {
   public:
      /// Allocates the data for an image.
      virtual std::shared_ptr<void> AllocateData(const UnsignedArray& dims,
                                                 IntegerArray& strides,
                                                 Tensor& tensor,
                                                 sint tstride,
                                                 DataType datatype) = 0;
};



//
// The Image class
//

// The class is documented in the file src/documentation/image.md
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

      /// Forged image of given sizes and data type.
      explicit Image( UnsignedArray d, uint nchan = 1, DataType dt = DT_SFLOAT ) :
         datatype(dt),
         dims(d),
         tensor(nchan)
      {
         Forge();
      }

      /// Forged image similar to `src`, but with different data type; the data is not copied.
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

      /// Create a 0-D image with the value of `p`.
      explicit Image( double p, DataType dt = DT_SFLOAT ) :
         datatype(dt)
      {
         Forge();       // dims is empty by default
         // TODO: write data. There's sure to be a good way to do so later on.
         // operator=( p );
      }

      /// Create an image around existing data.
      Image( std::shared_ptr<void> data,
             DataType dt,
             const UnsignedArray& d,            // dimensions
             const IntegerArray& s,             // strides
             const Tensor& t,                   // tensor properties
             sint ts,                           // tensor stride
             ExternalInterface* ei = nullptr ) :
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

      /// Creates an image with the ExternalInterface set.
      explicit Image( ExternalInterface* ei ) : external_interface(ei) {}

      //
      // Dimensions
      //

      /// Get the number of spatial dimensions.
      uint Dimensionality() const {
         return dims.size();
      }

      /// Get the spatial dimensions (image size).
      UnsignedArray Dimensions() const {
         return dims;
      }

      /// Get the number of pixels.
      uint NumberOfPixels() const {
         uint n = 1;
         for( uint ii=0; ii<dims.size(); ++ii ) {
            n *= dims[ii];
         }
         return n;
      }

      /// Set the spatial dimensions (image size); the image must be raw.
      void SetDimensions( const UnsignedArray& d ) {
         dip_ThrowIf( IsForged(), E::IMAGE_NOT_RAW );
         dims = d;
      }

      /// Permute dimensions. This function allows to re-arrange the dimensions
      /// of the image in any order. It also allows to remove singleton dimensions
      /// (but not to add them, should we add that? how?). For example, given
      /// an image with dimensions `{ 30, 1, 50 }`, and an `order` array of
      /// `{ 2, 0 }`, the image will be modified to have dimensions `{ 50, 30 }`.
      /// Dimension number 1 is not referenced, and was removed (this can only
      /// happen if the dimension has size 1, otherwise an exception will be
      /// thrown!). Dimension 2 was placed first, and dimension 0 was placed second.
      ///
      /// The image must be forged. If it is not, you can simply assign any
      /// new dimensions array through Image::SetDimensions. The data will never
      /// be copied (i.e. this is a quick and cheap operation).
      ///
      /// \see SwapDimensions, Squeeze, AddSingleton, ExpandDimensionality, Flatten.
      Image& PermuteDimensions( const UnsignedArray& order );

      /// Swap dimensions d1 and d2. This is a simplified version of the
      /// PermuteDimensions.
      ///
      /// The image must be forged, and the data will never
      /// be copied (i.e. this is a quick and cheap operation).
      ///
      /// \see PermuteDimensions.
      Image& SwapDimensions( uint d1, uint d2 );

      /// Make image 1D. The image must be forged. If HasContiguousData,
      /// this is a quick and cheap operation, but if not, the data segment
      /// will be copied.
      ///
      /// \see PermuteDimensions, ExpandDimensionality.
      Image& Flatten();

      /// Remove singleton dimensions (dimensions with size==1).
      /// The image must be forged, and the data will never
      /// be copied (i.e. this is a quick and cheap operation).
      ///
      /// \see AddSingleton, ExpandDimensionality, PermuteDimensions.
      Image& Squeeze();

      /// Add a singleton dimension (with size==1) to the image.
      /// Dimensions `dim` to last are shifted up, dimension `dim` will
      /// have a size of 1.
      ///
      /// The image must be forged, and the data will never
      /// be copied (i.e. this is a quick and cheap operation).
      ///
      /// Example: to an image with dimensions `{ 4, 5, 6 }` we add a
      /// singleton dimension `dim == 1`. The image will now have
      /// dimensions `{ 4, 1, 5, 6 }`.
      ///
      /// \see Squeeze, ExpandDimensionality, PermuteDimensions.
      Image& AddSingleton( uint dim );

      /// Append singleton dimensions to increase the image dimensionality.
      /// The image will have `n` dimensions. However, if the image already
      /// has `n` or more dimensions, nothing happens.
      ///
      /// The image must be forged, and the data will never
      /// be copied (i.e. this is a quick and cheap operation).
      ///
      /// \see AddSingleton, Squeeze, PermuteDimensions, Flatten.
      Image& ExpandDimensionality( uint n );

      /// Mirror de image about selected axes.
      /// The image must be forged, and the data will never
      /// be copied (i.e. this is a quick and cheap operation).
      Image& Mirror( const BooleanArray& process );

      //
      // Strides
      //

      /// Get the strides array.
      IntegerArray Strides() const {
         return strides;
      }

      /// Get the tensor stride.
      uint TensorStride() const {
         return tstride;
      }

      /// Set the strides array; the image must be raw.
      void SetStrides( const IntegerArray& s ) {
         dip_ThrowIf( IsForged(), E::IMAGE_NOT_RAW );
         strides = s;
      }

      /// Set the tensor stride; the image must be raw.
      void SetTensorStride( sint ts ) {
         dip_ThrowIf( IsForged(), E::IMAGE_NOT_RAW );
         tstride = ts;
      }

      /// Test if all the pixels are contiguous.
      /// If all pixels are contiguous, you can traverse the whole image,
      /// accessing each of the tensor elements in each of the pixles,
      /// using a single stride with a value of 1. To do so, you don't
      /// necessarily start at the origin; if any of the strides is
      /// negative, the origin of the contiguous data will be elsewhere.
      /// Use GetSimpleStrideAndOrigin to get a pointer to the origin
      /// of the contiguous data. Note that, in this case, the tensor
      /// values will be treated line one more spatial dimension.
      ///
      /// The image must be forged.
      /// /see GetSimpleStrideAndOrigin, HasSimpleStride, HasNormalStrides, Strides, TensorStride.
      bool HasContiguousData() const {
         dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
         uint size = NumberOfPixels() * TensorElements();
         sint start;
         uint sz;
         GetDataBlockSizeAndStart( sz, start );
         return sz == size;
      }

      /// Test if strides are as by default; the image must be forged.
      bool HasNormalStrides() const;

      /// Test if the whole image can be traversed with a single stride
      /// value. This is similar to HasContiguousData, but the stride
      /// value can be larger than 1.
      /// Use GetSimpleStrideAndOrigin to get a pointer to the origin
      /// of the contiguous data. Note that, in this case, the tensor
      /// values will be treated line one more spatial dimension.
      ///
      /// The image must be forged.
      /// /see GetSimpleStrideAndOrigin, HasContiguousData, HasNormalStrides, Strides, TensorStride.
      bool HasSimpleStride() const {
         void* p;
         uint s;
         GetSimpleStrideAndOrigin( s, p );
         return s>0;
      }

      /// Return a pointer to the start of the data and a single stride to
      /// walk through all pixels and all tensor elements.
      /// If this is not possible, the function sets `stride==0` and
      /// `porigin==nullptr`.
      ///
      /// The image must be forged.
      /// /see HasSimpleStride, HasContiguousData, HasNormalStrides, Strides, TensorStride, Data.
      void GetSimpleStrideAndOrigin( uint& stride, void*& origin ) const;

      /// Compute linear index (not memory offset) given coordinates.
      uint CoordinateToIndex( UnsignedArray& coords ) const;

      /// Compute coordinates given a linear index (not memory offset).
      UnsignedArray IndexToCoordinate( uint index ) const;

      //
      // Tensor
      //

      /// Get the tensor dimensions; the array returned can have 0, 1 or
      /// 2 elements, as those are the allowed tensor dimensionalities.
      UnsignedArray TensorDimensions() const {
         return tensor.Dimensions();
      }

      /// Get the number of tensor elements, the product of the elements
      /// in the array returned by TensorDimensions.
      uint TensorElements() const {
         return tensor.Elements();
      }

      /// Get the number of tensor columns.
      uint TensorColumns() const {
         return tensor.Columns();
      }

      /// Get the number of tensor rows.
      uint TensorRows() const {
         return tensor.Rows();
      }

      /// True for non-tensor (grey-value) images.
      bool IsScalar() const {
         return tensor.IsScalar();
      }

      /// True for vector images, where the tensor is one-dimensional.
      bool IsVector() const {
         return tensor.IsVector();
      }

      /// Set tensor dimensions; the image must be raw.
      void SetTensorDimensions( const UnsignedArray& tdims ) {
         dip_ThrowIf( IsForged(), E::IMAGE_NOT_RAW );
         tensor.SetDimensions( tdims );
      }

      /// Change the tensor shape, without changing the number of tensor elements.
      // NOTE: this currently forces the tensor to be a matrix. We should maybe
      // modify the Tensor class so that matrices with one of the dimensions==1
      // are marked as being vectors automatically. Thus setting the shape to
      // {1,3} will make this a row vector.
      void ReshapeTensor( uint rows, uint cols ) {
         dip_ThrowIf( tensor.Elements() != rows*cols, "Cannot reshape tensor to requested dimensions." );
         tensor.ChangeShape( rows );
      }

      /// Change the tensor to a vector, without changing the number of tensor elements.
      void ReshapeTensorAsVector() {
         tensor.ChangeShape();
      }

      /// Transpose the tensor.
      void Transpose() {
         tensor.Transpose();
      }

      //
      // Data Type
      //

      // From this point on, we must refer to the DataType type as
      // `struct DataType`, because we've hidden it with the declaration
      // of the function `DataType`.

      /// Get the image's data type.
      struct DataType DataType() const {
         return datatype;
      }

      /// Set the image's data type; the image must be raw.
      void SetDataType( struct DataType dt ) {
         dip_ThrowIf( IsForged(), E::IMAGE_NOT_RAW );
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

      /// Compare properties of an image against a template, either
      /// returns true/false or throws an error.
      bool Compare( const Image& src, bool error=true ) const;

      /// Check image properties, either returns true/false or throws an error.
      bool Check( const uint ndims, const struct DataType dt, bool error=true ) const;

      /// Copy all image properties from `src`; the image must be raw.
      void CopyProperties( const Image& src ) {
         dip_ThrowIf( IsForged(), E::IMAGE_NOT_RAW );
         datatype       = src.datatype;
         dims           = src.dims;
         strides        = src.strides;
         tensor         = src.tensor;
         color_space    = src.color_space;
         physdims       = src.physdims;
         if( !external_interface )
            external_interface = src.external_interface;
      }

      /// Make this image similar to the template by copying all its
      /// properties, but not the data.
      void Assimilate( const Image& src ) {
         Strip();
         CopyProperties( src );
         Forge();
      }

      /// Determine if this image shares any pixels with `other`.
      /// If `true`, writing into this image will change the data in
      /// `other`, and vice-versa.
      ///
      /// Both images must be forged.
      /// \see SharesData, Alias.
      bool Aliases( const Image& other ) const;

      //
      // Data
      //

      /// Get pointer to the data segment. This is useful to identify
      /// the data segment, but not to access the pixel data stored in
      /// it. Use Origin instead. The image must be forged.
      /// \see Origin, IsShared, ShareCount, SharesData.
      void* Data() const {
         dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
         return datablock.get();
      }

      /// Check to see if the data segment is shared with other images.
      /// The image must be forged.
      /// \see Data, ShareCount, SharesData.
      bool IsShared() const {
         dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
         return !datablock.unique();
      }

      /// Get the number of images that share their data with this image.
      /// The count is always at least 1. If the count is 1, IsShared is
      /// false. The image must be forged.
      /// \see Data, IsShared, SharesData.
      uint ShareCount() const {
         dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
         return datablock.use_count();
      }

      /// Determine if this image shares its data pointer with `other`.
      /// Both images must be forged.
      ///
      /// Note that sharing the data pointer
      /// does not imply that the two images share any pixel data, as it
      /// is possible for the two images to represent disjoint windows
      /// into the same data block. To determine if any pixels are shared,
      /// use Aliases.
      ///
      /// \see Aliases, Data, IsShared, ShareCount.
      bool SharesData( const Image& other ) const {
         dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
         dip_ThrowIf( !other.IsForged(), E::IMAGE_NOT_FORGED );
         return datablock == other.datablock;
      }

      /// Get pointer to the first pixel in the image, at coordinates (0,0,0,...);
      /// the image must be forged.
      void* Origin() const {
         dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
         return origin;
      }

      /// Allocate data segment. This function allocates a memory block
      /// to hold the pixel data. If the stride array is consistent with
      /// size array, and leads to a compact memory block, it is honored.
      /// Otherwise, it is ignored and a new stride array is created that
      /// leads to an image that HasNormalStrides.
      void Forge();

      /// Dissasociate the data segment from the image. If there are no
      /// other images using the same data segment, it will be freed.
      void Strip() {
         if( IsForged() ) {
            datablock = nullptr; // Automatically frees old memory if no other pointers to it exist.
            origin = nullptr;    // Keep this one in sync!
         }
      }

      /// Test if forged.
      bool IsForged() const {
         if( origin )
            return true;
         else
            return false;
      }

      /// Set external interface pointer; the image must be raw.
      void SetExternalInterface( ExternalInterface* ei ) {
         dip_ThrowIf( IsForged(), E::IMAGE_NOT_RAW );
         external_interface = ei;
      }

      /// Extract a tensor element, `indices` must have one or two elements; the image must be forged.
      Image operator[]( const UnsignedArray& indices ) const;

      /// Extract a tensor element using linear indexing; the image must be forged.
      Image operator[]( uint index ) const;

      /// Extracts the tensor elements along the diagonal; the image must be forged.
      Image Diagonal() const;

      /// Extracts the pixel at the given coordinates; the image must be forged.
      Image At( const UnsignedArray& coords ) const;

      /// Extracts the pixel at the given linear index; the image must be forged.
      Image At( uint index ) const;

      /// Extracts a subset of pixels from a 1D image; the image must be forged.
      Image At( Range x_range ) const;

      /// Extracts a subset of pixels from a 2D image; the image must be forged.
      Image At( Range x_range, Range y_range ) const;

      /// Extracts a subset of pixels from a 3D image; the image must be forged.
      Image At( Range x_range, Range y_range, Range z_range ) const;

      /// Extracts a subset of pixels from an image; the image must be forged.
      Image At( RangeArray ranges ) const;

      /// Deep copy, `this` will become a copy of `img` with its own data.
      void Copy( Image& img );

      /// Deep copy with data type conversion, `this` will become a copy of `img` with its own data.
      void ConvertDataType( Image&, struct DataType );   // TODO: should this be a method???

      /// Extracts the fist value in the first pixel (At(0,0)[0]), for complex values
      /// returns the absolute value.
      explicit operator sint() const;
      /// Extracts the fist value in the first pixel (At(0,0)[0]), for complex values
      /// returns the absolute value.
      explicit operator dfloat() const;
      /// Extracts the fist value in the first pixel (At(0,0)[0]).
      explicit operator dcomplex() const;


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

   private:

      //
      // Implementation
      //

      struct DataType datatype = DT_SFLOAT;
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

      bool HasValidStrides() const;       // Are the two strides arrays of the same size as the dims arrays?

      void ComputeStrides();              // Fill in both strides arrays.

      void GetDataBlockSizeAndStart( uint& size, sint& start ) const;
                                          // size is the distance between top left and bottom right corners.
                                          // start is the distance between top left corner and origin
                                          // (will be <0 if any strides[ii] < 0). All measured in pixels.

}; // class Image

typedef std::vector<Image>  ImageArray;
typedef std::vector<Image&> ImageRefArray;


//
// Functions to work with image properties
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

/// You can output an Image to `std::cout` or any other stream; some
/// information about the image is printed.
std::ostream& operator<<( std::ostream&, const Image& );

//
// Utility functions
//

/// Calls `img1.Aliases( img2 )`; see Image::Aliases.
inline bool Alias( const Image& img1, const Image& img2 ) {
   return img1.Aliases( img2 );
}

/// Makes a new image object pointing to same pixel data as `src`, but
/// with different origin, strides and size (backwards compatibility
/// function, we recommend the Image::At function instead).
void DefineROI(
   const Image& src,
   Image& dest,
   const UnsignedArray& origin,
   const UnsignedArray& dims,
   const IntegerArray& spacing );

} // namespace dip

#endif // DIP_IMAGE_H
