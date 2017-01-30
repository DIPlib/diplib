/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2017, Cris Luengo.
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
#include <utility>

#include "diplib/library/datatype.h"
#include "diplib/library/tensor.h"
#include "diplib/library/physical_dimensions.h"
#include "diplib/library/clamp_cast.h"

#include <iostream>

/// \file
/// \brief Defines the `dip::Image` class and support functions. This file is always included through `diplib.h`.
/// \see infrastructure


/// \brief The `dip` namespace contains all the library functionality.
namespace dip {


/// \addtogroup infrastructure
/// \{


//
// Support for external interfaces
//

/// \brief Support for external interfaces.
///
/// Software using *DIPlib* might want to
/// control how the image data is allocated. Such software should derive
/// a class from this one, and assign a pointer to it into each of the
/// images that it creates, through Image::SetExternalInterface().
/// The caller will maintain ownership of the interface.
///
/// See dip_matlab_interface.h for an example of how to create an ExternalInterface.
class ExternalInterface {
   public:
      /// Allocates the data for an image. The function is free to modify
      /// `strides` and `tstride` if desired, though they will be set
      /// to the normal values by the calling function.
      virtual std::shared_ptr< void > AllocateData(
            UnsignedArray const& sizes,
            IntegerArray& strides,
            dip::Tensor const& tensor,
            dip::sint& tstride,
            dip::DataType datatype
      ) = 0;
};


//
// Functor that converts indices or offsets to coordinates.
//

/// \brief Computes pixel coordinates based on an index or offset.
///
/// Objects of this class are returned by `dip::Image::OffsetToCoordinatesComputer`
/// and `dip::Image::IndexToCoordinatesComputer`, and act as functors.
/// Call it with an offset or index (depending on which function created the
/// functor), and it will return the coordinates:
///
/// ```cpp
///     auto coordComp = img.OffsetToCoordinatesComputer();
///     auto coords1 = coordComp( offset1 );
///     auto coords2 = coordComp( offset2 );
///     auto coords3 = coordComp( offset3 );
/// ```
///
/// Note that the coordinates must be inside the image domain, if the offset given
/// does not correspond to one of the image's pixels, the result is meaningless.
class CoordinatesComputer {
   public:
      CoordinatesComputer( UnsignedArray const& sizes, IntegerArray const& strides );

      UnsignedArray operator()( dip::sint offset ) const;

   private:
      IntegerArray strides_; // a copy of the image's strides array, but with all positive values
      IntegerArray sizes_;   // a copy of the image's sizes array, but with negative values where the strides are negative
      UnsignedArray index_;  // sorted indices to the strides array (largest to smallest)
      dip::sint offset_;     // offset needed to handle negative strides
};


//
// The Image class
//

// Forward declarations
class Image;
class ImageSliceIterator;

/// \brief An array of images
using ImageArray = std::vector< Image >;

/// \brief An array of const images
using ConstImageArray = std::vector< Image const >;

/// \brief An array of image references
using ImageRefArray = std::vector< std::reference_wrapper< Image >>;

/// \brief An array of const image references
using ImageConstRefArray = std::vector< std::reference_wrapper< Image const >>;

// The class is documented in the file src/documentation/image.md
class Image {

   public:

      //
      // Constructor
      //

      /// \name Constructors
      /// \{

      /// \brief The default-initialized image is 0D (an empty sizes array), one tensor element, dip::DT_SFLOAT,
      /// and raw (it has no data segment).
      Image() {}

      // Copy constructor, move constructor, copy assignment, move assignment and destructor are all default.
      Image( Image const& ) = default;
      Image( Image&& ) = default;
      ~Image() = default;

      /// \brief The copy assignment does not copy pixel data, the LHS shares the data pointer with the RHS, except
      /// in the case where the LHS image has an external interface set. See \ref external_interface.
      Image& operator=( Image const& rhs ) {
         if( externalInterface_ && ( externalInterface_ != rhs.externalInterface_ )) {
            // Copy pixel data too
            this->Copy( rhs );
         } else {
            // Do what the default copy assignment would do
            dataType_ = rhs.dataType_;
            sizes_ = rhs.sizes_;
            strides_ = rhs.strides_;
            tensor_ = rhs.tensor_;
            tensorStride_ = rhs.tensorStride_;
            protect_ = rhs.protect_;
            colorSpace_ = rhs.colorSpace_;
            pixelSize_ = rhs.pixelSize_;
            dataBlock_ = rhs.dataBlock_;
            origin_ = rhs.origin_;
            externalInterface_ = rhs.externalInterface_;
         }
         return *this;
      }

      /// \brief The move assignment copies the data in the case where the LHS image has an external interface set.
      /// See \ref external_interface.
      Image& operator=( Image&& rhs ) {
         if( externalInterface_ && ( externalInterface_ != rhs.externalInterface_ )) {
            // Copy pixel data too
            this->Copy( rhs );
         } else {
            // Do what the default move assignment would do
            this->swap( rhs );
         }
         return *this;
      }

      /// \brief Forged image of given sizes and data type. The data is left uninitialized.
      explicit Image( UnsignedArray const& sizes, dip::uint tensorElems = 1, dip::DataType dt = DT_SFLOAT ) :
            dataType_( dt ),
            sizes_( sizes ),
            tensor_( tensorElems ) {
         Forge();
      }

      /// \brief Create a 0-D image with the value and data type of `p`.
      template< typename T >
      explicit Image( T p ) {
         dataType_ = dip::DataType( p );
         Forge();                            // sizes is empty by default
         *static_cast< T* >( origin_ ) = p;
      }

      /// \brief Create a 0-D vector image with the values and data type of `plist`.
      template< typename T >
      explicit Image( std::initializer_list< T > const& plist ) {
         tensor_.SetVector( plist.size() );  // will throw if p.size()==0
         dataType_ = dip::DataType( *( plist.begin() ));
         Forge();                            // sizes is empty by default
         T* ptr = static_cast< T* >( origin_ );
         for( T const& p : plist ) {
            *ptr = p;
            ptr += tensorStride_;
         }
      }

      /// \brief Create a 0-D image with the value of `p` and the given data type.
      template< typename T >
      Image( T p, dip::DataType dt ) {
         dataType_ = dt;
         Forge();                            // sizes is empty by default
         switch( dataType_ ) {
            case dip::DT_BIN:
               *static_cast< bin* >( origin_ ) = clamp_cast< bin >( p );
               break;
            case dip::DT_UINT8:
               *static_cast< uint8* >( origin_ ) = clamp_cast< uint8 >( p );
               break;
            case dip::DT_UINT16:
               *static_cast< uint16* >( origin_ ) = clamp_cast< uint16 >( p );
               break;
            case dip::DT_UINT32:
               *static_cast< uint32* >( origin_ ) = clamp_cast< uint32 >( p );
               break;
            case dip::DT_SINT8:
               *static_cast< sint8* >( origin_ ) = clamp_cast< sint8 >( p );
               break;
            case dip::DT_SINT16:
               *static_cast< sint16* >( origin_ ) = clamp_cast< sint16 >( p );
               break;
            case dip::DT_SINT32:
               *static_cast< sint32* >( origin_ ) = clamp_cast< sint32 >( p );
               break;
            case dip::DT_SFLOAT:
               *static_cast< sfloat* >( origin_ ) = clamp_cast< sfloat >( p );
               break;
            case dip::DT_DFLOAT:
               *static_cast< dfloat* >( origin_ ) = clamp_cast< dfloat >( p );
               break;
            case dip::DT_SCOMPLEX:
               *static_cast< scomplex* >( origin_ ) = clamp_cast< scomplex >( p );
               break;
            case dip::DT_DCOMPLEX:
               *static_cast< dcomplex* >( origin_ ) = clamp_cast< dcomplex >( p );
               break;
            default: DIP_THROW( dip::E::DATA_TYPE_NOT_SUPPORTED );
         }
      }

      /// \brief Create a 0-D vector image with the values of `plist` and the given data type.
      template< typename T >
      Image( std::initializer_list< T > const& plist, dip::DataType dt ) {
         tensor_.SetVector( plist.size() );  // will throw if p.size()==0
         dataType_ = dt;
         Forge();                            // sizes is empty by default
         switch( dataType_ ) {
            case dip::DT_BIN: {
               bin* ptr = static_cast< bin* >( origin_ );
               for( T const& p : plist ) {
                  *ptr = p;
                  ptr += tensorStride_;
               }
               break;
            }
            case dip::DT_UINT8: {
               uint8* ptr = static_cast< uint8* >( origin_ );
               for( T const& p : plist ) {
                  *ptr = p;
                  ptr += tensorStride_;
               }
               break;
            }
            case dip::DT_UINT16: {
               uint16* ptr = static_cast< uint16* >( origin_ );
               for( T const& p : plist ) {
                  *ptr = p;
                  ptr += tensorStride_;
               }
               break;
            }
            case dip::DT_UINT32: {
               uint32* ptr = static_cast< uint32* >( origin_ );
               for( T const& p : plist ) {
                  *ptr = p;
                  ptr += tensorStride_;
               }
               break;
            }
            case dip::DT_SINT8: {
               sint8* ptr = static_cast< sint8* >( origin_ );
               for( T const& p : plist ) {
                  *ptr = p;
                  ptr += tensorStride_;
               }
               break;
            }
            case dip::DT_SINT16: {
               sint16* ptr = static_cast< sint16* >( origin_ );
               for( T const& p : plist ) {
                  *ptr = p;
                  ptr += tensorStride_;
               }
               break;
            }
            case dip::DT_SINT32: {
               sint32* ptr = static_cast< sint32* >( origin_ );
               for( T const& p : plist ) {
                  *ptr = p;
                  ptr += tensorStride_;
               }
               break;
            }
            case dip::DT_SFLOAT: {
               sfloat* ptr = static_cast< sfloat* >( origin_ );
               for( T const& p : plist ) {
                  *ptr = p;
                  ptr += tensorStride_;
               }
               break;
            }
            case dip::DT_DFLOAT: {
               dfloat* ptr = static_cast< dfloat* >( origin_ );
               for( T const& p : plist ) {
                  *ptr = p;
                  ptr += tensorStride_;
               }
               break;
            }
            case dip::DT_SCOMPLEX: {
               scomplex* ptr = static_cast< scomplex* >( origin_ );
               for( T const& p : plist ) {
                  *ptr = p;
                  ptr += tensorStride_;
               }
               break;
            }
            case dip::DT_DCOMPLEX: {
               dcomplex* ptr = static_cast< dcomplex* >( origin_ );
               for( T const& p : plist ) {
                  *ptr = p;
                  ptr += tensorStride_;
               }
               break;
            }
            default: DIP_THROW( dip::E::DATA_TYPE_NOT_SUPPORTED );
         }
      }

      /// \brief Create an image around existing data.
      Image(
            std::shared_ptr< void > data,        // points at the data block, not necessarily the origin!
            dip::DataType dataType,
            UnsignedArray const& sizes,
            IntegerArray const& strides,
            dip::Tensor const& tensor,
            dip::sint tensorStride,
            dip::ExternalInterface* externalInterface = nullptr
      ) :
            dataType_( dataType ),
            sizes_( sizes ),
            strides_( strides ),
            tensor_( tensor ),
            tensorStride_( tensorStride ),
            dataBlock_( data ),
            externalInterface_( externalInterface ) {
         dip::uint size;
         dip::sint start;
         GetDataBlockSizeAndStartWithTensor( size, start );
         origin_ = static_cast< uint8* >( dataBlock_.get() ) + start * dataType_.SizeOf();
      }

      /// \brief Create a new forged image similar to `this`. The data is not copied, and left uninitialized.
      Image Similar() {
         Image out;
         out.CopyProperties( *this );
         out.Forge();
         return out;
      }

      /// \brief Create a new forged image similar to `this`, but with different data type. The data is not copied, and left uninitialized.
      Image Similar( dip::DataType dt ) {
         Image out;
         out.CopyProperties( *this );
         out.dataType_ = dt;
         out.Forge();
         return out;
      }

      /// \}

      //
      // Sizes
      //

      /// \name Sizes
      /// \{

      /// \brief Get the number of spatial dimensions.
      dip::uint Dimensionality() const {
         return sizes_.size();
      }

      /// \brief Get a const reference to the sizes array (image size).
      UnsignedArray const& Sizes() const {
         return sizes_;
      }

      /// \brief Get the image size along a specific dimension.
      dip::uint Size( dip::uint dim ) const {
         return sizes_[ dim ];
      }

      /// \brief Get the number of pixels.
      dip::uint NumberOfPixels() const {
         return sizes_.product();
      }

      /// \brief Get the number of samples.
      dip::uint NumberOfSamples() const {
         return NumberOfPixels() * TensorElements();
      }

      /// \brief Set the image sizes. The image must be raw.
      void SetSizes( UnsignedArray const& d ) {
         DIP_THROW_IF( IsForged(), E::IMAGE_NOT_RAW );
         sizes_ = d;
      }

      /// \}

      //
      // Strides
      //

      /// \name Strides
      /// \{

      /// \brief Get a const reference to the strides array.
      IntegerArray const& Strides() const {
         return strides_;
      }

      /// \brief Get the stride along a specific dimension.
      dip::sint Stride( dip::uint dim ) const {
         return strides_[ dim ];
      }

      /// \brief Get the tensor stride.
      dip::sint TensorStride() const {
         return tensorStride_;
      }

      /// \brief Set the strides array. The image must be raw.
      void SetStrides( IntegerArray const& s ) {
         DIP_THROW_IF( IsForged(), E::IMAGE_NOT_RAW );
         strides_ = s;
      }

      /// \brief Set the tensor stride. The image must be raw.
      void SetTensorStride( dip::sint ts ) {
         DIP_THROW_IF( IsForged(), E::IMAGE_NOT_RAW );
         tensorStride_ = ts;
      }

      /// \brief Test if all the pixels are contiguous.
      ///
      /// If all pixels are contiguous, you can traverse the whole image,
      /// accessing each of the pixles, using a single stride with a value
      /// of 1. To do so, you don't necessarily start at the origin: if any
      /// of the strides is negative, the origin of the contiguous data will
      /// be elsewhere.
      /// Use `dip::Image::GetSimpleStrideAndOrigin` to get a pointer to the origin
      /// of the contiguous data.
      ///
      /// The image must be forged.
      ///
      /// \see GetSimpleStrideAndOrigin, HasSimpleStride, HasNormalStrides, Strides, TensorStride.
      bool HasContiguousData() const {
         DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
         dip::uint size = NumberOfPixels() * TensorElements();
         dip::sint start;
         dip::uint sz;
         GetDataBlockSizeAndStartWithTensor( sz, start );
         return sz == size;
      }

      /// \brief Test if strides are as by default. The image must be forged.
      bool HasNormalStrides() const;

      /// \brief Test if the whole image can be traversed with a single stride
      /// value.
      ///
      /// This is similar to `dip::Image::HasContiguousData`, but the stride
      /// value can be larger than 1.
      /// Use `dip::Image::GetSimpleStrideAndOrigin` to get a pointer to the origin
      /// of the contiguous data. Note that this only tests spatial
      /// dimesions, the tensor dimension must still be accessed separately.
      ///
      /// The image must be forged.
      ///
      /// \see GetSimpleStrideAndOrigin, HasContiguousData, HasNormalStrides, Strides, TensorStride.
      bool HasSimpleStride() const {
         void* p;
         dip::uint s;
         GetSimpleStrideAndOrigin( s, p );
         return p != nullptr;
      }

      /// \brief Return a pointer to the start of the data and a single stride to
      /// walk through all pixels.
      ///
      /// If this is not possible, the function
      /// sets `porigin==nullptr`. Note that this only tests spatial dimesions,
      /// the tensor dimension must still be accessed separately.
      ///
      /// The image must be forged.
      ///
      /// \see HasSimpleStride, HasContiguousData, HasNormalStrides, Strides, TensorStride, Data.
      void GetSimpleStrideAndOrigin( dip::uint& stride, void*& origin ) const;

      /// \brief Checks to see if `other` and `this` have their dimensions ordered in
      /// the same way.
      ///
      /// Traversing more than one image using simple strides is only
      /// possible if they have their dimensions ordered in the same way, otherwise
      /// the simple stride does not visit the pixels in the same order in the
      /// various images.
      ///
      /// The images must be forged.
      ///
      /// \see HasSimpleStride, GetSimpleStrideAndOrigin, HasContiguousData.
      bool HasSameDimensionOrder( Image const& other ) const;

      /// \}

      //
      // Tensor
      //

      /// \name Tensor
      /// \{

      /// \brief Get the tensor sizes. The array returned can have 0, 1 or
      /// 2 elements, as those are the allowed tensor dimensionalities.
      UnsignedArray TensorSizes() const {
         return tensor_.Sizes();
      }

      /// \brief Get the number of tensor elements (i.e. the number of samples per pixel),
      /// the product of the elements in the array returned by TensorSizes.
      dip::uint TensorElements() const {
         return tensor_.Elements();
      }

      /// \brief Get the number of tensor columns.
      dip::uint TensorColumns() const {
         return tensor_.Columns();
      }

      /// \brief Get the number of tensor rows.
      dip::uint TensorRows() const {
         return tensor_.Rows();
      }

      /// \brief Get the tensor shape.
      enum dip::Tensor::Shape TensorShape() const {
         return tensor_.Shape();
      }

      // Note: This function is the reason we refer to the Tensor class as
      // dip::Tensor everywhere in this file.

      /// \brief Get the tensor shape.
      dip::Tensor const& Tensor() const {
         return tensor_;
      }

      /// \brief True for non-tensor (grey-value) images.
      bool IsScalar() const {
         return tensor_.IsScalar();
      }

      /// \brief True for vector images, where the tensor is one-dimensional.
      bool IsVector() const {
         return tensor_.IsVector();
      }

      /// \brief Set tensor sizes. The image must be raw.
      void SetTensorSizes( UnsignedArray const& tdims ) {
         DIP_THROW_IF( IsForged(), E::IMAGE_NOT_RAW );
         tensor_.SetSizes( tdims );
      }

      /// \brief Set tensor sizes. The image must be raw.
      void SetTensorSizes( dip::uint nelems ) {
         DIP_THROW_IF( IsForged(), E::IMAGE_NOT_RAW );
         tensor_.SetVector( nelems );
      }

      /// \}

      //
      // Data type
      //

      /// \name Data type
      /// \{

      // Note: This function is the reason we refer to the DataType class as
      // dip::DataType everywhere in this file.

      /// \brief Get the image's data type.
      dip::DataType DataType() const {
         return dataType_;
      }

      /// \brief Set the image's data type. The image must be raw.
      void SetDataType( dip::DataType dt ) {
         DIP_THROW_IF( IsForged(), E::IMAGE_NOT_RAW );
         dataType_ = dt;
      }

      /// \}

      //
      // Color space
      //

      /// \name Color space
      /// \{

      /// \brief Get the image's color space name.
      String const& ColorSpace() const {
         return colorSpace_;
      }

      /// \brief Returns true if the image is in color, false if the image is grey-valued.
      bool IsColor() const {
         return !colorSpace_.empty();
      }

      /// \brief Sets the image's color space name. This causes the image to be a color
      /// image, but will cause errors to occur if the number of tensor elements
      /// does not match the expected number of channels for the given color space.
      void SetColorSpace( String const& cs ) {
         colorSpace_ = cs;
      }

      /// \brief Resets the image's color space information, turning the image into a non-color image.
      void ResetColorSpace() {
         colorSpace_.clear();
      }

      /// \}

      //
      // Pixel size
      //

      /// \name Pixel size
      /// \{

      // Note: This function is the reason we refer to the PixelSize class as
      // dip::PixelSize everywhere in this file.

      /// \brief Get the pixels's size in physical units, by reference, allowing to modify it at will.
      dip::PixelSize& PixelSize() {
         return pixelSize_;
      }

      /// \brief Get the pixels's size in physical units.
      dip::PixelSize const& PixelSize() const {
         return pixelSize_;
      }

      /// \brief Get the pixels's size in physical units along the given dimension.
      PhysicalQuantity PixelSize( dip::uint dim ) const {
         return pixelSize_[ dim ];
      }

      /// \brief Set the pixels's size.
      void SetPixelSize( dip::PixelSize const& ps ) {
         pixelSize_ = ps;
      }

      /// \brief Returns true if the pixel has physical dimensions.
      bool HasPixelSize() const {
         return pixelSize_.IsDefined();
      }

      /// \brief Returns true if the pixel has the same size in all dimensions.
      bool IsIsotropic() const {
         return pixelSize_.IsIsotropic();
      }

      /// \brief Converts a size in pixels to a size in phyical units.
      PhysicalQuantityArray PixelsToPhysical( FloatArray const& in ) const {
         return pixelSize_.ToPhysical( in );
      }

      /// \brief Converts a size in physical units to a size in pixels.
      FloatArray PhysicalToPixels( PhysicalQuantityArray const& in ) const {
         return pixelSize_.ToPixels( in );
      }

      /// \}

      //
      // Utility functions
      //

      /// \name Utility functions
      /// \{

      /// \brief Compare properties of an image against a template, either
      /// returns true/false or throws an error.
      bool CompareProperties(
            Image const& src,
            Option::CmpProps cmpProps,
            Option::ThrowException throwException = Option::ThrowException::DO_THROW
      ) const;

      /// \brief Check image properties, either returns true/false or throws an error.
      bool CheckProperties(
            dip::uint ndims,
            dip::DataType::Classes dts,
            Option::ThrowException throwException = Option::ThrowException::DO_THROW
      ) const;

      /// \brief Check image properties, either returns true/false or throws an error.
      bool CheckProperties(
            dip::uint ndims,
            dip::uint tensorElements,
            dip::DataType::Classes dts,
            Option::ThrowException throwException = Option::ThrowException::DO_THROW
      ) const;

      /// \brief Check image properties, either returns true/false or throws an error.
      bool CheckProperties(
            UnsignedArray const& sizes,
            dip::DataType::Classes dts,
            Option::ThrowException throwException = Option::ThrowException::DO_THROW
      ) const;

      /// \brief Check image properties, either returns true/false or throws an error.
      bool CheckProperties(
            UnsignedArray const& sizes,
            dip::uint tensorElements,
            dip::DataType::Classes dts,
            Option::ThrowException throwException = Option::ThrowException::DO_THROW
      ) const;

      /// \brief Check image properties for a mask image, either returns true/false or throws an error.
      bool CheckIsMask(
            UnsignedArray const& sizes,
            Option::AllowSingletonExpansion allowSingletonExpansion = Option::AllowSingletonExpansion::DONT_ALLOW,
            Option::ThrowException throwException = Option::ThrowException::DO_THROW
      ) const;

      /// \brief Copy all image properties from `src`. The image must be raw.
      void CopyProperties( Image const& src ) {
         DIP_THROW_IF( IsForged(), E::IMAGE_NOT_RAW );
         dataType_ = src.dataType_;
         sizes_ = src.sizes_;
         strides_ = src.strides_;
         tensor_ = src.tensor_;
         tensorStride_ = src.tensorStride_;
         colorSpace_ = src.colorSpace_;
         pixelSize_ = src.pixelSize_;
         if( !externalInterface_ ) {
            externalInterface_ = src.externalInterface_;
         }
      }

      /// \brief Copy non-data image properties from `src`.
      ///
      /// The non-data image properties are those that do not influence how the data is stored in
      /// memory: tensor shape, color space, and pixel size. The number of tensor elements of the
      /// the two images must match. The image must be forged.
      ///
      /// \see CopyProperties, ResetNonDataProperties
      void CopyNonDataProperties( Image const& src ) {
         DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( tensor_.Elements() != src.tensor_.Elements(), E::NTENSORELEM_DONT_MATCH );
         tensor_ = src.tensor_;
         colorSpace_ = src.colorSpace_;
         pixelSize_ = src.pixelSize_;
      }

      /// \brief Reset non-data image properties.
      ///
      /// The non-data image properties are those that do not influence how the data is stored in
      /// memory: tensor shape, color space, and pixel size.
      ///
      /// \see CopyNonDataProperties, Strip
      void ResetNonDataProperties() {
         tensor_ = {};
         colorSpace_ = {};
         pixelSize_ = {};
      }

      /// \brief Swaps `this` and `other`.
      void swap( Image& other ) {
         using std::swap;
         swap( dataType_, other.dataType_ );
         swap( sizes_, other.sizes_ );
         swap( strides_, other.strides_ );
         swap( tensor_, other.tensor_ );
         swap( tensorStride_, other.tensorStride_ );
         swap( protect_, other.protect_ );
         swap( colorSpace_, other.colorSpace_ );
         swap( pixelSize_, other.pixelSize_ );
         swap( dataBlock_, other.dataBlock_ );
         swap( origin_, other.origin_ );
         swap( externalInterface_, other.externalInterface_ );
      }

      /// \}

      //
      // Data
      // Defined in src/library/image_data.cpp
      //

      /// \name Data
      /// \{

      /// \brief Get pointer to the data segment.
      ///
      /// This is useful to identify
      /// the data segment, but not to access the pixel data stored in
      /// it. Use `dip::Image::Origin` instead. The image must be forged.
      ///
      /// \see Origin, IsShared, ShareCount, SharesData.
      void* Data() const {
         DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
         return dataBlock_.get();
      }

      /// \brief Check to see if the data segment is shared with other images.
      /// The image must be forged.
      ///
      /// \see Data, ShareCount, SharesData.
      bool IsShared() const {
         DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
         return !dataBlock_.unique();
      }

      /// \brief Get the number of images that share their data with this image.
      ///
      /// The count is always at least 1. If the count is 1, `dip::Image::IsShared` is
      /// false. The image must be forged.
      ///
      /// \see Data, IsShared, SharesData.
      dip::uint ShareCount() const {
         DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
         return static_cast< dip::uint >( dataBlock_.use_count() );
      }

      /// \brief Determine if `this` shares its data pointer with `other`.
      ///
      /// Note that sharing the data pointer
      /// does not imply that the two images share any pixel data, as it
      /// is possible for the two images to represent disjoint windows
      /// into the same data block. To determine if any pixels are shared,
      /// use Aliases.
      ///
      /// Both images must be forged.
      ///
      /// \see Aliases, IsIdenticalView, IsOverlappingView, Data, IsShared, ShareCount.
      bool SharesData( Image const& other ) const {
         DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( !other.IsForged(), E::IMAGE_NOT_FORGED );
         return dataBlock_ == other.dataBlock_;
      }

      /// \brief Determine if `this` shares any samples with `other`.
      ///
      /// If `true`, writing into this image will change the data in
      /// `other`, and vice-versa.
      ///
      /// Both images must be forged.
      ///
      /// \see SharesData, IsIdenticalView, IsOverlappingView, Alias.
      bool Aliases( Image const& other ) const;

      /// \brief Determine if `this` and `other` offer an identical view of the
      /// same set of pixels.
      ///
      /// If `true`, changing one sample in this image will change the same sample in `other`.
      ///
      /// Both images must be forged.
      ///
      /// \see SharesData, Aliases, IsOverlappingView.
      bool IsIdenticalView( Image const& other ) const {
         DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( !other.IsForged(), E::IMAGE_NOT_FORGED );
         // We don't need to check dataBlock_ here, as origin_ is a pointer, not an offset.
         return ( origin_ == other.origin_ ) &&
                ( dataType_ == other.dataType_ ) &&
                ( strides_ == other.strides_ ) &&
                ( tensorStride_ == other.tensorStride_ );
      }

      /// \brief Determine if `this` and `other` offer different views of the
      /// same data segment, and share at least one sample.
      ///
      /// If `true`, changing one
      /// sample in this image might change a different sample in `other`.
      /// An image with an overlapping view of an input image cannot be used as output to a
      /// filter, as it might change input data that still needs to be used. Use this function
      /// to test whether to use the existing data segment or allocate a new one.
      ///
      /// Both images must be forged.
      ///
      /// \see SharesData, Aliases, IsIdenticalView.
      bool IsOverlappingView( Image const& other ) const {
         // Aliases checks for both images to be forged.
         return Aliases( other ) && !IsIdenticalView( other );
      }

      /// \brief Determine if `this` and any of those in `other` offer different views of the
      /// same data segment, and share at least one sample.
      ///
      /// If `true`, changing one
      /// sample in this image might change a different sample in at least one image in `other`.
      /// An image with an overlapping view of an input image cannot be used as output to a
      /// filter, as it might change input data that still needs to be used. Use this function
      /// to test whether to use the existing data segment or allocate a new one.
      ///
      /// `this` must be forged.
      ///
      /// \see SharesData, Aliases, IsIdenticalView.
      bool IsOverlappingView( ImageConstRefArray const& other ) const {
         for( dip::uint ii = 0; ii < other.size(); ++ii ) {
            Image const& tmp = other[ ii ].get();
            if( tmp.IsForged() && IsOverlappingView( tmp )) {
               return true;
            }
         }
         return false;
      }

      /// Determine if `this` and any of those in `other` offer different views of the
      /// same data segment, and share at least one sample.
      ///
      /// If `true`, changing one
      /// sample in this image might change a different sample in at least one image in `other`.
      /// An image with an overlapping view of an input image cannot be used as output to a
      /// filter, as it might change input data that still needs to be used. Use this function
      /// to test whether to use the existing data segment or allocate a new one.
      ///
      /// `this` must be forged.
      ///
      /// \see SharesData, Aliases, IsIdenticalView.
      bool IsOverlappingView( ImageArray const& other ) const {
         for( dip::uint ii = 0; ii < other.size(); ++ii ) {
            Image const& tmp = other[ ii ];
            if( tmp.IsForged() && IsOverlappingView( tmp )) {
               return true;
            }
         }
         return false;
      }

      /// \brief Allocate data segment.
      ///
      /// This function allocates a memory block
      /// to hold the pixel data. If the stride array is consistent with
      /// size array, and leads to a compact data segment, it is honored.
      /// Otherwise, it is ignored and a new stride array is created that
      /// leads to an image that `dip::Image::HasNormalStrides`. If an
      /// external interface is registered for this image, that interface
      /// may create whatever strides are suitable, may honor or not the
      /// exising stride array, and may or may not produce normal strides.
      void Forge();

      /// \brief Modify image properties and forge the image.
      ///
      /// ReForge has three
      /// signatures that match three image constructors. ReForge will try
      /// to avoid freeing the current data segment and allocating a new one.
      /// This version will cause `this` to be an identical copy of `src`,
      /// but with uninitialized data. The external interface of `src` is
      /// not used, nor are its strides.
      ///
      /// If `this` doesn't match the requested properties, it must be stripped
      /// and forged. If `this` is protected (see `dip::Image::Protect`) and
      /// forged, an exception will be thrown by `dip::Image::Strip`. However,
      /// if `acceptDataTypeChange` is `dip::Option::AcceptDataTypeChange::DO_ALLOW`,
      /// a protected image will keep its
      /// old data type, and no exception will be thrown if this data type
      /// is different from `dt`. Note that other properties much still match
      /// if `this` was forged. Thus, this flag allows `this` to control the
      /// data type of the image, ignoring any requested data type here.
      void ReForge(
            Image const& src,
            Option::AcceptDataTypeChange acceptDataTypeChange = Option::AcceptDataTypeChange::DONT_ALLOW
      ) {
         ReForge( src, src.dataType_, acceptDataTypeChange );
      }

      /// \brief Modify image properties and forge the image.
      ///
      /// ReForge has three
      /// signatures that match three image constructors. ReForge will try
      /// to avoid freeing the current data segment and allocating a new one.
      /// This version will cause `this` to be an identical copy of `src`,
      /// but with a different data type and uninitialized data. The
      /// external interface of `src` is not used, nor are its strides.
      ///
      /// If `this` doesn't match the requested properties, it must be stripped
      /// and forged. If `this` is protected (see `dip::Image::Protect`) and
      /// forged, an exception will be thrown by `dip::Image::Strip`. However,
      /// if `acceptDataTypeChange` is `dip::Option::AcceptDataTypeChange::DO_ALLOW`,
      /// a protected image will keep its
      /// old data type, and no exception will be thrown if this data type
      /// is different from `dt`. Note that other properties much still match
      /// if `this` was forged. Thus, this flag allows `this` to control the
      /// data type of the image, ignoring any requested data type here.
      void ReForge(
            Image const& src,
            dip::DataType dt,
            Option::AcceptDataTypeChange acceptDataTypeChange = Option::AcceptDataTypeChange::DONT_ALLOW
      ) {
         ReForge( src.sizes_, src.tensor_.Elements(), dt, acceptDataTypeChange );
         CopyNonDataProperties( src );
      }

      /// \brief Modify image properties and forge the image.
      ///
      /// ReForge has three
      /// signatures that match three image constructors. ReForge will try
      /// to avoid freeing the current data segment and allocating a new one.
      /// This version will cause `this` to be of the requested sizes and
      /// data type.
      ///
      /// If `this` doesn't match the requested properties, it must be stripped
      /// and forged. If `this` is protected (see `dip::Image::Protect`) and
      /// forged, an exception will be thrown by `dip::Image::Strip`. However,
      /// if `acceptDataTypeChange` is `dip::Option::AcceptDataTypeChange::DO_ALLOW`,
      /// a protected image will keep its
      /// old data type, and no exception will be thrown if this data type
      /// is different from `dt`. Note that other properties much still match
      /// if `this` was forged. Thus, this flag allows `this` to control the
      /// data type of the image, ignoring any requested data type here.
      void ReForge(
            UnsignedArray const& sizes,
            dip::uint tensorElems = 1,
            dip::DataType dt = DT_SFLOAT,
            Option::AcceptDataTypeChange acceptDataTypeChange = Option::AcceptDataTypeChange::DONT_ALLOW
      );

      /// \brief Dissasociate the data segment from the image. If there are no
      /// other images using the same data segment, it will be freed.
      void Strip() {
         if( IsForged() ) {
            DIP_THROW_IF( IsProtected(), "Image is protected" );
            dataBlock_ = nullptr; // Automatically frees old memory if no other pointers to it exist.
            origin_ = nullptr;    // Keep this one in sync!
         }
      }

      /// \brief Test if forged.
      bool IsForged() const {
         return origin_ != nullptr;
      }

      /// \brief Set protection flag.
      ///
      /// A protected image cannot be stripped or reforged. See \ref protect "the \"protect\" flag" for more information.
      ///
      /// Returns the old setting. This can be used as follows to temporarily
      /// protect an image:
      ///
      /// ```cpp
      ///     bool wasProtected = img.Protect();
      ///     [...] // do your thing
      ///     img.Protect( wasProtected );
      /// ```
      ///
      /// \see IsProtected, Strip
      bool Protect( bool set = true ) {
         bool old = protect_;
         protect_ = set;
         return old;
      }

      /// \brief Test if protected. See `dip::Image::Protect` for information.
      bool IsProtected() const {
         return protect_;
      }

      // Note: This function is the reason we refer to the ExternalInterface class as
      // dip::ExternalInterface everywhere inside the dip::Image class.

      /// \brief Set external interface pointer. The image must be raw.
      void SetExternalInterface( dip::ExternalInterface* ei ) {
         DIP_THROW_IF( IsForged(), E::IMAGE_NOT_RAW );
         externalInterface_ = ei;
      }

      /// \brief Get external interface pointer.
      dip::ExternalInterface* ExternalInterface() const {
         return externalInterface_;
      }

      /// \brief Test if an external interface is set.
      bool HasExternalInterface() const {
         return externalInterface_ != nullptr;
      }

      /// \}

      //
      // Pointers, offsets, indices
      // Defined in src/library/image_data.cpp
      //

      /// \name Pointers, offsets, indices
      /// \{

      /// \brief Get pointer to the first sample in the image, the first tensor
      /// element at coordinates (0,0,0,...). The image must be forged.
      void* Origin() const {
         DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
         return origin_;
      }

      /// \brief Get a pointer to the pixel given by the offset.
      ///
      /// Cast the pointer to the right type before use. No check is made on the index.
      ///
      /// The image must be forged.
      ///
      /// \see Origin, Offset, OffsetToCoordinates
      void* Pointer( dip::sint offset ) const {
         return static_cast< uint8* >( origin_ ) + offset * dataType_.SizeOf();
      }

      /// \brief Get a pointer to the pixel given by the coordinates index.
      ///
      /// Cast the
      /// pointer to the right type before use. This is not the most efficient
      /// way of indexing many pixels in the image.
      ///
      /// If `coords` is not within the image domain, an exception is thrown.
      ///
      /// The image must be forged.
      ///
      /// \see Origin, Offset, OffsetToCoordinates
      void* Pointer( UnsignedArray const& coords ) const {
         return Pointer( Offset( coords ));
      }

      /// \brief Get a pointer to the pixel given by the coordinates index.
      ///
      /// Cast the
      /// pointer to the right type before use. This is not the most efficient
      /// way of indexing many pixels in the image.
      ///
      /// `coords` can be outside the image domain.
      ///
      /// The image must be forged.
      ///
      /// \see Origin, Offset, OffsetToCoordinates
      void* Pointer( IntegerArray const& coords ) const {
         return Pointer( Offset( coords ));
      }

      /// \brief Compute offset given coordinates.
      ///
      /// The offset needs to be multiplied
      /// by the number of bytes of each sample to become a memory offset
      /// within the image.
      ///
      /// If `coords` is not within the image domain, an exception is thrown.
      ///
      /// The image must be forged.
      ///
      /// \see Origin, Pointer, OffsetToCoordinates
      dip::sint Offset( UnsignedArray const& coords ) const;

      /// \brief Compute offset given coordinates.
      ///
      /// The offset needs to be multiplied
      /// by the number of bytes of each sample to become a memory offset
      /// within the image.
      ///
      /// `coords` can be outside the image domain.
      ///
      /// The image must be forged.
      ///
      /// \see Origin, Pointer, OffsetToCoordinates
      dip::sint Offset( IntegerArray const& coords ) const;

      /// \brief Compute coordinates given an offset.
      ///
      /// If the image has any singleton-expanded
      /// dimensions, the computed coordinate along that dimension will always be 0.
      /// This is an expensive operation, use `dip::Image::OffsetToCoordinatesComputer` to make it
      /// more efficient when performing multiple computations in sequence.
      ///
      /// Note that the coordinates must be inside the image domain, if the offset given
      /// does not correspond to one of the image's pixels, the result is meaningless.
      ///
      /// The image must be forged.
      ///
      /// \see Offset, OffsetToCoordinatesComputer, IndexToCoordinates
      UnsignedArray OffsetToCoordinates( dip::sint offset ) const;

      /// \brief Returns a functor that computes coordinates given an offset.
      ///
      /// This is
      /// more efficient than using `dip::Image::OffsetToCoordinates` when repeatedly
      /// computing offsets, but still requires complex calculations.
      ///
      /// The image must be forged.
      ///
      /// \see Offset, OffsetToCoordinates, IndexToCoordinates, IndexToCoordinatesComputer
      CoordinatesComputer OffsetToCoordinatesComputer() const;

      /// \brief Compute linear index (not offset) given coordinates.
      ///
      /// This index is not
      /// related to the position of the pixel in memory, and should not be used
      /// to index many pixels in sequence.
      ///
      /// The image must be forged.
      ///
      /// \see IndexToCoordinates, Offset
      dip::uint Index( UnsignedArray const& coords ) const;

      /// \brief Compute coordinates given a linear index.
      ///
      /// If the image has any singleton-expanded
      /// dimensions, the computed coordinate along that dimension will always be 0.
      /// This is an expensive operation, use `dip::Image::IndexToCoordinatesComputer` to make it
      /// more efficient when performing multiple computations in sequence.
      ///
      /// Note that the coordinates must be inside the image domain, if the index given
      /// does not correspond to one of the image's pixels, the result is meaningless.
      ///
      /// The image must be forged.
      ///
      /// \see Index, Offset, IndexToCoordinatesComputer, OffsetToCoordinates
      UnsignedArray IndexToCoordinates( dip::uint index ) const;

      /// \brief Returns a functor that computes coordinates given a linear index.
      ///
      /// This is
      /// more efficient than using `dip::Image::IndexToCoordinates`, when repeatedly
      /// computing indices, but still requires complex calculations.
      ///
      /// The image must be forged.
      ///
      /// \see Index, Offset, IndexToCoordinates, OffsetToCoordinates, OffsetToCoordinatesComputer
      CoordinatesComputer IndexToCoordinatesComputer() const;

      /// \}

      //
      // Modifying geometry of a forged image without data copy
      // Defined in src/library/image_manip.cpp
      //

      /// \name Reshaping forged image
      /// \{

      /// \brief Permute dimensions.
      ///
      /// This function allows to re-arrange the dimensions
      /// of the image in any order. It also allows to remove singleton dimensions
      /// (but not to add them, should we add that? how?). For example, given
      /// an image with size `{ 30, 1, 50 }`, and an `order` array of
      /// `{ 2, 0 }`, the image will be modified to have size `{ 50, 30 }`.
      /// Dimension number 1 is not referenced, and was removed (this can only
      /// happen if the dimension has size 1, otherwise an exception will be
      /// thrown!). Dimension 2 was placed first, and dimension 0 was placed second.
      ///
      /// The image must be forged. If it is not, you can simply assign any
      /// new sizes array through Image::SetSizes. The data will never
      /// be copied (i.e. this is a quick and cheap operation).
      ///
      /// \see SwapDimensions, Squeeze, AddSingleton, ExpandDimensionality, Flatten.
      Image& PermuteDimensions( UnsignedArray const& order );

      /// \brief Swap dimensions d1 and d2. This is a simplified version of the
      /// PermuteDimensions.
      ///
      /// The image must be forged, and the data will never
      /// be copied (i.e. this is a quick and cheap operation).
      ///
      /// \see PermuteDimensions.
      Image& SwapDimensions( dip::uint dim1, dip::uint dim2 );

      /// \brief Make image 1D.
      ///
      /// The image must be forged. If HasSimpleStride,
      /// this is a quick and cheap operation, but if not, the data segment
      /// will be copied. Note that the order of the pixels in the
      /// resulting image depend on the strides, and do not necessarily
      /// follow the same order as linear indices.
      ///
      /// \see PermuteDimensions, ExpandDimensionality.
      Image& Flatten();

      /// \brief Remove singleton dimensions (dimensions with size==1).
      ///
      /// The image must be forged, and the data will never
      /// be copied (i.e. this is a quick and cheap operation).
      ///
      /// \see AddSingleton, ExpandDimensionality, PermuteDimensions.
      Image& Squeeze();

      /// \brief Add a singleton dimension (with size==1) to the image.
      ///
      /// Dimensions `dim` to last are shifted up, dimension `dim` will
      /// have a size of 1.
      ///
      /// The image must be forged, and the data will never
      /// be copied (i.e. this is a quick and cheap operation).
      ///
      /// Example: to an image with sizes `{ 4, 5, 6 }` we add a
      /// singleton dimension `dim == 1`. The image will now have
      /// sizes `{ 4, 1, 5, 6 }`.
      ///
      /// \see Squeeze, ExpandDimensionality, PermuteDimensions.
      Image& AddSingleton( dip::uint dim );

      /// \brief Append singleton dimensions to increase the image dimensionality.
      ///
      /// The image will have `n` dimensions. However, if the image already
      /// has `n` or more dimensions, nothing happens.
      ///
      /// The image must be forged, and the data will never
      /// be copied (i.e. this is a quick and cheap operation).
      ///
      /// \see AddSingleton, ExpandSingletonDimension, Squeeze, PermuteDimensions, Flatten.
      Image& ExpandDimensionality( dip::uint dim );

      /// \brief Expand singleton dimension `dim` to `sz` pixels, setting the corresponding
      /// stride to 0.
      ///
      /// If `dim` is not a singleton dimension (size==1), an exception is thrown.
      ///
      /// The image must be forged, and the data will never
      /// be copied (i.e. this is a quick and cheap operation).
      ///
      /// \see AddSingleton, ExpandDimensionality.
      Image& ExpandSingletonDimension( dip::uint dim, dip::uint sz );

      /// \brief Performs singleton expansion.
      ///
      /// The image is modified so that it has `size`
      /// as dimensions. It must be forged and singleton-expandable to `size`,
      /// otherwise an exception is thrown. See `dip::Image::ExpandSingletonDimension`.
      /// `size` is the array as returned by `dip::Framework::SingletonExpandedSize`.
      Image& ExpandSingletonDimensions( UnsignedArray const& newSizes );

      /// \brief Tests if the image can be singleton-expanded to `size`.
      bool IsSingletonExpansionPossible( UnsignedArray const& newSizes ) const;

      /// \brief Expand singleton tensor dimension `sz` samples, setting the tensor
      /// stride to 0.
      ///
      /// If there is more than one tensor element, an exception is thrown.
      ///
      /// The image must be forged, and the data will never be copied (i.e. this is a
      /// quick and cheap operation).
      ///
      /// \see ExpandSingletonDimension.
      Image& ExpandSingletonTensor( dip::uint sz );

      /// \brief Mirror de image about selected axes.
      ///
      /// The image must be forged, and the data will never
      /// be copied (i.e. this is a quick and cheap operation).
      Image& Mirror( BooleanArray const& process );

      /// \brief Change the tensor shape, without changing the number of tensor elements.
      Image& ReshapeTensor( dip::uint rows, dip::uint cols ) {
         DIP_THROW_IF( tensor_.Elements() != rows * cols, "Cannot reshape tensor to requested sizes." );
         tensor_.ChangeShape( rows );
         return *this;
      }

      /// \brief Change the tensor shape, without changing the number of tensor elements.
      Image& ReshapeTensor( dip::Tensor const& other ) {
         tensor_.ChangeShape( other );
         return *this;
      }

      /// \brief Change the tensor to a vector, without changing the number of tensor elements.
      Image& ReshapeTensorAsVector() {
         tensor_.ChangeShape();
         return *this;
      }

      /// \brief Change the tensor to a diagonal matrix, without changing the number of tensor elements.
      Image& ReshapeTensorAsDiagonal() {
         dip::Tensor other{ dip::Tensor::Shape::DIAGONAL_MATRIX, tensor_.Elements(), tensor_.Elements() };
         tensor_.ChangeShape( other );
         return *this;
      }

      /// \brief Transpose the tensor.
      Image& Transpose() {
         tensor_.Transpose();
         return *this;
      }

      /// \brief Convert tensor dimensions to spatial dimension.
      ///
      /// Works even for scalar images, creating a singleton dimension. `dim`
      /// defines the new dimension, subsequent dimensions will be shifted over.
      /// `dim` should not be larger than the number of dimensions. If `dim`
      /// is negative, the new dimension will be the last one. The image must
      /// be forged.
      Image& TensorToSpatial( dip::sint dim = -1 );

      /// \brief Convert spatial dimension to tensor dimensions. The image must be scalar.
      ///
      /// If `rows` or `cols` is zero, its size is computed from the size of the
      /// image along dimension `dim`. If both are zero, a default column tensor
      /// is created. If `dim` is negative, the last dimension is used. The
      /// image must be forged.
      Image& SpatialToTensor( dip::sint dim = -1, dip::uint rows = 0, dip::uint cols = 0 );

      /// \brief Split the two values in a complex sample into separate samples,
      /// creating a new spatial dimension of size 2.
      ///
      /// `dim` defines the new
      /// dimension, subsequent dimensions will be shifted over. `dim` should
      /// not be larger than the number of dimensions. If `dim` is negative,
      /// the new dimension will be the last one. The image must be forged.
      Image& SplitComplex( dip::sint dim = -1 );

      /// \brief Merge the two samples along dimension `dim` into a single complex-valued sample.
      ///
      /// Dimension `dim` must have size 2 and a stride of 1. If `dim` is negative, the last
      /// dimension is used. The image must be forged.
      Image& MergeComplex( dip::sint dim = -1 );

      /// \brief Split the two values in a complex sample into separate samples of
      /// a tensor. The image must be scalar and forged.
      Image& SplitComplexToTensor();

      /// \brief Merge the two samples in the tensor into a single complex-valued sample.
      ///
      /// The image must have two tensor elements, a tensor stride of 1, and be forged.
      Image& MergeTensorToComplex();

      /// \}

      //
      // Creating views of the data -- indexing without data copy
      // Defined in src/library/image_indexing.cpp
      //

      /// \name Indexing without data copy
      /// \{

      /// \brief Extract a tensor element, `indices` must have one or two elements. The image must be forged.
      Image operator[]( UnsignedArray const& indices ) const;

      /// \brief Extract a tensor element using linear indexing. The image must be forged.
      Image operator[]( dip::uint index ) const;

      /// \brief Extracts the tensor elements along the diagonal. The image must be forged.
      Image Diagonal() const;

      /// \brief Extracts the pixel at the given coordinates. The image must be forged.
      Image At( UnsignedArray const& coords ) const;

      /// \brief Extracts the pixel at the given linear index (inneficient!). The image must be forged.
      Image At( dip::uint index ) const;

      /// \brief Extracts a subset of pixels from a 1D image. The image must be forged.
      Image At( Range x_range ) const;

      /// \brief Extracts a subset of pixels from a 2D image. The image must be forged.
      Image At( Range x_range, Range y_range ) const;

      /// \brief Extracts a subset of pixels from a 3D image. The image must be forged.
      Image At( Range x_range, Range y_range, Range z_range ) const;

      /// \brief Extracts a subset of pixels from an image. The image must be forged.
      Image At( RangeArray ranges ) const;

      /// \brief Extracts the real component of a complex-typed image. The image must be forged.
      Image Real() const;

      /// \brief Extracts the imaginary component of a complex-typed image. The image must be forged.
      Image Imaginary() const;

      /// \brief Quick copy, returns a new image that points at the same data as `this`,
      /// and has mostly the same properties.
      ///
      /// The color space and pixel size
      /// information are not copied, and the protect flag is reset.
      /// This function is mostly meant for use in functions that need to
      /// modify some properties of the input images, without actually modifying
      /// the input images.
      Image QuickCopy() const {
         Image out;
         out.dataType_ = dataType_;
         out.sizes_ = sizes_;
         out.strides_ = strides_;
         out.tensor_ = tensor_;
         out.tensorStride_ = tensorStride_;
         out.dataBlock_ = dataBlock_;
         out.origin_ = origin_;
         out.externalInterface_ = externalInterface_;
         return out;
      }

      /// \}

      //
      // Getting/setting pixel values
      // Defined in src/library/image_data.cpp
      //

      /// \name Getting and setting pixel values
      /// \{

      /// \brief Creates a 1D image containing the pixels selected by `mask`.
      ///
      /// The values are copied, not referenced. The output image will be of the same data type and tensor shape
      /// as `this`, but have only one dimension. Pixels will be read from `mask` in the linear index order.
      ///
      /// `this` must be forged and be of equal size as `mask`. `mask` is a scalar binary image.
      Image CopyAt( Image const& mask ) const;

      /// \brief Creates a 1D image containing the pixels selected by `indices`.
      ///
      /// The values are copied, not referenced. The output image will be of the same data type and tensor shape
      /// as `this`, but have only one dimension. It will have as many pixels as indices are in `indices`, and
      /// be sorted in the same order.
      ///
      /// `indices` contains linear indices into the image. Note that converting indices into offsets is not a
      /// trivial operation; prefer to use the version of this function that uses coordinates.
      ///
      /// `this` must be forged.
      Image CopyAt( UnsignedArray const& indices ) const;

      /// \brief Creates a 1D image containing the pixels selected by `coordinates`.
      ///
      /// The values are copied, not referenced. The output image will be of the same data type and tensor shape
      /// as `this`, but have only one dimension. It will have as many pixels as coordinates are in `coordinates`,
      /// and be sorted in the same order.
      ///
      /// Each of the coordinates must have the same number of dimensions as `this`.
      ///
      /// `this` must be forged.
      Image CopyAt( CoordinateArray const& coordinates ) const;

      /// \brief Copies the pixel values from `source` into `this`, to the pixels selected by `mask`.
      ///
      /// `source` must have the same number of tensor elements as `this`. The pixel values will be
      /// cast to match the data type of `this`, where values are clipped to the target range and/or
      /// truncated, as applicable. Complex values are converted to non-complex values by taking the
      /// absolute value.
      ///
      /// Pixels selected by `mask` are taken in the linear index order.
      ///
      /// `source` is expected to have the same number of pixels as are selected by `mask`, but this is
      /// not tested for before the copy begins unless `throws` is set to `dip::Option::ThrowException::DO_THROW`.
      /// Note that this tests costs extra time, and often is not necessary.
      /// By default, the pixels are copied until either all `source` pixels have been copied or until all
      /// pixels selected by `mask` have been written to.
      ///
      /// `this` must be forged and be of equal size as `mask`. `mask` is a scalar binary image.
      void CopyAt( Image const& source, Image const& mask, Option::ThrowException throws = Option::ThrowException::DONT_THROW );

      /// \brief Copies the pixel values from `source` into `this`, to the pixels selected by `indices`.
      ///
      /// `source` must have the same number of tensor elements as `this`. The pixel values will be
      /// cast to match the data type of `this`, where values are clipped to the target range and/or
      /// truncated, as applicable. Complex values are converted to non-complex values by taking the
      /// absolute value.
      ///
      /// `source` must have the same number of pixels as indices are in `indices`, and are used in the
      /// same order. `indices` contains linear indices into the image. Note that converting indices
      /// into offsets is not a trivial operation; prefer to use the version of this function that uses coordinates.
      ///
      /// `this` must be forged.
      void CopyAt( Image const& source, UnsignedArray const& indices );

      /// \brief Copies the pixel values from `source` into `this`, to the pixels selected by `coordinates`.
      ///
      /// `source` must have the same number of tensor elements as `this`. The pixel values will be
      /// cast to match the data type of `this`, where values are clipped to the target range and/or
      /// truncated, as applicable. Complex values are converted to non-complex values by taking the
      /// absolute value.
      ///
      /// `source` must have the same number of pixels as coordinates are in `coordinates`, and are used
      /// in the same order. Each of the coordinates must have the same number of dimensions as `this`.
      ///
      /// `this` must be forged.
      void CopyAt( Image const& source, CoordinateArray const& coordinates );

      /// \brief Deep copy, `this` will become a copy of `src` with its own data.
      ///
      /// If `this` is forged, then `src` is expected to have the same sizes
      /// and number of tensor elements, and the data is copied over from `src`
      /// to `this`. The copy will apply data type conversion, where values are
      /// clipped to the target range and/or truncated, as applicable. Complex
      /// values are converted to non-complex values by taking the absolute
      /// value.
      ///
      /// If `this` is not forged, then all the properties of `src` will be
      /// copied to `this`, `this` will be forged, and the data from `src` will
      /// be copied over.
      ///
      /// `src` must be forged.
      void Copy( Image const& src );

      /// \brief Converts the image to another data type.
      ///
      /// The data segment is replaced by a new one, unless the old and new data
      /// types have the same size and it is not shared with other images.
      /// If the data segment is replaced, strides are set to normal.
      void Convert( dip::DataType dt );

      /// \brief Sets all samples in the image to the value `v`.
      ///
      /// The function is defined for values `v` of type bool, int, dip::uint, dip::sint,
      /// dip::dfloat (double), and dip::dcomplex.
      /// The value will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged.
      void Fill( bool v );

      /// \brief Sets all samples in the image to the value `v`.
      ///
      /// The function is defined for values `v` of type bool, int, dip::uint, dip::sint,
      /// dip::dfloat (double), and dip::dcomplex.
      /// The value will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged.
      void Fill( int v );

      /// \brief Sets all samples in the image to the value `v`.
      ///
      /// The function is defined for values `v` of type bool, int, dip::uint, dip::sint,
      /// dip::dfloat (double), and dip::dcomplex.
      /// The value will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged.
      void Fill( dip::uint v );

      /// \brief Sets all samples in the image to the value `v`.
      ///
      /// The function is defined for values `v` of type bool, int, dip::uint, dip::sint,
      /// dip::dfloat (double), and dip::dcomplex.
      /// The value will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged.
      void Fill( dip::sint v );

      /// \brief Sets all samples in the image to the value `v`.
      ///
      /// The function is defined for values `v` of type bool, int, dip::uint, dip::sint,
      /// dip::dfloat (double), and dip::dcomplex.
      /// The value will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged.
      void Fill( dfloat v );

      /// \brief Sets all samples in the image to the value `v`.
      ///
      /// The function is defined for values `v` of type bool, int, dip::uint, dip::sint,
      /// dip::dfloat (double), and dip::dcomplex.
      /// The value will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged.
      void Fill( dcomplex v );

      /// \brief Sets all pixels in the image to the values `vlist`.
      ///
      /// The function is defined for initializer lists `vlist` of type bool, int, dip::uint,
      /// dip::sint, dip::dfloat (double), and dip::dcomplex.
      /// The values will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged and have the same number of tensor elements as
      /// elements in `vlist`.
      void Fill( std::initializer_list< bool > const& vlist ) {
         DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( vlist.size() != tensor_.Elements(), E::INITIALIZERLIST_ILLEGAL_SIZE );
         Image tmp = QuickCopy();
         tmp.tensor_.SetScalar();
         for( auto v = vlist.begin(); v != vlist.end(); ++v, tmp.origin_ = tmp.Pointer( tmp.tensorStride_ )) {
            // NOTE: tmp.Pointer( tmp.tensorStride_ ) takes the current tmp.origin_ and adds the tensor stride to it.
            // Thus, assigning this into tmp.origin_ is equivalent to tmp.origin += tmp_tensorStride_ if tmp.origin_
            // were a pointer to the correct data type.
            tmp.Fill( *v );
         }
      }

      /// \brief Sets all pixels in the image to the values `vlist`.
      ///
      /// The function is defined for initializer lists `vlist` of type bool, int, dip::uint,
      /// dip::sint, dip::dfloat (double), and dip::dcomplex.
      /// The values will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged and have the same number of tensor elements as
      /// elements in `vlist`.
      void Fill( std::initializer_list< int > const& vlist ) {
         DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( vlist.size() != tensor_.Elements(), E::INITIALIZERLIST_ILLEGAL_SIZE );
         Image tmp = QuickCopy();
         tmp.tensor_.SetScalar();
         for( auto v = vlist.begin(); v != vlist.end(); ++v, tmp.origin_ = tmp.Pointer( tmp.tensorStride_ )) {
            // NOTE: tmp.Pointer( tmp.tensorStride_ ) takes the current tmp.origin_ and adds the tensor stride to it.
            // Thus, assigning this into tmp.origin_ is equivalent to tmp.origin += tmp_tensorStride_ if tmp.origin_
            // were a pointer to the correct data type.
            tmp.Fill( *v );
         }
      }

      /// \brief Sets all pixels in the image to the values `vlist`.
      ///
      /// The function is defined for initializer lists `vlist` of type bool, int, dip::uint,
      /// dip::sint, dip::dfloat (double), and dip::dcomplex.
      /// The values will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged and have the same number of tensor elements as
      /// elements in `vlist`.
      void Fill( std::initializer_list< dip::uint > const& vlist ) {
         DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( vlist.size() != tensor_.Elements(), E::INITIALIZERLIST_ILLEGAL_SIZE );
         Image tmp = QuickCopy();
         tmp.tensor_.SetScalar();
         for( auto v = vlist.begin(); v != vlist.end(); ++v, tmp.origin_ = tmp.Pointer( tmp.tensorStride_ )) {
            // NOTE: tmp.Pointer( tmp.tensorStride_ ) takes the current tmp.origin_ and adds the tensor stride to it.
            // Thus, assigning this into tmp.origin_ is equivalent to tmp.origin += tmp_tensorStride_ if tmp.origin_
            // were a pointer to the correct data type.
            tmp.Fill( *v );
         }
      }

      /// \brief Sets all pixels in the image to the values `vlist`.
      ///
      /// The function is defined for initializer lists `vlist` of type bool, int, dip::uint,
      /// dip::sint, dip::dfloat (double), and dip::dcomplex.
      /// The values will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged and have the same number of tensor elements as
      /// elements in `vlist`.
      void Fill( std::initializer_list< dip::sint > const& vlist ) {
         DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( vlist.size() != tensor_.Elements(), E::INITIALIZERLIST_ILLEGAL_SIZE );
         Image tmp = QuickCopy();
         tmp.tensor_.SetScalar();
         for( auto v = vlist.begin(); v != vlist.end(); ++v, tmp.origin_ = tmp.Pointer( tmp.tensorStride_ )) {
            // NOTE: tmp.Pointer( tmp.tensorStride_ ) takes the current tmp.origin_ and adds the tensor stride to it.
            // Thus, assigning this into tmp.origin_ is equivalent to tmp.origin += tmp_tensorStride_ if tmp.origin_
            // were a pointer to the correct data type.
            tmp.Fill( *v );
         }
      }

      /// \brief Sets all pixels in the image to the values `vlist`.
      ///
      /// The function is defined for initializer lists `vlist` of type bool, int, dip::uint,
      /// dip::sint, dip::dfloat (double), and dip::dcomplex.
      /// The values will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged and have the same number of tensor elements as
      /// elements in `vlist`.
      void Fill( std::initializer_list< dfloat > const& vlist ) {
         DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( vlist.size() != tensor_.Elements(), E::INITIALIZERLIST_ILLEGAL_SIZE );
         Image tmp = QuickCopy();
         tmp.tensor_.SetScalar();
         for( auto v = vlist.begin(); v != vlist.end(); ++v, tmp.origin_ = tmp.Pointer( tmp.tensorStride_ )) {
            tmp.Fill( *v );
         }
      }

      /// \brief Sets all pixels in the image to the values `vlist`.
      ///
      /// The function is defined for initializer lists `vlist` of type bool, int, dip::uint,
      /// dip::sint, dip::dfloat (double), and dip::dcomplex.
      /// The values will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged and have the same number of tensor elements as
      /// elements in `vlist`.
      void Fill( std::initializer_list< dcomplex > const& vlist ) {
         DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( vlist.size() != tensor_.Elements(), E::INITIALIZERLIST_ILLEGAL_SIZE );
         Image tmp = QuickCopy();
         tmp.tensor_.SetScalar();
         for( auto v = vlist.begin(); v != vlist.end(); ++v, tmp.origin_ = tmp.Pointer( tmp.tensorStride_ )) {
            tmp.Fill( *v );
         }
      }

      /// \brief Sets all samples in the image to the value `v`.
      ///
      /// The operator is defined for values `v` of type bool, int, dip::uint, dip::sint,
      /// dip::dfloat (double), and dip::dcomplex.
      /// The value will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged.
      Image& operator=( bool v ) {
         Fill( v );
         return *this;
      }

      /// \brief Sets all samples in the image to the value `v`.
      ///
      /// The operator is defined for values `v` of type bool, int, dip::uint, dip::sint,
      /// dip::dfloat (double), and dip::dcomplex.
      /// The value will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged.
      Image& operator=( int v ) {
         Fill( v );
         return *this;
      }

      /// \brief Sets all samples in the image to the value `v`.
      ///
      /// The operator is defined for values `v` of type bool, int, dip::uint, dip::sint,
      /// dip::dfloat (double), and dip::dcomplex.
      /// The value will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged.
      Image& operator=( dip::uint v ) {
         Fill( v );
         return *this;
      }

      /// \brief Sets all samples in the image to the value `v`.
      ///
      /// The operator is defined for values `v` of type bool, int, dip::uint, dip::sint,
      /// dip::dfloat (double), and dip::dcomplex.
      /// The value will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged.
      Image& operator=( dip::sint v ) {
         Fill( v );
         return *this;
      }

      /// \brief Sets all samples in the image to the value `v`.
      ///
      /// The operator is defined for values `v` of type bool, int, dip::uint, dip::sint,
      /// dip::dfloat (double), and dip::dcomplex.
      /// The value will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged.
      Image& operator=( dfloat v ) {
         Fill( v );
         return *this;
      }

      /// \brief Sets all samples in the image to the value `v`.
      ///
      /// The operator is defined for values `v` of type bool, int, dip::uint, dip::sint,
      /// dip::dfloat (double), and dip::dcomplex.
      /// The value will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged.
      Image& operator=( dcomplex v ) {
         Fill( v );
         return *this;
      }

      /// \brief Sets all pixels in the image to the values `vlist`.
      ///
      /// The operator is defined for initializer lists `vlist` of type bool, int, dip::uint,
      /// dip::sint, dip::dfloat (double), and dip::dcomplex.
      /// The values will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged and have the same number of tensor elements as
      /// elements in `vlist`.
      Image& operator=( std::initializer_list< bool > const& vlist ) {
         Fill( vlist );
         return *this;
      }

      /// \brief Sets all pixels in the image to the values `vlist`.
      ///
      /// The operator is defined for initializer lists `vlist` of type bool, int, dip::uint,
      /// dip::sint, dip::dfloat (double), and dip::dcomplex.
      /// The values will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged and have the same number of tensor elements as
      /// elements in `vlist`.
      Image& operator=( std::initializer_list< int > const& vlist ) {
         Fill( vlist );
         return *this;
      }

      /// \brief Sets all pixels in the image to the values `vlist`.
      ///
      /// The operator is defined for initializer lists `vlist` of type bool, int, dip::uint,
      /// dip::sint, dip::dfloat (double), and dip::dcomplex.
      /// The values will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged and have the same number of tensor elements as
      /// elements in `vlist`.
      Image& operator=( std::initializer_list< dip::uint > const& vlist ) {
         Fill( vlist );
         return *this;
      }

      /// \brief Sets all pixels in the image to the values `vlist`.
      ///
      /// The operator is defined for initializer lists `vlist` of type bool, int, dip::uint,
      /// dip::sint, dip::dfloat (double), and dip::dcomplex.
      /// The values will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged and have the same number of tensor elements as
      /// elements in `vlist`.
      Image& operator=( std::initializer_list< dip::sint > const& vlist ) {
         Fill( vlist );
         return *this;
      }

      /// \brief Sets all pixels in the image to the values `vlist`.
      ///
      /// The operator is defined for initializer lists `vlist` of type bool, int, dip::uint,
      /// dip::sint, dip::dfloat (double), and dip::dcomplex.
      /// The values will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged and have the same number of tensor elements as
      /// elements in `vlist`.
      Image& operator=( std::initializer_list< dfloat > const& vlist ) {
         Fill( vlist );
         return *this;
      }

      /// \brief Sets all pixels in the image to the values `vlist`.
      ///
      /// The operator is defined for initializer lists `vlist` of type bool, int, dip::uint,
      /// dip::sint, dip::dfloat (double), and dip::dcomplex.
      /// The values will be clipped to the target range and/or truncated, as applicable.
      /// The image must be forged and have the same number of tensor elements as
      /// elements in `vlist`.
      Image& operator=( std::initializer_list< dcomplex > const& vlist ) {
         Fill( vlist );
         return *this;
      }

      /// \brief Extracts the fist sample in the first pixel (At(0,0)[0]), casted
      /// to a signed integer of maximum width. For complex values
      /// returns the absolute value.
      explicit operator dip::sint() const;

      /// \brief Extracts the fist sample in the first pixel (At(0,0)[0]), casted
      /// to a double-precision floating-point value. For complex values
      /// returns the absolute value.
      explicit operator dfloat() const;

      /// \brief Extracts the fist sample in the first pixel (At(0,0)[0]), casted
      /// to a double-precision complex floating-point value.
      explicit operator dcomplex() const;

      /// \}

      // Some friend declarations
      friend class dip::ImageSliceIterator;

   private:

      //
      // Implementation
      //

      dip::DataType dataType_ = DT_SFLOAT;
      UnsignedArray sizes_;               // sizes_.size() == ndims (if forged)
      IntegerArray strides_;              // strides_.size() == ndims (if forged)
      dip::Tensor tensor_;
      dip::sint tensorStride_ = 0;
      bool protect_ = false;              // When set, don't strip image
      String colorSpace_;
      dip::PixelSize pixelSize_;
      std::shared_ptr< void > dataBlock_; // Holds the pixel data. Data block will be freed when last image that uses it is destroyed.
      void* origin_ = nullptr;            // Points to the origin ( pixel (0,0) ), not necessarily the first pixel of the data block.
      dip::ExternalInterface* externalInterface_ = nullptr; // A function that will be called instead of the default forge function.

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


//
// Overloaded operators
//

/// \brief You can output a `dip::Image` to `std::cout` or any other stream. Some
/// information about the image is printed.
std::ostream& operator<<( std::ostream& os, Image const& img );

//
// Utility functions
//

inline void swap( Image& v1, Image& v2 ) {
   v1.swap( v2 );
}

/// \brief Calls `img1.Aliases( img2 )`. See `dip::Image::Aliases`.
inline bool Alias( Image const& img1, Image const& img2 ) {
   return img1.Aliases( img2 );
}

/// \brief Makes a new image object pointing to same pixel data as `src`, but
/// with different origin, strides and size (backwards compatibility
/// function, we recommend the `dip::Image::At` function instead).
void DefineROI(
      Image const& src,
      Image& dest,
      UnsignedArray const& origin,
      UnsignedArray const& sizes,
      IntegerArray const& spacing
);

inline Image DefineROI(
      Image const& src,
      UnsignedArray const& origin,
      UnsignedArray const& sizes,
      IntegerArray const& spacing
) {
   Image dest;
   DefineROI( src, dest, origin, sizes, spacing );
   return dest;
}

/// \brief Copies samples over from `src` to `dest`, identical to the `dip::Image::Copy` method.
inline void Copy( Image const& src, Image& dest ) {
   dest.Copy( src );
}

inline Image Copy( Image const& src ) {
   Image dest;
   dest.Copy( src );
   return dest;
}

/// \brief Copies samples over from `src` to `dest`, expanding the tensor so it's a standard, column-major matrix.
///
/// If the tensor representation in `src` is one of those that do not save symmetric or zero values, to save space,
/// a new data segment will be allocated for `dest`, where the tensor representation is a column-major matrix
/// (`dest` will have `dip::Tensor::HasNormalOrder` be true).
/// This function simplifies manipulating tensors by normalizing their storage.
///
/// Creates an identical copy, if the tensor representation is scalar, vector, or a column-major matrix. Note that
/// this copy does not share data, a new data segment is always used, unless `src` and `dest` are the same object.
///
/// If `dest` is protected, it will keep its data type, and the data will be converted as usual to the destination
/// data type.
///
/// \see Copy, Convert
void ExpandTensor( Image const& src, Image& dest );

inline Image ExpandTensor( Image const& src ) {
   Image dest;
   ExpandTensor( src, dest );
   return dest;
}

/// \brief Copies samples over from `src` to `dest`, with data type conversion.
///
/// If `dest` is forged,
/// has the same size as number of tensor elements as `src`, and has data type `dt`, then
/// its data segment is reused. If `src` and `dest` are the same object, its `dip::Image::Convert`
/// method is called instead.
inline void Convert(
      Image const& src,
      Image& dest,
      dip::DataType dt
) {
   if( &src == &dest ) {
      dest.Convert( dt );
   } else {
      dest.ReForge( src, dt );
      dest.Copy( src );
   }
}

inline Image Convert( Image const& src, dip::DataType dt ) {
   Image dest;
   dest.ReForge( src, dt );
   dest.Copy( src );
   return dest;
}

/// \}

} // namespace dip

#endif // DIP_IMAGE_H
