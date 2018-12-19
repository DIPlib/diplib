/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2018, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


//
// NOTE!
// This file is included through diplib.h -- no need to include directly
//


#ifndef DIP_IMAGE_H
#define DIP_IMAGE_H

#include <memory>
#include <functional>
#include <cstring> // std::memcpy

#include "diplib/library/tensor.h"
#include "diplib/library/physical_dimensions.h"
#include "diplib/library/clamp_cast.h"
#include "diplib/library/sample_iterator.h"
#include "diplib/library/stringparams.h"


/// \file
/// \brief The `dip::Image` class and support functions. This file is always included through `diplib.h`.
/// \see infrastructure


namespace dip {


// Forward declarations
class DIP_NO_EXPORT Image;                      // in this file
template< typename T >
class DIP_NO_EXPORT GenericImageIterator;       // in `generic_iterators.h`, include explicitly if needed
template< dip::uint N, typename T >
class DIP_NO_EXPORT GenericJointImageIterator;  // in `generic_iterators.h`, include explicitly if needed


/// \addtogroup infrastructure
/// \{


//
// Support for external interfaces
//

/// \brief A `dip::Image` holds a shared pointer to the data segment using this type.
using DataSegment = std::shared_ptr< void >;

/// \brief This function converts a pointer to a `dip::DataSegment` that does not own the data pointed to.
inline DataSegment NonOwnedRefToDataSegment( void* ptr ) {
   return DataSegment{ ptr, []( void* ){} };
}

/// \brief This function converts a pointer to a `dip::DataSegment` that does not own the data pointed to.
inline DataSegment NonOwnedRefToDataSegment( void const* ptr ) {
   return DataSegment{ const_cast< void* >( ptr ), []( void* ){} };
}

/// \brief Support for external interfaces.
///
/// Software using *DIPlib* might want to control how the image data is allocated. Such software
/// should derive a class from this one, and assign a pointer to it into each of the images that
/// it creates, through `dip::Image::SetExternalInterface`. The caller will maintain ownership of
/// the interface.
///
/// See \ref external_interface for details on how to use the external interfaces.
class DIP_CLASS_EXPORT ExternalInterface {
   public:
      /// Allocates the data for an image. The function is required to set `strides`,
      /// `tensorStride` and `origin`, and return a `dip::DataSegment` that owns the
      /// allocated data segment. Note that `strides` and `tensorStride` might have
      /// been set by the user before calling `dip::Image::Forge`, and should be honored
      /// if possible.
      virtual DataSegment AllocateData(
            void*& origin,
            dip::DataType dataType,
            UnsignedArray const& sizes,
            IntegerArray& strides,
            dip::Tensor const& tensor,
            dip::sint& tensorStride
      ) = 0;
};

/// \brief ExternalInterface that allocates aligned data.
///
/// The class is designed as a singleton: `dip::AlignedAllocInterface::GetInstance()`
/// returns a pointer to a unique instance.
/// The alignment, in bytes, is passed to `dip::AlignedAllocInterface::GetInstance()`
/// as a template parameter.
/// For example:
/// ```cpp
///     // Obtain ExternalInterface pointer that guarantees 64-byte alignment:
///     ExternalInterface* ei = dip::AlignedAllocInterface::GetInstance<64>();
/// ```
///
/// Note: this interface is only suitable for allocating blocks of memory that are (much)
/// larger than the alignment size. Internally, the class allocates an oversized memory block
/// padded with `alignment`, and returns an aligned pointer within that oversized block.
class DIP_CLASS_EXPORT AlignedAllocInterface : public ExternalInterface {
   private:
      // Private constructor to enforce the singleton interface
      explicit AlignedAllocInterface( size_t alignment ) : alignment_( alignment ) {}

   protected:
      class DIP_NO_EXPORT Deleter {
         protected:
            // The pointer to the unaligned data must be freed, not the aligned pointer -> store it
            void* pUnaligned_;

         public:
            // Construct Deleter with pointer to unaligned memory
            Deleter( void* pUnaligned ) : pUnaligned_( pUnaligned ) {}
            // Deletes the memory of a DataSegment allocated by AlignedAllocInterface::AllocateData()
            DIP_EXPORT void operator()( void* pAligned );
      };

      // Alignment in bytes
      size_t alignment_;

   public:
      /// Called by `dip::Image::Forge`.
      DIP_EXPORT virtual DataSegment AllocateData(
            void*& origin,
            dip::DataType dataType,
            UnsignedArray const& sizes,
            IntegerArray& strides,
            dip::Tensor const& tensor,
            dip::sint& tensorStride
      ) override;

      /// Singleton interface, templated in the alignment parameter.
      /// Only one instance is needed for each distinct alignment.
      /// `alignment` is in bytes.
      template< size_t alignment >
      static AlignedAllocInterface* GetInstance() {
         static AlignedAllocInterface ei( alignment );
         return &ei;
      }
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
class DIP_NO_EXPORT CoordinatesComputer {
   public:
      DIP_EXPORT CoordinatesComputer( UnsignedArray const& sizes, IntegerArray const& strides );

      DIP_EXPORT UnsignedArray operator()( dip::sint offset ) const;

   private:
      IntegerArray strides_; // a copy of the image's strides array, but with all positive values
      IntegerArray sizes_;   // a copy of the image's sizes array, but with negative values where the strides are negative
      UnsignedArray index_;  // sorted indices to the strides array (largest to smallest)
      dip::sint offset_;     // offset needed to handle negative strides
};


//
// The Image class
//


/// \brief An array of images
using ImageArray = std::vector< Image >;

/// \brief An array of image references
using ImageRefArray = std::vector< std::reference_wrapper< Image >>;

/// \brief An array of const image references
using ImageConstRefArray = std::vector< std::reference_wrapper< Image const >>;

// The class is documented in the file src/documentation/image.md
class DIP_NO_EXPORT Image {

   public:

      //
      // Pixels and Samples. Find the implementation in pixel.h.
      //

      class Sample;
      class Pixel;
      class View;
      template< class T > class CastSample;
      template< class T > class CastPixel;

      //
      // Constructor
      //

      /// \name Constructors
      /// \{

      /// \brief The default-initialized image is 0D (an empty sizes array), one tensor element, dip::DT_SFLOAT,
      /// and raw (it has no data segment).
      Image() = default;

      // Copy constructor, move constructor and destructor are all default.
      Image( Image const& ) = default;
      Image( Image&& ) = default;
      ~Image() = default;

      /// \brief Copy assignment
      ///
      /// Copies the data if the LHS (`this`) is protected or has an external interface set, and this external
      /// interface is different from the one in `rhs` (see \ref protect and \ref external_interface).
      /// In this case, `rhs` will not be modified.
      ///
      /// Otherwise, `this` and `rhs` will share the data segment. See \ref assignment.
      ///
      /// The `protect` flag will not be copied over.
      Image& operator=( Image const& rhs ) {
         if( protect_ || ( externalInterface_ && ( externalInterface_ != rhs.externalInterface_ ))) {
            // Copy pixel data too
            DIP_STACK_TRACE_THIS( this->Copy( rhs ));
         } else {
            // Do what the default copy assignment would do
            dataType_ = rhs.dataType_;
            sizes_ = rhs.sizes_;
            strides_ = rhs.strides_;
            tensor_ = rhs.tensor_;
            tensorStride_ = rhs.tensorStride_;
            colorSpace_ = rhs.colorSpace_;
            pixelSize_ = rhs.pixelSize_;
            dataBlock_ = rhs.dataBlock_;
            origin_ = rhs.origin_;
            externalData_ = rhs.externalData_;
            externalInterface_ = rhs.externalInterface_;
         }
         return *this;
      }

      /// \brief Move assignment
      ///
      /// Copies the data if the LHS (`this`) is protected or has an external interface set, and this external
      /// interface is different from the one in `rhs` (see \ref protect and \ref external_interface).
      /// In this case, `rhs` will not be modified.
      ///
      /// Otherwise, `this` will become exactly what `rhs` was, and `rhs` will become raw.
      Image& operator=( Image&& rhs ) {
         if( protect_ || ( externalInterface_ && ( externalInterface_ != rhs.externalInterface_ ))) {
            // Copy pixel data too
            DIP_STACK_TRACE_THIS( this->Copy( rhs ));
         } else {
            // Do what the default move assignment would do
            this->move( std::move( rhs ));
         }
         return *this;
      }

      /// \brief Forged image of given sizes and data type. The data is left uninitialized.
      ///
      /// Note that to call this constructor with a single parameter, you need to explicitly type the parameter,
      /// an initializer list by itself will be considered a pixel, see the constructor below.
      explicit Image( UnsignedArray sizes, dip::uint tensorElems = 1, dip::DataType dt = DT_SFLOAT ) :
            dataType_( dt ),
            sizes_( std::move( sizes )),
            tensor_( tensorElems ) {
         TestSizes( sizes_ );
         Forge();
      }

      /// \brief Create a 0-D image with the data type, tensor shape, and values of `pixel`.
      ///
      /// Note that `pixel` can be created through an initializer list. Thus, the following
      /// is a valid way of creating a 0-D tensor image with 3 tensor components:
      ///
      /// ```cpp
      ///     dip::Image image( { 10.0f, 1.0f, 0.0f } );
      /// ```
      ///
      /// The image in the example above will be of type `dip::DT_SFLOAT`.
      explicit Image( Pixel const& pixel );

      /// \brief Create a 0-D image with data type `dt`, and tensor shape and values of `pixel`.
      ///
      /// Note that `pixel` can be created through an initializer list. Thus, the following
      /// is a valid way of creating a 0-D tensor image with 3 tensor components:
      ///
      /// ```cpp
      ///     dip::Image image( { 10, 1, 0 }, dip::DT_SFLOAT );
      /// ```
      ///
      /// The image in the example above will be of type `dip::DT_SFLOAT`.
      explicit Image( Pixel const& pixel, dip::DataType dt );

      /// \brief Create a 0-D image with the data type and value of `sample`.
      ///
      /// Note that `sample` can be created by implicit cast from any numeric value. Thus, the following
      /// are valid ways of creating a 0-D image:
      ///
      /// ```cpp
      ///     dip::Image image( 10.0f );
      ///     dip::Image complex_image( dip::dcomplex( 3, 4 ));
      /// ```
      ///
      /// The images in the examples above will be of type `dip::DT_SFLOAT` and `dip::DCOMPLEX`.
      explicit Image( Sample const& sample );

      /// \brief Create a 0-D image with data type `dt` and value of `sample`.
      ///
      /// Note that `sample` can be created by implicit cast from any numeric value. Thus, the following
      /// is a valid way of creating a 0-D image:
      ///
      /// ```cpp
      ///     dip::Image image( 10, dip::DT_SFLOAT );
      /// ```
      ///
      /// The image in the example above will be of type `dip::DT_SFLOAT`.
      explicit Image( Sample const& sample, dip::DataType dt );

      /// \brief Create a 0-D vector image with data type `dt`, and values of `values`.
      ///
      /// Note that if `values` is specified as an initializer list, the constructor `Image( Pixel const& pixel )`
      /// is called instead.
      ///
      /// Note also that this constructor is specifically with a `FloatArray`. If the array is of type
      /// `UnsignedArray`, a different constructor will be called, and the array will be interpreted as image
      /// sizes, not sample values.
      explicit Image( FloatArray const& values, dip::DataType dt = DT_SFLOAT );

      // This one is to disambiguate calling with a single initializer list. We don't mean UnsignedArray, we mean Pixel.
      template< typename T, typename = std::enable_if_t< IsSampleType< T >::value >>
      explicit Image( std::initializer_list< T > values ) : Image( Pixel( values )) {}

      // This one is to disambiguate calling with a single initializer list. We don't mean UnsignedArray, we mean Pixel.
      template< typename T, typename = std::enable_if_t< IsSampleType< T >::value >>
      explicit Image( std::initializer_list< T > values, dip::DataType dt ) : Image( Pixel( values ), dt ) {}

      /// \brief A `dip::Image::View` implicitly converts to an `%Image`.
      Image( View const& view );

      /// \brief A `dip::Image::View` implicitly converts to an `%Image`.
      Image( View&& view );

      /// \brief Create an image around existing data.
      ///
      /// `data` is a shared pointer used to manage the lifetime of the data segment.
      /// If the image is supposed to take ownership, put a pointer to the data segment or the object
      /// that owns it in `data`, with a deleter function that will delete the data segment or object
      /// when the image is stripped or deleted. Otherwise, use `dip::NonOwnedRefToDataSegment` to
      /// create a shared pointer without a deleter function, implying ownership is not transferred.
      ///
      /// `origin` is the pointer to the first pixel. It must be a valid pointer. This is typically,
      /// but not necessarily, the same pointer as used in `data`.
      ///
      /// `dataType` and `sizes` must be set appropriately. `strides` must either have the same number
      /// of elements as `sizes`, or be an empty array. If `strides` is an empty array, normal strides
      /// will be assumed (i.e. row-major, with tensor elements for one pixel stored contiguously). In
      /// this case, `tensorStride` will be ignored. `tensor` defaults to scalar (i.e. a single tensor
      /// element). No tests will be performed on the validity of the values passed in,
      /// except to enforce a few class invariants.
      ///
      /// See \ref external_interface for information about the `externalInterface` parameter.
      ///
      /// See \ref use_external_data for more information on how to use this constructor.
      ///
      /// See `dip::ConstructImage` for a simplified interface to this constructor.
      Image(
            DataSegment const& data,
            void* origin,
            dip::DataType dataType,
            UnsignedArray sizes,
            IntegerArray strides = {},
            dip::Tensor const& tensor = {},
            dip::sint tensorStride = 1,
            dip::ExternalInterface* externalInterface = nullptr
      ) :
            dataType_( dataType ),
            sizes_( std::move( sizes )),
            strides_( std::move( strides )),
            tensor_( tensor ),
            tensorStride_( tensorStride ),
            dataBlock_( data ),
            externalData_( true ),
            externalInterface_( externalInterface ) {
         DIP_THROW_IF( data.get() == nullptr, "Bad data pointer" );
         DIP_THROW_IF( origin == nullptr, "Bad origin pointer" );
         TestSizes( sizes_ );
         dip::uint nDims = sizes_.size();
         if( strides_.empty() ) {
            SetNormalStrides();
         } else {
            DIP_THROW_IF( strides_.size() != nDims, "Strides array size does not match image dimensionality" );
         }
         origin_ = origin; // Set this after calling `SetNormalStrides`. It's not forged until now.
      }

      /// \brief Create an image around existing data. No ownership is transferred.
      ///
      /// `data` is a raw pointer to the data that will be encapsulated by the output image. Normal strides
      /// will be assumed. That is, the data is contiguous and in row-major order. `sizes` indicates the
      /// size of each dimension in the data. `data` must point to a buffer that is at least `sizes.product()`
      /// elements long. The output image is scalar.
      ///
      /// To encapsulate data in a different format, or to transfer ownership of the data to the image, see
      /// the previous constructor.
      ///
      /// See \ref use_external_data for more information on how to use this function.
      ///
      /// **Note:** There is no way to make the data segment in an image read-only. It is possible to use this
      /// function to create an image around const data, and then write to that data. Use images pointing to
      /// const data only as input images!
      template< typename T, typename = std::enable_if_t< IsSampleType< T >::value >>
      Image( T const* data, UnsignedArray sizes ) {
         Image tmp{ NonOwnedRefToDataSegment( data ), const_cast< T* >( data ), dip::DataType( data[ 0 ] ), std::move( sizes ) };
         this->move( std::move( tmp )); // a way of calling a different constructor.
      }

      /// \brief Create a new forged image similar to `this`. The data is not copied, and left uninitialized.
      Image Similar() const {
         Image out;
         out.CopyProperties( *this );
         out.Forge();
         return out;
      }

      /// \brief Create a new forged image similar to `this`, but with different data type. The data is not copied, and left uninitialized.
      Image Similar( dip::DataType dt ) const {
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

      /// \brief Get the image size along a specific dimension, without test for dimensionality.
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
         TestSizes( d );
         sizes_ = d;
      }

      // Sets the sizes of the image. Do not use this function unless you know what you're doing.
      void dip__SetSizes( UnsignedArray const& d ) {
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

      /// \brief Get the stride along a specific dimension, without test for dimensionality.
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

      /// \brief computes normal strides given the sizes array and the number of tensor elements. Note that the
      /// tensor stride is presumed to be 1. If tensor dimension is to be sorted at the end, set `tensorElements` to 1.
      DIP_EXPORT static IntegerArray ComputeStrides( UnsignedArray const& sizes, dip::uint tensorElements );

      /// \brief Set the strides array and tensor stride so strides are normal. The image must be raw,
      /// but its sizes should be set first.
      DIP_EXPORT void SetNormalStrides();

      /// \brief Set the strides array and tensor stride to match the dimension order of `src`. The image must be raw,
      /// but its sizes should be set first.
      DIP_EXPORT void MatchStrideOrder( Image const& src );

      /// \brief Test if all the pixels are contiguous.
      ///
      /// If all pixels are contiguous, you can traverse the whole image,
      /// accessing each of the pixels, using a single stride with a value
      /// of 1. To do so, you don't necessarily start at the origin: if any
      /// of the strides is negative, the origin of the contiguous data will
      /// be elsewhere.
      /// Use `dip::Image::GetSimpleStrideAndOrigin` to get a pointer to the origin
      /// of the contiguous data.
      ///
      /// The image must be forged.
      ///
      /// \see GetSimpleStrideAndOrigin, HasSimpleStride, HasNormalStrides, IsSingletonExpanded, Strides, TensorStride.
      bool HasContiguousData() const {
         DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
         dip::uint size = NumberOfPixels() * TensorElements();
         dip::sint start;
         dip::uint sz;
         GetDataBlockSizeAndStartWithTensor( sz, start );
         return sz == size;
      }

      /// \brief Test if strides are as by default. The image must be forged.
      DIP_EXPORT bool HasNormalStrides() const;

      /// \brief Test if any of the image dimensions is a singleton dimension (size is 1). Singleton expanded
      /// dimensions are not considered. The image must be forged.
      DIP_EXPORT bool HasSingletonDimension() const;

      /// \brief Test if the image has been singleton expanded.
      ///
      /// If any dimension is larger than 1, but has a stride of 0, it means that a single pixel is being used
      /// across that dimension. The methods `dip::Image::ExpandSingletonDimension` and
      /// `dip::Image::ExpandSingletonTensor` create such dimensions.
      ///
      /// The image must be forged.
      ///
      /// \see HasContiguousData, HasNormalStrides, ExpandSingletonDimension, ExpandSingletonTensor.
      DIP_EXPORT bool IsSingletonExpanded() const;

      /// \brief Test if the whole image can be traversed with a single stride
      /// value.
      ///
      /// This is similar to `dip::Image::HasContiguousData`, but the stride
      /// value can be larger than 1.
      /// Use `dip::Image::GetSimpleStrideAndOrigin` to get a pointer to the origin
      /// of the contiguous data. Note that this only tests spatial
      /// dimensions, the tensor dimension must still be accessed separately.
      ///
      /// The image must be forged.
      ///
      /// \see GetSimpleStrideAndOrigin, HasContiguousData, HasNormalStrides, Strides, TensorStride.
      bool HasSimpleStride() const {
         void* p;
         std::tie( std::ignore, p ) = GetSimpleStrideAndOrigin();
         return p != nullptr;
      }

      /// \brief Return a single stride to walk through all pixels and pointer to the start of the data.
      ///
      /// If this is not possible, the function returns `nullptr` for the pointer.
      /// Note that this only tests spatial dimensions, the tensor dimension must still be accessed separately.
      ///
      /// ```cpp
      ///     dip::sint stride;
      ///     void* origin;
      ///     std::tie( stride, origin ) = img.GetSimpleStrideAndOrigin();
      /// ```
      ///
      /// The `stride` returned is always positive.
      ///
      /// The image must be forged.
      ///
      /// \see HasSimpleStride, HasContiguousData, HasNormalStrides, Strides, TensorStride, Data.
      DIP_EXPORT std::pair< dip::sint, void* > GetSimpleStrideAndOrigin() const;

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
      DIP_EXPORT bool HasSameDimensionOrder( Image const& other ) const;

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
         return tensor_.TensorShape();
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

      /// \brief True for square matrix images, independent from how they are stored.
      bool IsSquare() const {
         return tensor_.IsSquare();
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

      // Sets the tensor sizes. Do not use this function unless you know what you're doing.
      void dip__SetTensorSizes( dip::uint nelems ) {
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

      /// \brief Sets the image's color space name. This causes the image to be a color image,
      /// but will cause errors to occur (eventually, not immediately) if the number of tensor elements
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

      /// \brief Get the pixels' size in physical units, by reference, allowing to modify it at will.
      ///
      /// There are other `%Image` methods that can be used to modify the pixel size, and might be
      /// simpler. For example:
      /// ```
      ///     img.PixelSize() = ps;                   img.SetPixelSize(ps);
      ///     img.PixelSize().Set(dim,sz);            img.SetPixelSize(dim,sz);
      ///     img.PixelSize().Clear();                img.ResetPixelSize();
      /// ```
      ///
      /// Also for querying the pixel size there are several `%Image` methods:
      /// ```
      ///     pq = img.PixelSize()[dim];              pq = img.PixelSize(dim);
      ///     bd = img.PixelSize().IsDefined();       bd = img.HasPixelSize();
      ///     bi = img.PixelSize().IsIsotropic();     bi = img.IsIsotropic();
      ///     ar = img.PixelSize().AspectRatio(img.Dimensionality());
      ///                                             ar = img.AspectRatio();
      /// ```
      dip::PixelSize& PixelSize() {
         return pixelSize_;
      }

      /// \brief Get the pixels' size in physical units.
      dip::PixelSize const& PixelSize() const {
         return pixelSize_;
      }

      /// \brief Get the pixels' size along the given dimension in physical units.
      PhysicalQuantity const PixelSize( dip::uint dim ) const {
         // Returns a const to prevent errors like `img.PixelSize( 1 ) = 0`.
         // Indexing into a `dip::PixelSize` cannot return a reference.
         return pixelSize_[ dim ];
      }

      /// \brief Set the pixels' size in physical units.
      void SetPixelSize( dip::PixelSize const& ps ) {
         pixelSize_ = ps;
      }

      /// \brief Set the pixels' size along the given dimension in physical units.
      void SetPixelSize( dip::uint dim, PhysicalQuantity sz ) {
         pixelSize_.Set( dim, sz );
      }

      /// \brief Reset the pixels' size, so that `HasPixelSize` returns false.
      void ResetPixelSize() {
         pixelSize_.Clear();
      }

      /// \brief Returns true if the pixel has physical dimensions.
      bool HasPixelSize() const {
         return pixelSize_.IsDefined();
      }

      /// \brief Returns true if the pixel has the same size in all dimensions.
      bool IsIsotropic() const {
         return pixelSize_.IsIsotropic();
      }

      /// \brief Returns an array with aspect ratios: [1, y/x, z/x, ...]. If dimensions don't match, returns
      /// 0 for that dimension.
      FloatArray AspectRatio() const {
         return pixelSize_.AspectRatio( sizes_.size() );
      }

      /// \brief Converts a size in pixels to a size in physical units.
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
      DIP_EXPORT bool CompareProperties(
            Image const& src,
            Option::CmpPropFlags cmpProps,
            Option::ThrowException throwException = Option::ThrowException::DO_THROW
      ) const;

      /// \brief Check image properties, either returns true/false or throws an error.
      DIP_EXPORT bool CheckProperties(
            dip::uint ndims,
            dip::DataType::Classes dts,
            Option::ThrowException throwException = Option::ThrowException::DO_THROW
      ) const;

      /// \brief Check image properties, either returns true/false or throws an error.
      DIP_EXPORT bool CheckProperties(
            dip::uint ndims,
            dip::uint tensorElements,
            dip::DataType::Classes dts,
            Option::ThrowException throwException = Option::ThrowException::DO_THROW
      ) const;

      /// \brief Check image properties, either returns true/false or throws an error.
      DIP_EXPORT bool CheckProperties(
            UnsignedArray const& sizes,
            dip::DataType::Classes dts,
            Option::ThrowException throwException = Option::ThrowException::DO_THROW
      ) const;

      /// \brief Check image properties, either returns true/false or throws an error.
      DIP_EXPORT bool CheckProperties(
            UnsignedArray const& sizes,
            dip::uint tensorElements,
            dip::DataType::Classes dts,
            Option::ThrowException throwException = Option::ThrowException::DO_THROW
      ) const;

      /// \brief Check image properties for a mask image, either returns true/false or throws an error.
      DIP_EXPORT bool CheckIsMask(
            UnsignedArray const& sizes,
            Option::AllowSingletonExpansion allowSingletonExpansion = Option::AllowSingletonExpansion::DONT_ALLOW,
            Option::ThrowException throwException = Option::ThrowException::DO_THROW
      ) const;

      /// \brief Copy all image properties from `src`, including strides. The image must be raw.
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
         swap( externalData_, other.externalData_ );
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
      /// The pointer returned could be tangentially related to the data segment, if
      /// `dip::Image::IsExternalData` is true.
      ///
      /// \see Origin, IsShared, ShareCount, SharesData, IsExternalData.
      void* Data() const {
         DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
         return dataBlock_.get();
      }

      /// \brief Check to see if the data segment is shared with other images.
      ///
      /// \see Data, ShareCount, SharesData, IsExternalData.
      bool IsShared() const {
         return IsForged() && ( dataBlock_.use_count() > 1 );
      }

      /// \brief Get the number of images that share their data with this image.
      ///
      /// For normal images. the count is always at least 1. If the count is
      /// larger than 1, `dip::Image::IsShared` is true.
      ///
      /// If `this` encapsulates external data (`dip::Image::IsExternalData` is true),
      /// then the share count is not necessarily correct, as it might not count
      /// the uses of the source data.
      ///
      /// The image must be forged.
      ///
      /// \see Data, IsShared, SharesData, IsExternalData.
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
      /// use `dip::Image::Aliases`.
      ///
      /// \see Aliases, IsIdenticalView, IsOverlappingView, Data, IsShared, ShareCount, IsExternalData.
      bool SharesData( Image const& other ) const {
         return IsForged() && other.IsForged() && ( dataBlock_ == other.dataBlock_ );
      }

      /// \brief Returns true if the data segment was not allocated by *DIPlib*. See \ref external_data_segment.
      bool IsExternalData() const {
         return IsForged() && externalData_;
      }

      /// \brief Determine if `this` shares any samples with `other`.
      ///
      /// If `true`, writing into this image will change the data in
      /// `other`, and vice-versa.
      ///
      /// \see SharesData, IsIdenticalView, IsOverlappingView, Alias.
      DIP_EXPORT bool Aliases( Image const& other ) const;

      /// \brief Determine if `this` and `other` offer an identical view of the
      /// same set of pixels.
      ///
      /// If `true`, changing one sample in this image will change the same sample in `other`.
      ///
      /// \see SharesData, Aliases, IsOverlappingView.
      bool IsIdenticalView( Image const& other ) const {
         DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
         // We don't need to check dataBlock_ here, as origin_ is a pointer, not an offset.
         return IsForged() && other.IsForged() &&
                ( origin_ == other.origin_ ) &&
                ( dataType_ == other.dataType_ ) &&
                ( sizes_ == other.sizes_ ) &&
                ( tensor_.Elements() == other.tensor_.Elements() ) &&
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
      /// \see SharesData, Aliases, IsIdenticalView.
      bool IsOverlappingView( ImageConstRefArray const& other ) const {
         for( dip::uint ii = 0; ii < other.size(); ++ii ) {
            Image const& tmp = other[ ii ].get();
            if( IsOverlappingView( tmp )) {
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
      /// \see SharesData, Aliases, IsIdenticalView.
      bool IsOverlappingView( ImageArray const& other ) const {
         for( dip::uint ii = 0; ii < other.size(); ++ii ) {
            Image const& tmp = other[ ii ];
            if( IsOverlappingView( tmp )) {
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
      /// existing stride array, and may or may not produce normal strides.
      DIP_EXPORT void Forge();

      /// \brief Modify image properties and forge the image.
      ///
      /// `%ReForge` has three
      /// signatures that match three image constructors. `%ReForge` will try
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
      /// `%ReForge` has three
      /// signatures that match three image constructors. `%ReForge` will try
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
         auto tensor = src.tensor_;          // If `&src == this`, the `ReForge` will delete the non-data properties.
         auto colorSpace = src.colorSpace_;
         auto pixelSize = src.pixelSize_;
         ReForge( src.sizes_, src.tensor_.Elements(), dt, acceptDataTypeChange );
         tensor_ = tensor;
         colorSpace_ = std::move( colorSpace );
         pixelSize_ = std::move( pixelSize );
      }

      /// \brief Modify image properties and forge the image.
      ///
      /// `%ReForge` has three
      /// signatures that match three image constructors. `%ReForge` will try
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
      DIP_EXPORT void ReForge(
            UnsignedArray const& sizes,
            dip::uint tensorElems = 1,
            dip::DataType dt = DT_SFLOAT,
            Option::AcceptDataTypeChange acceptDataTypeChange = Option::AcceptDataTypeChange::DONT_ALLOW
      );

      /// \brief Disassociate the data segment from the image. If there are no
      /// other images using the same data segment, it will be freed.
      void Strip() {
         if( IsForged() ) {
            DIP_THROW_IF( IsProtected(), "Image is protected" );
            dataBlock_ = nullptr; // Automatically frees old memory if no other pointers to it exist.
            origin_ = nullptr;    // Keep this one in sync!
            externalData_ = false;
         }
      }

      /// \brief Test if forged.
      bool IsForged() const {
         return origin_ != nullptr;
      }

      /// \brief Set protection flag.
      ///
      /// A protected image cannot be stripped or reforged. See \ref protect for more information.
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

      /// \brief Set external interface pointer. The image must be raw. See \ref external_interface.
      void SetExternalInterface( dip::ExternalInterface* ei ) {
         DIP_THROW_IF( IsForged(), E::IMAGE_NOT_RAW );
         externalInterface_ = ei;
      }

      /// \brief Remove external interface pointer. The image behaves like a native one (for assignment, reforging,
      /// etc.), but the current pixel buffer (if forged) is not affected. See \ref external_interface.
      void ResetExternalInterface() {
         externalInterface_ = nullptr;
      }

      /// \brief Get external interface pointer. See \ref external_interface
      dip::ExternalInterface* ExternalInterface() const {
         return externalInterface_;
      }

      /// \brief Test if an external interface is set. See \ref external_interface
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

      // Sets the pointer to the first sample in the image. Do not use this function
      // unless you know what you're doing.
      void dip__SetOrigin( void* origin ) {
         origin_ = origin;
      }

      // Shifts the pointer to the first sample in the image by offset. Do not use this
      // function unless you know what you're doing.
      void dip__ShiftOrigin( dip::sint offset ) {
         origin_ = static_cast< uint8* >( origin_ ) + offset * static_cast< dip::sint >( dataType_.SizeOf() );
      }

      /// \brief Get a pointer to the pixel given by the offset.
      ///
      /// Cast the pointer to the right type before use. No check is made on the index.
      ///
      /// The image must be forged.
      ///
      /// \see Origin, Offset, OffsetToCoordinates
      void* Pointer( dip::sint offset ) const {
         DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
         return static_cast< uint8* >( origin_ ) + offset * static_cast< dip::sint >( dataType_.SizeOf() );
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

      /// \brief Return true if the coordinates are on the image edge.
      ///
      /// Coordinates on the image edge are such that at least one neighboring coordinates
      /// (direct neighbor) is outside the image domain.
      ///
      /// The image must be forged.
      bool IsOnEdge( UnsignedArray const& coords ) const {
         DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( coords.size() != sizes_.size(), E::ARRAY_PARAMETER_WRONG_LENGTH );
         for( dip::uint ii = 0; ii < coords.size(); ++ii ) {
            if(( coords[ ii ] == 0 ) || ( coords[ ii ] == sizes_[ ii ] - 1 )) {
               return true;
            }
         }
         return false;
      }

      /// \brief Returns whether the coordinates are inside the image
      template< typename CoordType >
      bool IsInside( DimensionArray< CoordType > const& coords ) const {
         DIP_THROW_IF( sizes_.empty(), "Image sizes not set" );
         DIP_THROW_IF( coords.size() != sizes_.size(), E::DIMENSIONALITIES_DONT_MATCH );
         for( dip::uint ii = 0; ii < coords.size(); ++ii ) {
            if(( coords[ ii ] < 0 ) || ( coords[ ii ] > static_cast< CoordType >( sizes_[ ii ] - 1 ))) {
               return false;
            }
         }
         return true;
      }

      /// \brief Compute offset given coordinates and strides.
      ///
      /// The offset needs to be multiplied by the number of bytes of each sample to become
      /// a memory offset within the image.
      ///
      /// If `coords` is not within the domain given by `sizes`, an exception is thrown.
      /// The size of `coords` is not verified.
      static dip::sint Offset( UnsignedArray const& coords, IntegerArray const& strides, UnsignedArray const& sizes ) {
         DIP_THROW_IF( coords.size() != strides.size(), E::ARRAY_PARAMETER_WRONG_LENGTH );
         DIP_ASSERT( coords.size() == sizes.size() );
         dip::sint offset = 0;
         for( dip::uint ii = 0; ii < coords.size(); ++ii ) {
            DIP_THROW_IF( coords[ ii ] >= sizes[ ii ], E::INDEX_OUT_OF_RANGE );
            offset += static_cast< dip::sint >( coords[ ii ] ) * strides[ ii ];
         }
         return offset;
      }

      /// \brief Compute offset given coordinates.
      ///
      /// The offset needs to be multiplied by the number of bytes of each sample to become
      /// a memory offset within the image.
      ///
      /// `coords` can have negative values, no domain assumptions are made.
      static dip::sint Offset( IntegerArray const& coords, IntegerArray const& strides ) {
         DIP_THROW_IF( coords.size() != strides.size(), E::ARRAY_PARAMETER_WRONG_LENGTH );
         dip::sint offset = 0;
         for( dip::uint ii = 0; ii < coords.size(); ++ii ) {
            offset += coords[ ii ] * strides[ ii ];
         }
         return offset;
      }

      /// \brief Compute offset given coordinates.
      ///
      /// The offset needs to be multiplied by the number of bytes of each sample to become
      /// a memory offset within the image.
      ///
      /// If `coords` is not within the image domain, an exception is thrown.
      ///
      /// The image must be forged.
      ///
      /// \see Origin, Pointer, OffsetToCoordinates
      dip::sint Offset( UnsignedArray const& coords ) const {
         DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
         return Offset( coords, strides_, sizes_ );
      }

      /// \brief Compute offset given coordinates.
      ///
      /// The offset needs to be multiplied by the number of bytes of each sample to become
      /// a memory offset within the image.
      ///
      /// `coords` can be outside the image domain.
      ///
      /// The image must be forged.
      ///
      /// \see Origin, Pointer, OffsetToCoordinates
      dip::sint Offset( IntegerArray const& coords ) const {
         DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
         return Offset( coords, strides_ );
      }

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
      UnsignedArray OffsetToCoordinates( dip::sint offset ) const  {
         CoordinatesComputer comp = OffsetToCoordinatesComputer();
         return comp( offset );
      }

      /// \brief Returns a functor that computes coordinates given an offset.
      ///
      /// This is
      /// more efficient than using `dip::Image::OffsetToCoordinates` when repeatedly
      /// computing offsets, but still requires complex calculations.
      ///
      /// The image must be forged.
      ///
      /// \see Offset, OffsetToCoordinates, IndexToCoordinates, IndexToCoordinatesComputer
      CoordinatesComputer OffsetToCoordinatesComputer() const {
         DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
         return CoordinatesComputer( sizes_, strides_ );
      }

      /// \brief Compute linear index (not offset) given coordinates and image sizes.
      ///
      /// This index is not related to the position of the pixel in memory, and should not be
      /// used to index many pixels in sequence.
      static dip::uint Index( UnsignedArray const& coords, UnsignedArray const& sizes ) {
         DIP_THROW_IF( coords.size() != sizes.size(), E::ARRAY_PARAMETER_WRONG_LENGTH );
         dip::uint index = 0;
         for( dip::uint ii = sizes.size(); ii > 0; ) {
            --ii;
            DIP_THROW_IF( coords[ ii ] >= sizes[ ii ], E::INDEX_OUT_OF_RANGE );
            index *= sizes[ ii ];
            index += coords[ ii ];
         }
         return index;
      }

      /// \brief Compute linear index (not offset) given coordinates.
      ///
      /// This index is not related to the position of the pixel in memory, and should not be
      /// used to index many pixels in sequence.
      ///
      /// The image must be forged.
      ///
      /// \see IndexToCoordinates, Offset
      dip::uint Index( UnsignedArray const& coords ) const {
         DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
         return Index( coords, sizes_ );
      }

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
      UnsignedArray IndexToCoordinates( dip::uint index ) const {
         CoordinatesComputer comp = IndexToCoordinatesComputer();
         return comp( static_cast< dip::sint >( index ));
      }

      /// \brief Returns a functor that computes coordinates given a linear index.
      ///
      /// This is
      /// more efficient than using `dip::Image::IndexToCoordinates`, when repeatedly
      /// computing indices, but still requires complex calculations.
      ///
      /// The image must be forged.
      ///
      /// \see Index, Offset, IndexToCoordinates, OffsetToCoordinates, OffsetToCoordinatesComputer
      DIP_EXPORT CoordinatesComputer IndexToCoordinatesComputer() const;

      /// \brief Returns the coordinates for the center of the image.
      ///
      /// `mode` specifies the origin of the coordinates. It can be one of the following strings:
      ///  - `"right"`: The origin is on the pixel right of the center (at integer division result of
      ///    `size/2`). This is the default.
      ///  - `"left"`: The origin is on the pixel left of the center (at integer division result of
      ///    `(size-1)/2`).
      ///  - `"true"`: The origin is halfway the first and last pixel, in between pixels if necessary
      ///    (at floating-point division result of `size/2`).
      ///  - `"corner"`: The origin is on the first pixel.
      ///  - `"frequency"`: The coordinates used are as for the Fourier transform. Same results as
      ///    for `"right"`.
      // Function defined in src/generation/coordinates.cpp for convenience.
      DIP_EXPORT FloatArray GetCenter( String const& mode = "right" ) const;

      /// \}

      //
      // Modifying geometry of a forged image without data copy
      // Defined in src/library/image_manip.cpp
      //

      /// \name Reshaping forged image
      /// These functions change the image object, providing a differently-shaped version of the same data.
      /// No data is copied, and the image contains the same set of samples as before the method call.
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
      DIP_EXPORT Image& PermuteDimensions( UnsignedArray const& order );

      /// \brief Swap dimensions d1 and d2. This is a simplified version of `PermuteDimensions`.
      ///
      /// The image must be forged. The data will never be copied (i.e. this is a quick and cheap operation).
      ///
      /// \see PermuteDimensions.
      DIP_EXPORT Image& SwapDimensions( dip::uint dim1, dip::uint dim2 );

      /// \brief Make image 1D.
      ///
      /// The image must be forged. If HasSimpleStride,
      /// this is a quick and cheap operation, but if not, the data segment
      /// will be copied. Note that the order of the pixels in the
      /// resulting image depend on the strides, and do not necessarily
      /// follow the same order as linear indices.
      ///
      /// \see FlattenAsMuchAsPossible, Squeeze.
      DIP_EXPORT Image& Flatten();

      /// \brief Make image have as few dimensions as possible.
      ///
      /// If the image has contiguous storage (or non-contiguous storage with a simple stride), then
      /// `dip::Image::Flatten` will convert it into a 1D image without copy. This method performs a similar
      /// function, but only merges the dimensions that are possible to merge without data copy. In the
      /// cases where `dip::Image::Flatten` doesn't copy data, this method will yield the same result. In
      /// other cases, the output of this method will yield an image with more than one dimension, sometimes
      /// as many as the input image. Dimensions can be reordered and reversed.
      ///
      /// The goal with reducing dimensions is to make it simpler to iterate through the image. Iterators
      /// will be more efficient on a flattened image.
      ///
      /// The image must be forged. This is always a quick and cheap operation.
      /// Note that the order of the pixels in the resulting image depend on the strides, and do not necessarily
      /// follow the same order as linear indices.
      ///
      /// \see Flatten, Squeeze
      DIP_EXPORT Image& FlattenAsMuchAsPossible();

      /// \brief Remove singleton dimensions (dimensions with size==1).
      ///
      /// The image must be forged. The data will never be copied (i.e. this is a quick and cheap operation).
      ///
      /// \see AddSingleton, ExpandDimensionality, PermuteDimensions, UnexpandSingletonDimensions.
      DIP_EXPORT Image& Squeeze();

      /// \brief Remove singleton dimension `dim` (has size==1).
      ///
      /// The image must be forged. The data will never be copied (i.e. this is a quick and cheap operation).
      ///
      /// \see Squeeze, AddSingleton, ExpandDimensionality, PermuteDimensions, UnexpandSingletonDimensions.
      DIP_EXPORT Image& Squeeze( dip::uint dim );

      /// \brief Add a singleton dimension (with size==1) to the image.
      ///
      /// Dimensions `dim` to last are shifted up, dimension `dim` will have a size of 1.
      ///
      /// The image must be forged. The data will never be copied (i.e. this is a quick and cheap operation).
      ///
      /// Example: to an image with sizes `{ 4, 5, 6 }` we add a
      /// singleton dimension `dim == 1`. The image will now have
      /// sizes `{ 4, 1, 5, 6 }`.
      ///
      /// \see Squeeze, ExpandDimensionality, PermuteDimensions.
      DIP_EXPORT Image& AddSingleton( dip::uint dim );

      /// \brief Append singleton dimensions to increase the image dimensionality.
      ///
      /// The image will have `n` dimensions. However, if the image already
      /// has `n` or more dimensions, nothing happens.
      ///
      /// The image must be forged. The data will never be copied (i.e. this is a quick and cheap operation).
      ///
      /// \see AddSingleton, ExpandSingletonDimension, Squeeze, PermuteDimensions, Flatten.
      DIP_EXPORT Image& ExpandDimensionality( dip::uint dim );

      /// \brief Expand singleton dimension `dim` to `sz` pixels, setting the corresponding stride to 0.
      ///
      /// If `dim` is not a singleton dimension (size==1), an exception is thrown.
      ///
      /// The image must be forged. The data will never be copied (i.e. this is a quick and cheap operation).
      ///
      /// \see ExpandSingletonDimensions, ExpandSingletonTensor, IsSingletonExpanded, UnexpandSingletonDimensions, AddSingleton, ExpandDimensionality.
      DIP_EXPORT Image& ExpandSingletonDimension( dip::uint dim, dip::uint sz );

      /// \brief Performs singleton expansion.
      ///
      /// The image is modified so that it has `size`
      /// as dimensions. It must be forged and singleton-expandable to `size`,
      /// otherwise an exception is thrown. See `dip::Image::ExpandSingletonDimension`.
      /// `size` is the array as returned by `dip::Framework::SingletonExpandedSize`.
      ///
      /// \see ExpandSingletonDimension, ExpandSingletonTensor, IsSingletonExpanded, UnexpandSingletonDimensions.
      DIP_EXPORT Image& ExpandSingletonDimensions( UnsignedArray const& newSizes );

      /// \brief Unexpands singleton-expanded dimensions.
      ///
      /// The image is modified so that each singleton-expanded dimension has a size of 1, including the tensor
      /// dimension. That is, the resulting image will no longer be `dip::Image::IsSingletonExpanded`.
      ///
      /// \see ExpandSingletonDimension, ExpandSingletonTensor, IsSingletonExpanded, Squeeze.
      DIP_EXPORT Image& UnexpandSingletonDimensions();

      /// \brief Tests if the image can be singleton-expanded to `size`.
      ///
      /// \see ExpandSingletonDimensions, ExpandSingletonTensor, IsSingletonExpanded.
      DIP_EXPORT bool IsSingletonExpansionPossible( UnsignedArray const& newSizes ) const;

      /// \brief Expand singleton tensor dimension `sz` samples, setting the tensor
      /// stride to 0.
      ///
      /// If there is more than one tensor element, an exception is thrown.
      ///
      /// The image must be forged. The data will never be copied (i.e. this is a quick and cheap operation).
      ///
      /// \see ExpandSingletonDimension, IsSingletonExpanded.
      DIP_EXPORT Image& ExpandSingletonTensor( dip::uint sz );

      /// \brief Mirror the image about selected axes.
      ///
      /// The image must be forged. The data will never be copied (i.e. this is a quick and cheap operation).
      ///
      /// `process` indicates which axes to mirror. If `process` is an empty array, all
      /// axes will be mirrored.
      DIP_EXPORT Image& Mirror( BooleanArray process = {} );

      /// \brief Rotates the image by `n` times 90 degrees, in the plane defined by dimensions
      /// `dimension1` and `dimension2`.
      ///
      /// The image must be forged. The data will never be copied (i.e. this is a quick and cheap operation).
      ///
      /// The rotation occurs in the direction of positive angles, as defined in the image coordinate system.
      /// That is, if `dimension1` is 0 (x-axis) and `dimension2` is 1 (y-axis), and considering the y-axis is
      /// positive in the down direction, then the rotation happens in clockwise direction. A negative
      /// value for `n` inverts the direction of rotation.
      DIP_EXPORT Image& Rotation90( dip::sint n, dip::uint dimension1, dip::uint dimension2 );

      /// \brief Rotates the image by `n` times 90 degrees, in the plane perpendicular to dimension `axis`.
      ///
      /// The image must be forged and have three dimensions. The data will never be copied
      /// (i.e. this is a quick and cheap operation).
      Image& Rotation90( dip::sint n, dip::uint axis ) {
         DIP_THROW_IF( Dimensionality() != 3, E::DIMENSIONALITY_NOT_SUPPORTED );
         dip::uint dim1, dim2;
         switch( axis ) {
            case 0: // x-axis
               dim1 = 1;
               dim2 = 2;
               break;
            case 1: // y-axis
               dim1 = 2;
               dim2 = 0;
               break;
            case 2: // z-axis
               dim1 = 0;
               dim2 = 1;
               break;
            default:
               DIP_THROW( E::ILLEGAL_DIMENSION );
         }
         return Rotation90( n, dim1, dim2 );
      }

      /// \brief Rotates the image by `n` times 90 degrees, in the x-y plane.
      ///
      /// The image must be forged. The data will never be copied (i.e. this is a quick and cheap operation).
      Image& Rotation90( dip::sint n ) {
         return Rotation90( n, 0, 1 );
      }

      /// \brief Undo the effects of `Mirror`, `Rotation90`, `PermuteDimensions`, and singleton expansion.
      /// Also removes singleton dimensions.
      ///
      /// Modifies the image such that all strides are positive and sorted smaller to larger. The first
      /// dimension will have the smallest stride. Visiting pixels in linear indexing order (as is done
      /// through `dip::ImageIterator`) will be most efficient after calling this function.
      ///
      /// Note that strides are not necessarily normal after this call, if the image is a view over a
      /// larger image, if singleton dimensions were created or expanded, etc. Use `ForceNormalStrides`
      /// to ensure that strides are normal.
      DIP_EXPORT Image& StandardizeStrides();

      /// \brief Transforms input arrays and outputs ordering required to standardize an image's strides.
      ///
      /// `strides` and `sizes` are modified such that any negative strides (mirrored image dimensions) become
      /// positive, and expanded singleton dimensions become singletons again.
      ///
      /// The output array can be used to permute the `strides` and the `sizes` arrays to reorder image dimensions
      /// such that linear indices match storage order.
      ///
      /// The output signed integer is the offset that needs to be applied to the origin to account for any
      /// image dimensions that were reversed.
      ///
      /// The non-static `%Image` method with the same name uses this function to standardize the strides of
      /// the image:
      ///
      /// ```cpp
      ///     dip::UnsignedArray order;
      ///     dip::sint offset;
      ///     std::tie( order, offset ) = dip::Image::StandardizeStrides( strides, sizes );
      ///     origin = origin + offset;
      ///     sizes = sizes.permute( order );
      ///     strides = strides.permute( order );
      /// ```
      ///
      /// `sizes` and `strides` are assumed to be of the same length, this is not tested for.
      DIP_EXPORT static std::pair< UnsignedArray, dip::sint > StandardizeStrides( IntegerArray& strides, UnsignedArray& sizes );

      /// \brief Change the tensor shape, without changing the number of tensor elements.
      Image& ReshapeTensor( dip::uint rows, dip::uint cols ) {
         DIP_THROW_IF( tensor_.Elements() != rows * cols, "Cannot reshape tensor to requested sizes" );
         tensor_.ChangeShape( rows );
         return *this;
      }

      /// \brief Change the tensor shape, without changing the number of tensor elements.
      Image& ReshapeTensor( dip::Tensor const& example ) {
         tensor_.ChangeShape( example );
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
      /// `dim` should not be larger than the number of dimensions. `dim`
      /// defaults to the image dimensionality, meaning that the new dimension will
      /// be the last one. The image must be forged.
      DIP_EXPORT Image& TensorToSpatial( dip::uint dim );
      inline Image& TensorToSpatial() {
         return TensorToSpatial( Dimensionality() );
      }

      /// \brief Convert spatial dimension to tensor dimensions. The image must be scalar.
      ///
      /// If `rows` or `cols` is zero, its size is computed from the size of the
      /// image along dimension `dim`. If both are zero, a default column tensor
      /// is created. `dim` defaults to the last spatial dimension. The image must
      /// be forged.
      DIP_EXPORT Image& SpatialToTensor( dip::uint dim, dip::uint rows, dip::uint cols );
      inline Image& SpatialToTensor( dip::uint rows, dip::uint cols ) {
         return SpatialToTensor( Dimensionality() - 1, rows, cols );
      }
      inline Image& SpatialToTensor( dip::uint dim ) {
         return SpatialToTensor( dim, 0, 0 );
      }
      inline Image& SpatialToTensor() {
         return SpatialToTensor( Dimensionality() - 1, 0, 0 );
      }

      /// \brief Split the two values in a complex sample into separate samples,
      /// creating a new spatial dimension of size 2.
      ///
      /// `dim` defines the new
      /// dimension, subsequent dimensions will be shifted over. `dim` should
      /// not be larger than the number of dimensions. `dim` defaults to the
      /// image dimensionality, meaning that the new dimension will be the last one.
      /// The image must be forged.
      DIP_EXPORT Image& SplitComplex( dip::uint dim );
      inline Image& SplitComplex() {
         return SplitComplex( Dimensionality() );
      }

      /// \brief Merge the two samples along dimension `dim` into a single complex-valued sample.
      ///
      /// Dimension `dim` must have size 2 and a stride of 1. `dim` defaults to the last
      /// spatial dimension. The image must be forged.
      DIP_EXPORT Image& MergeComplex( dip::uint dim );
      inline Image& MergeComplex() {
         return MergeComplex( Dimensionality() - 1 );
      }

      /// \brief Split the two values in a complex sample into separate samples of
      /// a tensor. The image must be scalar and forged.
      DIP_EXPORT Image& SplitComplexToTensor();

      /// \brief Merge the two samples in the tensor into a single complex-valued sample.
      ///
      /// The image must have two tensor elements, a tensor stride of 1, and be forged.
      DIP_EXPORT Image& MergeTensorToComplex();

      /// \brief Reduces the size of the image by cropping off the borders.
      ///
      /// Crops the image to the given size. Which pixels are selected is controlled by the
      /// `cropLocation` parameter. The default is `dip::Option::CropLocation::CENTER`, which
      /// maintains the origin pixel (as defined in `dip::FourierTransform` and other other places)
      /// at the origin of the output image.
      ///
      /// `dip::Image::Pad` does the inverse operation. `dip::Image::Cropped` does the same
      /// thing, but without modifying `this`, and returning a `dip::Image::View`.
      ///
      /// The image must be forged.
      DIP_EXPORT Image& Crop(
            UnsignedArray const& sizes,
            Option::CropLocation cropLocation = Option::CropLocation::CENTER
      );

      /// \brief Reduces the size of the image by cropping off the borders.
      ///
      /// This is an overloaded version of the function above, meant for use from bindings in other languages. The
      /// string `cropLocation` is translated to one of the `dip::Option::CropLocation` values as follows:
      ///
      /// `%CropLocation` constant | String
      /// ------------------------ | ----------
      /// `CENTER`                 | "center"
      /// `MIRROR_CENTER`          | "mirror center"
      /// `TOP_LEFT`               | "top left"
      /// `BOTTOM_RIGHT`           | "bottom right"
      DIP_EXPORT Image& Crop( UnsignedArray const& sizes, String const& cropLocation );

      /// \}

      //
      // Creating views of the data -- indexing without data copy
      // Defined in src/library/image_indexing.cpp
      //

      /// \name Indexing without data copy
      /// These functions create a different view of the data contained in the image. The output
      /// is usually a `dip::Image::View` or `dip::Image::Pixel` object. No data is copied.
      /// \{

      /// \brief Extract a tensor element, `indices` must have one or two elements. The image must be forged.
      View operator[]( UnsignedArray const& indices ) const;

      /// \brief Extract a tensor element using linear indexing. Negative indices start at the end. The image must be forged.
      template< typename T, typename = std::enable_if_t< IsIndexingType< T >::value >>
      View operator[]( T index ) const;

      /// \brief Extract tensor elements using linear indexing. The image must be forged.
      View operator[]( Range const& range ) const;

      /// \brief Extracts the tensor elements along the diagonal. The image must be forged.
      DIP_EXPORT View Diagonal() const;

      /// \brief Extracts the tensor elements along the given row. The image must be forged and the tensor
      /// representation must be full (i.e. no symmetric or triangular matrices). Use `dip::Image::ExpandTensor`
      /// to obtain a full representation.
      DIP_EXPORT View TensorRow( dip::uint index ) const;

      /// \brief Extracts the tensor elements along the given column. The image must be forged and the tensor
      /// representation must be full (i.e. no symmetric or triangular matrices). Use `dip::Image::ExpandTensor`
      /// to obtain a full representation.
      DIP_EXPORT View TensorColumn( dip::uint index ) const;

      /// \brief Extracts the pixel at the given coordinates. The image must be forged.
      DIP_EXPORT Pixel At( UnsignedArray const& coords ) const;

      /// \brief Same as above, but returns a type that implicitly casts to `T`.
      template< typename T >
      CastPixel< T > At( UnsignedArray const& coords ) const;

      /// \brief Extracts the pixel at the given linear index (inefficient if image is not 1D!). The image must be forged.
      DIP_EXPORT Pixel At( dip::uint index ) const;

      /// \brief Same as above, but returns a type that implicitly casts to `T`.
      template< typename T >
      CastPixel< T > At( dip::uint index ) const;

      /// \brief Extracts the pixel at the given coordinates from a 2D image. The image must be forged.
      DIP_EXPORT Pixel At( dip::uint x_index, dip::uint y_index ) const;

      /// \brief Same as above, but returns a type that implicitly casts to `T`.
      template< typename T >
      CastPixel< T > At( dip::uint x_index, dip::uint y_index ) const;

      /// \brief Extracts the pixel at the given coordinates from a 3D image. The image must be forged.
      DIP_EXPORT Pixel At( dip::uint x_index, dip::uint y_index, dip::uint z_index ) const;

      /// \brief Same as above, but returns a type that implicitly casts to `T`.
      template< typename T >
      CastPixel< T > At( dip::uint x_index, dip::uint y_index, dip::uint z_index ) const;

      /// \brief Returns an iterator to the first pixel in the image. Include `<diplib/generic_iterators.h>`
      /// to use this.
      GenericImageIterator< dip::dfloat > begin();

      /// \brief Returns an iterator to the end of the iterator range. It cannot be dereferenced or manipulated,
      /// and is meant solely as an end-of-iteration marker.
      GenericImageIterator< dip::dfloat > end();

      /// \brief Extracts a subset of pixels from a 1D image. The image must be forged.
      View At( Range const& x_range ) const;

      /// \brief Extracts a subset of pixels from a 2D image. The image must be forged.
      View At( Range const& x_range, Range const& y_range ) const;

      /// \brief Extracts a subset of pixels from a 3D image. The image must be forged.
      View At( Range const& x_range, Range const& y_range, Range const& z_range ) const;

      /// \brief Extracts a subset of pixels from an image. The image must be forged.
      View At( RangeArray const& ranges ) const;

      /// \brief Creates a 1D image view containing the pixels selected by `mask`.
      ///
      /// When cast to an image, the values will be copied, not referenced. The output is of the same data type
      /// and tensor shape as `this`, but have only one dimension. Pixels will be read from `mask` in the linear
      /// index order.
      ///
      /// `this` must be forged and be of equal size as `mask`. `mask` is a scalar binary image.
      View At( Image const& mask ) const;

      /// \brief Creates a 1D image view containing the pixels selected by `coordinates`.
      ///
      /// When cast to an image, the values will be copied, not referenced. The output is of the same data type
      /// and tensor shape as `this`, but have only one dimension. It will have as many pixels as coordinates are
      /// in `coordinates`, and be sorted in the same order.
      ///
      /// Each of the coordinates must have the same number of dimensions as `this`.
      ///
      /// `this` must be forged.
      View At( CoordinateArray const& coordinates ) const;

      /// \brief Creates a 1D image view containing the pixels selected by `indices`.
      ///
      /// When cast to an image, the values will be copied, not referenced. The output is of the same data type
      /// and tensor shape as `this`, but have only one dimension. It will have as many pixels as indices are in
      /// `indices`, and be sorted in the same order.
      ///
      /// `indices` contains linear indices into the image. Note that converting indices into offsets is not a
      /// trivial operation; prefer to use the version of this function that uses coordinates.
      ///
      /// `this` must be forged.
      ///
      /// Note that this function is not called `%At` because of the clash with `%At( %UnsignedArray const& coords )`.
      View AtIndices( UnsignedArray const& indices ) const;

      /// \brief Extracts a subset of pixels from an image.
      ///
      /// Returns a view to a smaller area within the image. Which pixels are selected is controlled by the
      /// `cropLocation` parameter. The default is `dip::Option::CropLocation::CENTER`, which
      /// maintains the origin pixel (as defined in `dip::FourierTransform` and other other places)
      /// at the origin of the output image.
      ///
      /// `dip::Image::Pad` does the inverse operation. `dip::Image::Crop` does the same thing, but modifies
      /// the image directly instead of returning a view.
      ///
      /// The image must be forged.
      DIP_EXPORT View Cropped(
            UnsignedArray const& sizes,
            Option::CropLocation cropLocation = Option::CropLocation::CENTER
      ) const;

      /// \brief Extracts a subset of pixels from an image.
      ///
      /// This is an overloaded version of the function above, meant for use from bindings in other languages. The
      /// string `cropLocation` is translated to one of the `dip::Option::CropLocation` values as follows:
      ///
      /// `%CropLocation` constant | String
      /// ------------------------ | ----------
      /// `CENTER`                 | "center"
      /// `MIRROR_CENTER`          | "mirror center"
      /// `TOP_LEFT`               | "top left"
      /// `BOTTOM_RIGHT`           | "bottom right"
      DIP_EXPORT View Cropped( UnsignedArray const& sizes, String const& cropLocation ) const;

      /// \brief Extracts the real component of a complex-typed image. The image must be forged.
      DIP_EXPORT View Real() const;

      /// \brief Extracts the imaginary component of a complex-typed image. The image must be forged.
      DIP_EXPORT View Imaginary() const;

      /// \brief Quick copy, returns a new image that points at the same data as `this`,
      /// and has mostly the same properties.
      ///
      /// The color space and pixel size information are not copied, and the protect flag is reset.
      /// The external interface is not taken over either.
      /// This function is mostly meant for use in functions that need to modify some properties of
      /// the input images, without actually modifying the input images.
      ///
      /// `dip::Image::Copy` is similar, but makes a deep copy of the image, such that the output image
      /// has its own data segment.
      Image QuickCopy() const {
         Image out;
         out.dataType_ = dataType_;
         out.sizes_ = sizes_;
         out.strides_ = strides_;
         out.tensor_ = tensor_;
         out.tensorStride_ = tensorStride_;
         out.dataBlock_ = dataBlock_;
         out.origin_ = origin_;
         out.externalData_ = externalData_;
         return out;
      }

      /// \}

      //
      // Getting/setting pixel values, data copies
      // Defined in src/library/image_data.cpp
      //

      /// \name Setting pixel values, copying
      /// \{

      /// \brief Extends the image by padding with zeros.
      ///
      /// Pads the image to the given size. Where the original image data is located in the output image
      /// is controlled by the `cropLocation` parameter. The default is `dip::Option::CropLocation::CENTER`,
      /// which maintains the origin pixel (as defined in `dip::FourierTransform` and other other places)
      /// at the origin of the output image.
      ///
      /// The object is not modified, a new image is created, with identical properties, but of the requested
      /// size.
      ///
      /// `dip::Image::Crop` does the inverse operation. See also `dip::ExtendImage`.
      ///
      /// The image must be forged.
      DIP_EXPORT Image Pad( UnsignedArray const& sizes, Option::CropLocation cropLocation = Option::CropLocation::CENTER ) const;

      /// \brief Extends the image by padding with zeros.
      ///
      /// This is an overloaded version of the function above, meant for use from bindings in other languages. The
      /// string `cropLocation` is translated to one of the `dip::Option::CropLocation` values as follows:
      ///
      /// `%CropLocation` constant | String
      /// ------------------------ | ----------
      /// `CENTER`                 | "center"
      /// `MIRROR_CENTER`          | "mirror center"
      /// `TOP_LEFT`               | "top left"
      /// `BOTTOM_RIGHT`           | "bottom right"
      inline Image Pad( UnsignedArray const& sizes, String const& cropLocation ) const {
         if( cropLocation == S::CENTER ) {
            return Pad( sizes, Option::CropLocation::CENTER );
         } else if( cropLocation == S::MIRROR_CENTER ) {
            return Pad( sizes, Option::CropLocation::MIRROR_CENTER );
         } else if( cropLocation == S::TOP_LEFT ) {
            return Pad( sizes, Option::CropLocation::TOP_LEFT );
         } else if( cropLocation == S::BOTTOM_RIGHT ) {
            return Pad( sizes, Option::CropLocation::BOTTOM_RIGHT );
         } else {
            DIP_THROW_INVALID_FLAG( cropLocation );
         }
      };

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
      DIP_EXPORT void Copy( Image const& src );

      /// \brief Deep copy, returns a copy of `this` with its own data.
      ///
      /// `this` must be forged.
      ///
      /// `dip::Image::QuickCopy` does the same, but without copying the data, its output image shares
      /// the data segment with `this`.
      ///
      /// Any external interface is not preserved. Use `dip::Copy` to control the data allocation for the output image.
      Image Copy() const {
         Image out;
         out.Copy( *this );
         return out;
      }

      /// \brief Converts the image to another data type.
      ///
      /// The data type conversion clips values to the target range and/or truncates them, as applicable.
      /// Complex values are converted to non-complex values by taking the absolute value.
      ///
      /// The data segment is replaced by a new one, unless the old and new data
      /// types have the same size and it is not shared with other images.
      /// If the data segment is replaced, strides are set to normal.
      ///
      /// A binary image can be converted to an 8-bit integer type without copying or touching the data.
      DIP_EXPORT void Convert( dip::DataType dt );

      /// \brief Expands the image's tensor, such that the tensor representation is a column-major matrix.
      ///
      /// If the image has a non-full tensor representation (diagonal, symmetric, triangular), or
      /// a row-major ordering, then the data segment is replaced by a new one. Otherwise, nothing is done.
      ///
      /// After calling this method, the object always has `dip::Tensor::HasNormalOrder` equal `true`.
      /// This method simplifies manipulating tensors by normalizing their storage.
      DIP_EXPORT void ExpandTensor();

      /// \brief Copies pixel data over to a new data segment if the strides are not normal.
      ///
      /// Will throw an exception if reallocating the data segment does not yield normal strides.
      /// This can happen only if there is an external interface.
      ///
      /// The image must be forged.
      ///
      /// \see HasNormalStrides, ForceContiguousData.
      void ForceNormalStrides() {
         if( !HasNormalStrides() ) {
            CopyDataToNewDataSegment();
            DIP_THROW_IF( !HasNormalStrides(), "Cannot force strides to normal" );
         }
      }

      /// \brief Copies pixel data over to a new data segment if the data is not contiguous.
      ///
      /// The image must be forged.
      ///
      /// \see HasContiguousData, ForceNormalStrides.
      void ForceContiguousData() {
         if( !HasContiguousData() ) {
            CopyDataToNewDataSegment();
            DIP_ASSERT( HasContiguousData() );
         }
      }

      /// \brief If the image shares its data segment with another image, create a data copy so it no longer
      /// shares data.
      ///
      /// The image must be forged.
      ///
      /// \see IsShared, Copy, ForceNormalStrides, ForceContiguousData
      void Separate() {
         if( IsShared() ) {
            CopyDataToNewDataSegment();
         }
      }

      /// \brief Sets all pixels in the image to the value `pixel`.
      ///
      /// `pixel` must have the same number of tensor elements as the image, or be a scalar.
      /// Its values will be clipped to the target range and/or truncated, as applicable.
      ///
      /// The image must be forged.
      DIP_EXPORT void Fill( Pixel const& pixel );

      /// \brief Sets all samples in the image to the value `sample`.
      ///
      /// The value will be clipped to the target range and/or truncated, as applicable.
      ///
      /// The image must be forged.
      DIP_EXPORT void Fill( Sample const& sample );

      /// \brief Sets all pixels in the image to the value `pixel`.
      ///
      /// `pixel` must have the same number of tensor elements as the image, or be a scalar.
      /// Its values will be clipped to the target range and/or truncated, as applicable.
      ///
      /// The image must be forged.
      Image& operator=( Pixel const& pixel ) {
         Fill( pixel );
         return *this;
      }

      /// \brief Sets all samples in the image to the value `sample`.
      ///
      /// The value will be clipped to the target range and/or truncated, as applicable.
      ///
      /// The image must be forged.
      Image& operator=( Sample const& sample ) {
         Fill( sample );
         return *this;
      }

      // This one is to disambiguate calling with a single initializer list. We don't mean UnsignedArray, we mean Pixel.
      template< typename T, typename = std::enable_if_t< IsSampleType< T >::value >>
      Image& operator=( std::initializer_list< T > values ) {
         Fill( Pixel( values ));
         return *this;
      }

      /// Returns the value of the first sample in the first pixel in the image as the given numeric type.
      template< typename T, typename = std::enable_if_t< IsNumericType< T >::value >>
      T As() const { return detail::CastSample< T >( dataType_, origin_ ); }

      /// \brief Returns a FloatArray containing the sample values of the first pixel in the image.
      /// For a complex-valued image, the modulus (absolute value) is returned.
      operator FloatArray() const;

      /// \}

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
      DataSegment dataBlock_;             // Holds the pixel data. Data block will be freed when last image that uses it is destroyed.
      void* origin_ = nullptr;            // Points to the origin ( pixel (0,0) ), not necessarily the first pixel of the data block.
      bool externalData_ = false;         // Is true if origin_ points to a data segment that was not allocated by DIPlib.
      dip::ExternalInterface* externalInterface_ = nullptr; // A function that will be called instead of the default forge function.

      //
      // Some private functions
      //

      // Are the strides such that no two samples are in the same memory cell?
      DIP_EXPORT bool HasValidStrides() const;

      DIP_EXPORT void GetDataBlockSizeAndStart( dip::uint& size, dip::sint& start ) const;

      DIP_EXPORT void GetDataBlockSizeAndStartWithTensor( dip::uint& size, dip::sint& start ) const;
      // `size` is the distance between top left and bottom right corners. `start` is the distance between
      // top left corner and origin (will be <0 if any strides[ii] < 0). All measured in samples.

      // Throws is `sizes_` is not good.
      DIP_NO_EXPORT static void TestSizes( UnsignedArray sizes ) {
         for( auto s : sizes ) {
            DIP_THROW_IF(( s == 0 ) || ( s > maxint ), "Sizes must be non-zero and no larger than " + std::to_string( maxint ));
         }
      }

      /// \brief Moves data from `other` to `this`. `this` will be identical to what `other` was, `other` will
      /// become a raw image.
      void move( Image&& other ) {
         using std::swap;
         dataType_ = other.dataType_;
         sizes_ = std::move( other.sizes_ );
         strides_ = std::move( other.strides_ );
         tensor_ = other.tensor_;
         tensorStride_ = other.tensorStride_;
         protect_ = other.protect_;
         colorSpace_ = std::move( other.colorSpace_ );
         pixelSize_ = std::move( other.pixelSize_ );
         swap( dataBlock_, other.dataBlock_ );  // prevent incrementing and decrementing counters
         other.dataBlock_ = nullptr;            // free previous data block, if any
         origin_ = other.origin_;
         other.origin_ = nullptr;               // keep `other` consistent
         externalData_ = other.externalData_;
         other.externalData_ = false;
         externalInterface_ = other.externalInterface_;
      }

      /// \brief Allocates a new data segment and copies the data over. The image will be the same as before, but
      /// have normal strides and not share data with another image.
      ///
      /// Don't call this function if the image is not forged.
      void CopyDataToNewDataSegment() {
         DIP_ASSERT( IsForged() );
         Image tmp;
         tmp.externalInterface_ = externalInterface_;
         tmp.ReForge( *this ); // This way we don't copy the strides. out.Copy( *this ) would do so if out is not yet forged!
         tmp.Copy( *this );
         this->move( std::move( tmp ));
      }

}; // class Image


//
// Utility functions
//

/// \brief You can output a `dip::Image` to `std::cout` or any other stream. Some
/// information about the image is printed.
DIP_EXPORT std::ostream& operator<<( std::ostream& os, Image const& img );

inline void swap( Image& v1, Image& v2 ) {
   v1.swap( v2 );
}

/// \brief Calls `img1.Aliases( img2 )`. See `dip::Image::Aliases`.
inline bool Alias( Image const& img1, Image const& img2 ) {
   return img1.Aliases( img2 );
}

/// \brief Makes a new image object pointing to same pixel data as `src`, but
/// with different origin, strides and size.
///
/// This function does the same as `dip::Image::At`, but allows for more flexible
/// defaults: If `origin`, `sizes` or `spacing` have only one value, that value is
/// repeated for each dimension. For empty arrays, `origin` defaults to all zeros,
/// `sizes` to `src.Sizes() - origin`, and `spacing` to all ones. These defaults
/// make it easy to crop pixels from one side of the image, to subsample the image,
/// etc. For example, the following code subsamples by a factor 2 in each dimension:
///
/// ```cpp
///     DefineROI( src, dest, {}, {}, { 2 } );
/// ```
DIP_EXPORT void DefineROI(
      Image const& src,
      Image& dest,
      UnsignedArray origin = {},
      UnsignedArray sizes = {},
      UnsignedArray spacing = {}
);
inline Image DefineROI(
      Image const& src,
      UnsignedArray const& origin,
      UnsignedArray const& sizes,
      UnsignedArray const& spacing = {}
) {
   Image dest;
   DefineROI( src, dest, origin, sizes, spacing );
   return dest;
}

/// \brief Copies samples over from `src` to `dest`, identical to the `dip::Image::Copy` method, except `dest` can
/// have an external interface.
inline void Copy( Image const& src, Image& dest ) {
   dest.Copy( src );
}
inline Image Copy( Image const& src ) {
   return src.Copy();
}

/// \brief Copies the pixels selected by `mask` in `src` over to `dest`. `dest` will be a 1D image.
DIP_EXPORT void CopyFrom( Image const& src, Image& dest, Image const& mask );

/// \brief Copies the pixels selected by `offsets` over from `src` to `dest`. `dest` will be a 1D image.
DIP_EXPORT void CopyFrom( Image const& src, Image& dest, IntegerArray const& offsets );

/// \brief Copies all pixels from `src` over to the pixels selected by `mask` in `dest`. `dest` must be forged.
DIP_EXPORT void CopyTo( Image const& src, Image& dest, Image const& mask );

/// \brief Copies all pixels from `src` over to the pixels selected by `offsets` in `dest`. `dest` must be forged.
DIP_EXPORT void CopyTo( Image const& src, Image& dest, IntegerArray const& offsets );

/// \brief Copies samples over from `src` to `dest`, expanding the tensor so it's a standard, column-major matrix.
///
/// If the tensor representation in `src` is one of those that do not save symmetric or zero values, to save space,
/// a new data segment will be allocated for `dest`, where the tensor representation is a column-major matrix
/// (`dest` will have `dip::Tensor::HasNormalOrder` be true). Otherwise, `dest` will share the data segment with `src`.
/// This function simplifies manipulating tensors by normalizing their storage.
///
/// \see dip::Copy, dip::Convert, dip::Image::ExpandTensor
inline void ExpandTensor( Image const& src, Image& dest ) {
   if( &src == &dest ) {
      dest.ExpandTensor();
   } else {
      dest = src;
      dest.ExpandTensor();
   }
}
inline Image ExpandTensor( Image const& src ) {
   Image dest = src;
   dest.ExpandTensor();
   return dest;
}

/// \brief Copies samples over from `src` to `dest`, with data type conversion.
///
/// If `dest` is forged,
/// has the same size as number of tensor elements as `src`, and has data type `dt`, then
/// its data segment is reused. If `src` and `dest` are the same object, its `dip::Image::Convert`
/// method is called instead.
///
/// The data type conversion clips values to the target range and/or truncates them, as applicable.
/// Complex values are converted to non-complex values by taking the absolute value.
inline void Convert( Image const& src, Image& dest, dip::DataType dt ) {
   if( &src == &dest ) {
      dest.Convert( dt );
   } else {
      dest.ReForge( src, dt );
      dest.Copy( src );
   }
}
inline Image Convert( Image src, dip::DataType dt ) { // taking `src` by value
   src.Convert( dt );
   return src;
}

/// \brief Creates a `dip::ImageRefArray` from a `dip::ImageArray`.
inline ImageRefArray CreateImageRefArray( ImageArray& imar ) {
   dip::ImageRefArray out;
   out.reserve( imar.size() );
   for( auto& im : imar ) {
      out.emplace_back( im );
   }
   return out;
}

/// \brief Creates a `dip::ImageConstRefArray` from a `dip::ImageArray`.
inline ImageConstRefArray CreateImageConstRefArray( ImageArray const& imar ) {
   dip::ImageConstRefArray out;
   out.reserve( imar.size() );
   for( auto const& im : imar ) {
      out.emplace_back( im );
   }
   return out;
}

/// \}

} // namespace dip

#endif // DIP_IMAGE_H
