/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


//
// NOTE!
// This file is included through diplib.h -- no need to include directly
//


#ifndef DIP_IMAGE_H
#define DIP_IMAGE_H

#include <memory>
#include <functional>

#include "dip_datatype.h"
#include "dip_tensor.h"
#include "dip_physdims.h"
#include "dip_clamp_cast.h"


/// \file
/// Defines the dip::Image class and support functions. This file is always included through diplib.h.


/// The dip namespace contains all the library functionality.
namespace dip {


//
// Support for external interfaces
//

/// Support for external interfaces. Software using DIPlib might want to
/// control how the image data is allocated. Such software should derive
/// a class from this one, and assign a pointer to it into each of the
/// images that it creates, through Image::SetExternalInterface().
/// The caller will maintain ownership of the interface.
///
/// See dip_matlab.h for an example of how to create an ExternalInterface.
class ExternalInterface {
   public:
      /// Allocates the data for an image. The function is free to modify
      /// `strides` and `tstride` if desired, though they will be set
      /// to the normal values by the calling function.
      virtual std::shared_ptr<void> AllocateData(const UnsignedArray& dims,
                                                 IntegerArray& strides,
                                                 const dip::Tensor& tensor,
                                                 dip::sint& tstride,
                                                 dip::DataType datatype) = 0;
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
      explicit Image( UnsignedArray dimensions, dip::uint tensorElems = 1, dip::DataType dt = DT_SFLOAT ) :
         datatype( dt ),
         dims( dimensions ),
         tensor( tensorElems )
      {
         Forge();
      }

      /// Forged image similar to `src`, but with different data type; the data is not copied.
      Image( const Image& src, dip::DataType dt ) :
         datatype( dt ),
         dims( src.dims ),
         strides( src.strides ),
         tensor( src.tensor ),
         tstride( src.tstride ),
         colspace( src.colspace ),
         pixelsize( src.pixelsize ),
         externalInterface( src.externalInterface )
      {
         Forge();
      }

      /// Create a 0-D image with the value and data type of `p`.
      template< typename T >
      explicit Image( T p ) {
         datatype = dip::DataType( p );
         Forge();       // dims is empty by default
         *(( T* )origin ) = p;
      }

      /// Create a 0-D image with the value of `p` and the given data type.
      template< typename T >
      Image( T p, dip::DataType dt ) {
         datatype = dt;
         Forge();       // dims is empty by default
         switch( datatype ) {
            case dip::DT_BIN      : ( bin*      )origin = clamp_cast<bin>     ( p ); break;
            case dip::DT_UINT8    : ( uint8*    )origin = clamp_cast<uint8>   ( p ); break;
            case dip::DT_UINT16   : ( uint16*   )origin = clamp_cast<uint16>  ( p ); break;
            case dip::DT_UINT32   : ( uint32*   )origin = clamp_cast<uint32>  ( p ); break;
            case dip::DT_SINT8    : ( sint8*    )origin = clamp_cast<sint8>   ( p ); break;
            case dip::DT_SINT16   : ( sint16*   )origin = clamp_cast<sint16>  ( p ); break;
            case dip::DT_SINT32   : ( sint32*   )origin = clamp_cast<sint32>  ( p ); break;
            case dip::DT_SFLOAT   : ( sfloat*   )origin = clamp_cast<sfloat>  ( p ); break;
            case dip::DT_DFLOAT   : ( dfloat*   )origin = clamp_cast<dfloat>  ( p ); break;
            case dip::DT_SCOMPLEX : ( scomplex* )origin = clamp_cast<scomplex>( p ); break;
            case dip::DT_DCOMPLEX : ( dcomplex* )origin = clamp_cast<dcomplex>( p ); break;
            default: dip_Throw( dip::E::DATA_TYPE_NOT_SUPPORTED );
         }
      }

      /// Create an image around existing data.
      Image( std::shared_ptr<void> data,        // points at the data block, not necessarily the origin!
             dip::DataType dt,
             const UnsignedArray& d,            // dimensions
             const IntegerArray& s,             // strides
             const dip::Tensor& t,              // tensor properties
             dip::sint ts,                      // tensor stride
             ExternalInterface* ei = nullptr ) :
         datatype( dt ),
         dims( d ),
         strides( s ),
         tensor( t ),
         tstride( ts ),
         datablock( data ),
         externalInterface( ei )
      {
         dip::uint size;
         dip::sint start;
         GetDataBlockSizeAndStartWithTensor( size, start );
         origin = (uint8*)datablock.get() + start * dt.SizeOf();
      }

      //
      // Dimensions
      //

      /// Get the number of spatial dimensions.
      dip::uint Dimensionality() const {
         return dims.size();
      }

      /// Get a const reference to the dimensions array (image size).
      const UnsignedArray& Dimensions() const {
         return dims;
      }

      /// Get the dimension along a specific dimension.
      dip::uint Dimension( dip::uint dim ) const {
         return dims[dim];
      }

      /// Get the number of pixels.
      dip::uint NumberOfPixels() const {
         dip::uint n = 1;
         for( dip::uint ii=0; ii<dims.size(); ++ii ) {
            n *= dims[ii];
         }
         return n;
      }

      /// Set the spatial dimensions (image size); the image must be raw.
      void SetDimensions( const UnsignedArray& d ) {
         dip_ThrowIf( IsForged(), E::IMAGE_NOT_RAW );
         dims = d;
      }

      //
      // Strides
      //

      /// Get a const reference to the strides array.
      const IntegerArray& Strides() const {
         return strides;
      }

      /// Get the stride along a specific dimension.
      dip::sint Stride( dip::uint dim ) const {
         return strides[dim];
      }

      /// Get the tensor stride.
      dip::uint TensorStride() const {
         return tstride;
      }

      /// Set the strides array; the image must be raw.
      void SetStrides( const IntegerArray& s ) {
         dip_ThrowIf( IsForged(), E::IMAGE_NOT_RAW );
         strides = s;
      }

      /// Set the tensor stride; the image must be raw.
      void SetTensorStride( dip::sint ts ) {
         dip_ThrowIf( IsForged(), E::IMAGE_NOT_RAW );
         tstride = ts;
      }

      /// Test if all the pixels are contiguous.
      /// If all pixels are contiguous, you can traverse the whole image,
      /// accessing each of the pixles, using a single stride with a value
      /// of 1. To do so, you don't necessarily start at the origin; if any
      /// of the strides is negative, the origin of the contiguous data will
      /// be elsewhere.
      /// Use GetSimpleStrideAndOrigin to get a pointer to the origin
      /// of the contiguous data.
      ///
      /// The image must be forged.
      /// \see GetSimpleStrideAndOrigin, HasSimpleStride, HasNormalStrides, Strides, TensorStride.
      bool HasContiguousData() const {
         dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
         dip::uint size = NumberOfPixels() * TensorElements();
         dip::sint start;
         dip::uint sz;
         GetDataBlockSizeAndStartWithTensor( sz, start );
         return sz == size;
      }

      /// Test if strides are as by default; the image must be forged.
      bool HasNormalStrides() const;

      /// Test if the whole image can be traversed with a single stride
      /// value. This is similar to HasContiguousData, but the stride
      /// value can be larger than 1.
      /// Use GetSimpleStrideAndOrigin to get a pointer to the origin
      /// of the contiguous data. Note that this only tests spatial
      /// dimesions, the tensor dimension must still be accessed separately.
      ///
      /// The image must be forged.
      /// \see GetSimpleStrideAndOrigin, HasContiguousData, HasNormalStrides, Strides, TensorStride.
      bool HasSimpleStride() const {
         void* p;
         dip::uint s;
         GetSimpleStrideAndOrigin( s, p );
         return p != nullptr;
      }

      /// Return a pointer to the start of the data and a single stride to
      /// walk through all pixels. If this is not possible, the function
      /// sets `porigin==nullptr`. Note that this only tests spatial dimesions,
      /// the tensor dimension must still be accessed separately.
      ///
      /// The image must be forged.
      /// \see HasSimpleStride, HasContiguousData, HasNormalStrides, Strides, TensorStride, Data.
      void GetSimpleStrideAndOrigin( dip::uint& stride, void*& origin ) const;

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
      dip::uint TensorElements() const {
         return tensor.Elements();
      }

      /// Get the number of tensor columns.
      dip::uint TensorColumns() const {
         return tensor.Columns();
      }

      /// Get the number of tensor rows.
      dip::uint TensorRows() const {
         return tensor.Rows();
      }

      /// Get the tensor shape.
      enum dip::Tensor::Shape TensorShape() const {
         return tensor.Shape();
      }

      // Note: This function is the reason we refer to the Tensor class as
      // dip::Tensor everywhere in this file.

      /// Get the tensor shape.
      const dip::Tensor& Tensor() const {
         return tensor;
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

      /// Set tensor dimensions; the image must be raw.
      void SetTensorDimensions( dip::uint nelems ) {
         dip_ThrowIf( IsForged(), E::IMAGE_NOT_RAW );
         tensor.SetVector( nelems );
      }

      //
      // Data Type
      //

      // Note: This function is the reason we refer to the DataType class as
      // dip::DataType everywhere in this file.

      /// Get the image's data type.
      dip::DataType DataType() const {
         return datatype;
      }

      /// Set the image's data type; the image must be raw.
      void SetDataType( dip::DataType dt ) {
         dip_ThrowIf( IsForged(), E::IMAGE_NOT_RAW );
         datatype = dt;
      }

      //
      // Color space
      //

      /// Get the image's color space name.
      const String& ColorSpace() const { return colspace; }

      /// Returns true if the image is in color, false if the image is grey-valued.
      bool IsColor() const { return !colspace.empty(); }

      /// Sets the image's color space name; this causes the image to be a color
      /// image, but will cause errors to occur if the number of tensor elements
      /// does not match the expected number of channels for the given color space.
      void SetColorSpace( const String& cs ) { colspace = cs; }

      /// Resets the image's color space information, turning the image into a non-color image.
      void ResetColorSpace() { colspace.clear(); }

      //
      // Physical dimensions
      //

      // Note: This function is the reason we refer to the PixelSize class as
      // dip::PixelSize everywhere in this file.

      /// Get the pixels's size in physical units, by reference, allowing to modify it at will.
      dip::PixelSize& PixelSize() { return pixelsize; }

      /// Get the pixels's size in physical units.
      const dip::PixelSize& PixelSize() const { return pixelsize; }

      /// Set the pixels's physical dimensions.
      void SetPixelSize( const dip::PixelSize& ps ) {
         pixelsize = ps;
      }

      /// Returns true if the pixel has physical dimensions.
      bool HasPixelSize() const { return pixelsize.IsDefined(); }

      /// Returns true if the pixel has the same size in all dimensions.
      bool IsIsotropic() const { return pixelsize.IsIsotropic(); }

      /// Converts a size in pixels to a size in phyical units.
      PhysicalQuantityArray PixelsToPhysical( const FloatArray& in ) const { return pixelsize.ToPhysical( in ); }

      /// Converts a size in physical units to a size in pixels.
      FloatArray PhysicalToPixels( const PhysicalQuantityArray& in ) const { return pixelsize.ToPixels( in ); }

      //
      // Utility functions
      //

      /// Compare properties of an image against a template, either
      /// returns true/false or throws an error.
      bool CompareProperties(
            const Image& src,
            Option::CmpProps cmpProps,
            Option::ThrowException throwException = Option::ThrowException::doThrow
      ) const;

      /// Check image properties, either returns true/false or throws an error.
      ///
      bool CheckProperties(
            const dip::uint ndims,
            const dip::DataType::Classes dts,
            Option::ThrowException throwException = Option::ThrowException::doThrow
      ) const;

      /// Check image properties, either returns true/false or throws an error.
      bool CheckProperties(
            const UnsignedArray& dimensions,
            const dip::DataType::Classes dts,
            Option::ThrowException throwException = Option::ThrowException::doThrow
      ) const;

      /// Check image properties, either returns true/false or throws an error.
      bool CheckProperties(
            const UnsignedArray& dimensions,
            dip::uint tensorElements,
            const dip::DataType::Classes dts,
            Option::ThrowException throwException = Option::ThrowException::doThrow
      ) const;

      /// Copy all image properties from `src`; the image must be raw.
      void CopyProperties( const Image& src ) {
         dip_ThrowIf( IsForged(), E::IMAGE_NOT_RAW );
         datatype       = src.datatype;
         dims           = src.dims;
         strides        = src.strides;
         tensor         = src.tensor;
         colspace       = src.colspace;
         pixelsize      = src.pixelsize;
         if( !externalInterface )
            externalInterface = src.externalInterface;
      }

      /// Make this image similar to the template by copying all its
      /// properties, but not the data.
      void Assimilate( const Image& src ) {
         Strip();
         CopyProperties( src );
         Forge();
      }

      //
      // Data
      // Defined in src/library/image_data.cpp
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
      dip::uint ShareCount() const {
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

      /// Determine if this image shares any pixels with `other`.
      /// If `true`, writing into this image will change the data in
      /// `other`, and vice-versa.
      ///
      /// Both images must be forged.
      /// \see SharesData, Alias.
      bool Aliases( const Image& other ) const;

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
            dip_ThrowIf( IsProtected(), "Image is protected" );
            datablock = nullptr; // Automatically frees old memory if no other pointers to it exist.
            origin = nullptr;    // Keep this one in sync!
         }
      }

      /// Test if forged.
      bool IsForged() const {
         return origin != nullptr;
      }

      /// Set protection flag.
      void Protect( bool set = true ) {
         protect = set;
      }

      /// Test if protected.
      bool IsProtected() const {
         return protect;
      }

      /// Set external interface pointer; the image must be raw.
      void SetExternalInterface( ExternalInterface* ei ) {
         dip_ThrowIf( IsForged(), E::IMAGE_NOT_RAW );
         externalInterface = ei;
      }

      /// Get the number of samples.
      dip::uint NumberOfSamples() const {
         return NumberOfPixels() * TensorElements();
      }

      //
      // Pointers, Offsets, Indices
      // Defined in src/library/image_data.cpp
      //

      /// Get pointer to the first sample in the image, the first tensor
      /// element at coordinates (0,0,0,...); the image must be forged.
      void* Origin() const {
         dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
         return origin;
      }

      /// Get a pointer to the pixel given by the offset. Cast the pointer
      /// to the right type before use. No check is made on the index.
      ///
      /// \see Origin, Offset, OffsetToCoordinates
      void* Pointer( dip::sint offset ) const {
         return (uint8*)origin + offset * datatype.SizeOf();
      }

      /// Get a pointer to the pixel given by the coordinates index. Cast the
      /// pointer to the right type before use. This is not the most efficient
      /// way of indexing many pixels in the image.
      ///
      /// The image must be forged.
      /// \see Origin, Offset, OffsetToCoordinates
      void* Pointer( const UnsignedArray& coords ) const {
         return Pointer( Offset( coords ) );
      }

      /// Compute offset given coordinates. The offset needs to be multiplied
      /// by the number of bytes of each sample to become a memory offset
      /// within the image.
      ///
      /// The image must be forged.
      /// \see Origin, Pointer, OffsetToCoordinates
      dip::sint Offset( const UnsignedArray& coords ) const;

      /// Compute coordinates given an offset.
      ///
      /// The image must be forged.
      /// \see Offset, IndexToCoordinates
      UnsignedArray OffsetToCoordinates( dip::uint offset ) const;

      /// Compute linear index (not offset) given coordinates. This index is not
      /// related to the position of the pixel in memory, and should not be used
      /// to index many pixels in sequence.
      ///
      /// The image must be forged.
      /// \see IndexToCoordinates, Offset
      dip::uint Index( const UnsignedArray& coords ) const;

      /// Compute coordinates given a linear index.
      ///
      /// The image must be forged.
      /// \see Index, Offset, OffsetToCoordinates
      UnsignedArray IndexToCoordinates( dip::uint index ) const;

      //
      // Modifying geometry of a forged image without data copy
      // Defined in src/library/image_manip.cpp
      //

      // TODO: many of these functions must adjust pixel size array

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
      Image& SwapDimensions( dip::uint d1, dip::uint d2 );

      /// Make image 1D. The image must be forged. If HasSimpleStride,
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
      Image& AddSingleton( dip::uint dim );

      /// Append singleton dimensions to increase the image dimensionality.
      /// The image will have `n` dimensions. However, if the image already
      /// has `n` or more dimensions, nothing happens.
      ///
      /// The image must be forged, and the data will never
      /// be copied (i.e. this is a quick and cheap operation).
      ///
      /// \see AddSingleton, ExpandSingletonDimension, Squeeze, PermuteDimensions, Flatten.
      Image& ExpandDimensionality( dip::uint n );

      /// Expand singleton dimension `dim` to `sz` pixels, setting the corresponding
      /// stride to 0. If `dim` is not a singleton dimension (size==1), an
      /// exception is thrown.
      ///
      /// The image must be forged, and the data will never
      /// be copied (i.e. this is a quick and cheap operation).
      ///
      /// \see AddSingleton, ExpandDimensionality.
      Image& ExpandSingletonDimension( dip::uint dim, dip::uint sz );

      /// Mirror de image about selected axes.
      /// The image must be forged, and the data will never
      /// be copied (i.e. this is a quick and cheap operation).
      Image& Mirror( const BooleanArray& process );

      /// Change the tensor shape, without changing the number of tensor elements.
      Image& ReshapeTensor( dip::uint rows, dip::uint cols ) {
         dip_ThrowIf( tensor.Elements() != rows*cols, "Cannot reshape tensor to requested dimensions." );
         tensor.ChangeShape( rows );
         return *this;
      }

      /// Change the tensor shape, without changing the number of tensor elements.
      Image& ReshapeTensor( const dip::Tensor& other ) {
         tensor.ChangeShape( other );
         return *this;
      }

      /// Change the tensor to a vector, without changing the number of tensor elements.
      Image& ReshapeTensorAsVector() {
         tensor.ChangeShape();
         return *this;
      }

      /// Change the tensor to a diagonal matrix, without changing the number of tensor elements.
      Image& ReshapeTensorAsDiagonal() {
         dip::Tensor other { dip::Tensor::Shape::DIAGONAL_MATRIX, tensor.Elements(), tensor.Elements() };
         tensor.ChangeShape( other );
         return *this;
      }

      /// Transpose the tensor.
      Image& Transpose() {
         tensor.Transpose();
         return *this;
      }

      /// Convert tensor dimensions to spatial dimension.
      /// Works even for scalar images, creating a singleton dimension. `dim`
      /// defines the new dimension, subsequent dimensions will be shifted over.
      /// `dim` should not be larger than the number of dimensions. If `dim`
      /// is negative, the new dimension will be the last one. The image must
      /// be forged.
      Image& TensorToSpatial( dip::sint dim = -1 );

      /// Convert spatial dimension to tensor dimensions. The image must be scalar.
      /// If `rows` or `cols` is zero, its size is computed from the size of the
      /// image along dimension `dim`. If both are zero, a default column tensor
      /// is created. If `dim` is negative, the last dimension is used. The
      /// image must be forged.
      Image& SpatialToTensor( dip::sint dim = -1, dip::uint rows = 0, dip::uint cols = 0 );

      /// Split the two values in a complex sample into separate samples,
      /// creating a new spatial dimension of size 2. `dim` defines the new
      /// dimension, subsequent dimensions will be shifted over. `dim` should
      /// not be larger than the number of dimensions. If `dim` is negative,
      /// the new dimension will be the last one. The image must be forged.
      Image& SplitComplex( dip::sint dim = -1 );

      /// Merge the two samples along dimension `dim` into a single complex-valued sample.
      /// Dimension `dim` must have size 2 and a stride of 1. If `dim` is negative, the last
      /// dimension is used. The image must be forged.
      Image& MergeComplex( dip::sint dim = -1 );

      /// Split the two values in a complex sample into separate samples of
      /// a tensor. The image must be scalar and forged.
      Image& SplitComplexToTensor();

      /// Merge the two samples in the tensor into a single complex-valued sample.
      /// The image must have two tensor elements, a tensor stride of 1, and be forged.
      Image& MergeTensorToComplex();

      //
      // Creating views of the data -- indexing without data copy
      // Defined in src/library/image_indexing.cpp
      //

      /// Extract a tensor element, `indices` must have one or two elements; the image must be forged.
      Image operator[]( const UnsignedArray& indices ) const;

      /// Extract a tensor element using linear indexing; the image must be forged.
      Image operator[]( dip::uint index ) const;

      /// Extracts the tensor elements along the diagonal; the image must be forged.
      Image Diagonal() const;

      /// Extracts the pixel at the given coordinates; the image must be forged.
      Image At( const UnsignedArray& coords ) const;

      /// Extracts the pixel at the given linear index (inneficient!); the image must be forged.
      Image At( dip::uint index ) const;

      /// Extracts a subset of pixels from a 1D image; the image must be forged.
      Image At( Range x_range ) const;

      /// Extracts a subset of pixels from a 2D image; the image must be forged.
      Image At( Range x_range, Range y_range ) const;

      /// Extracts a subset of pixels from a 3D image; the image must be forged.
      Image At( Range x_range, Range y_range, Range z_range ) const;

      /// Extracts a subset of pixels from an image; the image must be forged.
      Image At( RangeArray ranges ) const;

      /// Extracts the real component of a complex-typed image; the image must be forged.
      Image Real() const;

      /// Extracts the imaginary component of a complex-typed image; the image must be forged.
      Image Imaginary() const;

      /// Quick copy, returns a new image that points at the same data as `this`,
      /// and has mostly the same properties. The color space and pixel size
      /// information are not copied, and the protect flag is reset.
      /// This function is mostly meant for use in functions that need to
      /// modify some properties of the input images, without actually modifying
      /// the input images.
      Image QuickCopy() const {
         Image out {};
         out.datatype = datatype;
         out.dims = dims;
         out.strides = strides;
         out.tensor = tensor;
         out.tstride = tstride;
         out.datablock = datablock;
         out.origin = origin;
         out.externalInterface = externalInterface;
         return out;
      }

      //
      // Getting/setting pixel values
      // Defined in src/library/image_data.cpp
      //

      /// Deep copy, `this` will become a copy of `src` with its own data.
      ///
      /// If `this` is forged, then `src` is expected to have the same dimensions
      /// and number of tensor elements, and the data is copied over from `src`
      /// to `this`. The copy will apply data type conversion, where values are
      /// clipped to the target range and/or truncated, as applicable. Complex
      /// values are converted to non-complex values by taking the absolute
      /// value.
      ///
      /// If `this` is not forged, then all the properties of `src` will be
      /// copied to `this`, `this` will be forged, and the data from `src` will
      /// be copied over.
      void Copy( const Image& src );

      // TODO: Add a function to convert the image to another data type.
      // Conversion from uint8 to bin and back can occur in-place.

      /// Sets all samples in the image to the value `v`. The function is defined
      /// for values `v` of type dip::sint, dip::dfloat, and dip::dcomplex. The
      /// value will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged.
      void Set( dip::sint v );

      /// Sets all samples in the image to the value `v`. The function is defined
      /// for values `v` of type dip::sint, dip::dfloat, and dip::dcomplex. The
      /// value will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged.
      void Set( dfloat v );

      /// Sets all samples in the image to the value `v`. The function is defined
      /// for values `v` of type dip::sint, dip::dfloat, and dip::dcomplex. The
      /// value will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged.
      void Set( dcomplex v );

      /// Extracts the fist sample in the first pixel (At(0,0)[0]), casted
      /// to a signed integer of maximum width; for complex values
      /// returns the absolute value.
      explicit operator sint() const;

      /// Extracts the fist sample in the first pixel (At(0,0)[0]), casted
      /// to a double-precision floating point value; for complex values
      /// returns the absolute value.
      explicit operator dfloat() const;

      /// Extracts the fist sample in the first pixel (At(0,0)[0]).
      explicit operator dcomplex() const;


   private:

      //
      // Implementation
      //

      dip::DataType datatype = DT_SFLOAT;
      UnsignedArray dims;                 // dims.size() == ndims (if forged)
      IntegerArray strides;               // strides.size() == ndims (if forged)
      dip::Tensor tensor;
      dip::sint tstride = 0;
      bool protect = false;               // When set, don't strip image
      String colspace;
      dip::PixelSize pixelsize;
      std::shared_ptr<void> datablock;    // Holds the pixel data. Data block will be freed when last image
                                          //    that uses it is destroyed.
      void* origin = nullptr;             // Points to the origin ( pixel (0,0) ), not necessarily the first
                                          //    pixel of the data block.
      ExternalInterface* externalInterface = nullptr;
                                          // A function that will be called instead of the default forge function.

      //
      // Some private functions
      //

      bool HasValidStrides() const;       // Are the strides such that no two samples are in the same memory cell?

      void SetNormalStrides();            // Fill in all strides.

      void GetDataBlockSizeAndStart( dip::uint& size, dip::sint& start ) const;
      void GetDataBlockSizeAndStartWithTensor( dip::uint& size, dip::sint& start ) const;
                                          // size is the distance between top left and bottom right corners.
                                          // start is the distance between top left corner and origin
                                          // (will be <0 if any strides[ii] < 0). All measured in samples.

}; // class Image

/// An array of images
typedef std::vector<Image> ImageArray;

/// An array of image references
typedef std::vector<std::reference_wrapper<Image>> ImageRefArray;

/// An array of const image references
typedef std::vector<std::reference_wrapper<const Image>> ImageConstRefArray;


//
// Overloaded operators
//

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
