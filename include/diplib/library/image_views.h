/*
 * DIPlib 3.0
 * This file contains definitions for support classes for the Image class.
 *
 * (c)2014-2017, Cris Luengo.
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


#ifndef DIP_IMAGE_VIEWS_H
#define DIP_IMAGE_VIEWS_H

#include "diplib/library/image.h"


/// \file
/// \brief Defines support classes for the `dip::Image` class. This file is always included through `diplib.h`.
/// \see infrastructure


/// \brief The `dip` namespace contains all the library functionality.
namespace dip {

/// \addtogroup infrastructure
/// \{


//
// dip::Image::Sample
//

/// \brief A sample represents a single numeric value in an image, see \ref image_representation.
///
/// Objects of this class are meant as an interface between images and numbers. These objects are
/// not actually how values are stored in an image, but rather represent a reference to a sample
/// in an image. Through this reference, individual samples in an image can be changed. For example:
///
/// ```cpp
///     dip::Image img( { 256, 256 } );
///     img.At( 10, 20 )[ 0 ] = 3;
/// ```
///
/// In the code above, `img.At( 10, 20 )[ 0 ]` returns a `%Sample` object. Assigning to this object
/// changes the sample in `img` that is referenced.
///
/// See \ref indexing for more information.
///
/// \see dip::Image::Pixel, dip::Image::View, dip::Image::CastSample
class Image::Sample {
      friend class Pixel; // This is necessary so that Pixel::Iterator can modify origin_.

   public:

      // Default copy constructor doesn't do what we need
      Sample( Sample const& sample ) : dataType_( sample.dataType_ ) {
         origin_ = &buffer_;
         std::memcpy( origin_, sample.origin_, dataType_.SizeOf() );
      }

      // Default move constructor, otherwise it's implicitly deleted.
      Sample( Sample&& ) = default;

      // Construct a Sample over existing data, used by dip::Image, dip::GenericImageIterator,
      // dip::GenericJointImageIterator.
      constexpr Sample( void* data, dip::DataType dataType ) : origin_( data ), dataType_( dataType ) {}

      /// Construct a new `%Sample` by giving the data type. Initialized to 0.
      explicit Sample( dip::DataType dataType = DT_SFLOAT ) : dataType_( dataType ) {
         buffer_ = { 0.0, 0.0 };
         // The buffer filled with zeros yields a zero value no matter as what data type we interpret it.
      }

      /// A numeric value implicitly converts to a `%Sample`.
      template< typename T, typename = std::enable_if_t< IsSampleType< T >::value >>
      constexpr Sample( T value ) {
         dataType_ = dip::DataType( value );
         *static_cast< T* >( origin_ ) = value;
      }
      constexpr Sample( bool value ) : dataType_( DT_BIN ) {
         *static_cast< bin* >( origin_ ) = value;
      }
      #if SIZE_MAX != UINT32_MAX // we don't want to compile the next two on 32-bit machines, they'd conflict with s/uint32 constructors above.
      constexpr Sample( dip::uint value ) : dataType_( DT_UINT32 ) {
         *static_cast< uint32* >( origin_ ) = clamp_cast< uint32 >( value );
      }
      constexpr Sample( dip::sint value ) : dataType_( DT_SINT32 ) {
         *static_cast< sint32* >( origin_ ) = clamp_cast< sint32 >( value );
      }
      #endif

      /// A `dip::Image::Pixel`, when cast to a `%Sample`, references the first value in the pixel.
      Sample( Pixel const& pixel );

      /// A `dip::Image`, when cast to a `%Sample`, references the first sample in the first pixel in the image.
      explicit Sample( Image const& image ) : origin_( image.Origin() ), dataType_( image.DataType() ) {}

      /// Swaps `*this` and `other`.
      void swap( Sample& other ) {
         using std::swap;
         bool thisInternal = origin_ == &buffer_;
         bool otherInternal = other.origin_ == &other.buffer_;
         if( thisInternal ) {
            if( otherInternal ) {
               swap( buffer_, other.buffer_ );
            } else {
               origin_ = other.origin_;
               other.buffer_ = buffer_;
               other.origin_ = &other.buffer_;
            }
         } else {
            if( otherInternal ) {
               other.origin_ = origin_;
               buffer_ = other.buffer_;
               origin_ = &buffer_;
            } else {
               swap( origin_, other.origin_ );
            }
         }
         swap( dataType_, other.dataType_ );
      }

      void swap( Sample&& other ) { swap( other ); };

      /// Returns the value of the sample as the given numeric type, similar to using `static_cast`.
      template< typename T, typename = std::enable_if_t< IsNumericType< T >::value >>
      constexpr T As() const { return detail::CastSample< T >( dataType_, origin_ ); }

      /// A `%Sample` can be cast to basic numerical types.
      constexpr explicit operator bool() const { return As< bin >(); }
      /// A `%Sample` can be cast to basic numerical types.
      constexpr explicit operator dip::uint() const { return As< dip::uint >(); }
      /// A `%Sample` can be cast to basic numerical types.
      constexpr explicit operator dip::sint() const { return As< dip::sint >(); }
      /// A `%Sample` can be cast to basic numerical types.
      constexpr explicit operator sfloat() const { return As< sfloat >(); }
      /// A `%Sample` can be cast to basic numerical types.
      constexpr explicit operator dfloat() const { return As< dfloat >(); }
      /// A `%Sample` can be cast to basic numerical types.
      constexpr explicit operator scomplex() const { return As< scomplex >(); }
      /// A `%Sample` can be cast to basic numerical types.
      constexpr explicit operator dcomplex() const { return As< dcomplex >(); }

      /// Assigning to a `%Sample` copies the value over to the sample referenced.
      constexpr Sample& operator=( Sample const& sample ) {
         detail::CastSample( sample.dataType_, sample.origin_, dataType_, origin_ );
         return *this;
      }
      constexpr Sample& operator=( Sample&& sample ) {
         detail::CastSample( sample.dataType_, sample.origin_, dataType_, origin_ );
         return *this;
      }
      template< typename T >
      constexpr Sample& operator=( CastSample< T > const& sample ) {
         return operator=( static_cast< Sample const& >( sample ));
      }

      /// It is also possible to assign a constant directly.
      template< typename T, typename = std::enable_if_t< IsSampleType< T >::value >>
      constexpr Sample& operator=( T value ) {
         detail::CastSample( dip::DataType( value ), &value, dataType_, origin_ );
         return *this;
      }
      constexpr Sample& operator=( bool value ) {
         detail::CastSample( DT_BIN, &value, dataType_, origin_ );
         return *this;
      }
      #if SIZE_MAX != UINT32_MAX // we don't want to compile the next two on 32-bit machines, they'd conflict with s/uint32 constructors above.
      constexpr Sample& operator=( dip::uint value ) {
         uint32 tmp = clamp_cast< uint32 >( value );
         detail::CastSample( DT_UINT32, &tmp, dataType_, origin_ );
         return *this;
      }
      constexpr Sample& operator=( dip::sint value ) {
         sint32 tmp = clamp_cast< sint32 >( value );
         detail::CastSample( DT_SINT32, &tmp, dataType_, origin_ );
         return *this;
      }
      #endif

      /// Returns a pointer to the sample referenced.
      constexpr void* Origin() const { return origin_; }

      /// The data type of the sample referenced.
      constexpr dip::DataType DataType() const { return dataType_; }

      /// \brief Compound assignment operator.
      template< typename T >
      Sample& operator+=( T const& rhs ) {
         return *this = dataType_.IsComplex()
                        ? detail::CastSample< dcomplex >( dataType_, origin_ ) + static_cast< dcomplex >( rhs )
                        : detail::CastSample< dfloat >( dataType_, origin_ ) + rhs;
      }

      /// \brief Compound assignment operator.
      template< typename T >
      Sample& operator-=( T const& rhs ) {
         return *this = dataType_.IsComplex()
                        ? detail::CastSample< dcomplex >( dataType_, origin_ ) - static_cast< dcomplex >( rhs )
                        : detail::CastSample< dfloat >( dataType_, origin_ ) - rhs;
      }

      /// \brief Compound assignment operator.
      template< typename T >
      Sample& operator*=( T const& rhs ) {
         return *this = dataType_.IsComplex()
                        ? detail::CastSample< dcomplex >( dataType_, origin_ ) * static_cast< dcomplex >( rhs )
                        : detail::CastSample< dfloat >( dataType_, origin_ ) * rhs;
      }

      /// \brief Compound assignment operator.
      template< typename T >
      Sample& operator/=( T const& rhs ) {
         return *this = dataType_.IsComplex()
                        ? detail::CastSample< dcomplex >( dataType_, origin_ ) / static_cast< dcomplex >( rhs )
                        : detail::CastSample< dfloat >( dataType_, origin_ ) / rhs;
      }

      /// \brief Compound assignment operator.
      template< typename T >
      Sample& operator%=( T const& rhs );
      /// \brief Bit-wise compound assignment operator.
      template< typename T >
      Sample& operator&=( T const& rhs );
      /// \brief Bit-wise compound assignment operator.
      template< typename T >
      Sample& operator|=( T const& rhs );
      /// \brief Bit-wise compound assignment operator.
      template< typename T >
      Sample& operator^=( T const& rhs );

   protected:
      dcomplex buffer_;
      void* origin_ = &buffer_;
      dip::DataType dataType_;
};

inline void swap( Image::Sample& v1, Image::Sample& v2 ) { v1.swap( v2 ); }


/// \brief You can output a `dip::Image::Sample` to `std::cout` or any other stream.
/// It is printed like any numeric value of the same type.
inline std::ostream& operator<<(
      std::ostream& os,
      Image::Sample const& sample
) {
   switch( sample.DataType() ) {
      case DT_BIN:
         os << sample.As< bin >(); break;
      case DT_UINT8:
      case DT_UINT16:
      case DT_UINT32:
         os << sample.As< uint32 >(); break;
      default: // signed integers
         os << sample.As< sint32 >(); break;
      case DT_SFLOAT:
      case DT_DFLOAT:
         os << sample.As< dfloat >(); break;
      case DT_SCOMPLEX:
      case DT_DCOMPLEX:
         os << sample.As< dcomplex >(); break;
   }
   return os;
}


//
// dip::Image::Pixel
//

/// \brief A pixel represents a set of numeric value in an image, see \ref image_representation.
///
/// Objects of this class are meant as an interface between images and numbers. These objects are
/// not actually how pixels are stored in an image, but rather represent a reference to a pixel
/// in an image. Through this reference, individual pixels in an image can be changed. For example:
///
/// ```cpp
///     dip::Image img( { 256, 256 }, 3 );
///     img.At( 10, 20 ) = { 4, 5, 6 };
/// ```
///
/// In the code above, `img.At( 10, 20 )` returns a `%Pixel` object. Assigning to this object
/// changes the pixel in `img` that is referenced.
///
/// See \ref indexing for more information.
///
/// \see dip::Image::Sample, dip::Image::View, dip::Image::CastPixel
class Image::Pixel {
   public:

      // Default copy constructor doesn't do what we need
      Pixel( Pixel const& pixel ) : dataType_( pixel.dataType_ ), tensor_( pixel.tensor_ ) {
         SetInternalData();
         operator=( pixel );
      }

      // Default move constructor, otherwise it's implicitly deleted.
      Pixel( Pixel&& ) = default;

      // Construct a Pixel over existing data, used by dip::Image, dip::GenericImageIterator,
      // dip::GenericJointImageIterator, dml::GetArray.
      Pixel( void* data, dip::DataType dataType, dip::Tensor const& tensor, dip::sint tensorStride ) :
            origin_( data ), dataType_( dataType ), tensor_( tensor ), tensorStride_( tensorStride ) {}

      /// Construct a new `%Pixel` by giving data type and number of tensor elements. Initialized to 0.
      explicit Pixel( dip::DataType dataType = DT_SFLOAT, dip::uint tensorElements = 1 ) :
            dataType_( dataType ), tensor_( tensorElements ) {
         SetInternalData();
         std::fill( buffer_.begin(), buffer_.end(), 0 );
      }

      /// \brief A `%Pixel` can be constructed from a single sample, yielding a scalar pixel with the same
      /// data type as the sample.
      Pixel( Sample const& sample ) : dataType_( sample.DataType() ) { // tensor_ is scalar by default
         SetInternalData();
         std::memcpy( buffer_.data(), sample.Origin(), dataType_.SizeOf() );
      }

      /// \brief A `%Pixel` can be constructed from a `dip::FloatArray`. The pixel will be a column vector.
      DIP_EXPORT explicit Pixel( FloatArray const& values, dip::DataType dt = DT_SFLOAT );

      /// \brief A `%Pixel` can be constructed from an initializer list, yielding a pixel with the same data
      /// type and number of tensor elements as the initializer list. The pixel will be a column vector.
      template< typename T, typename std::enable_if_t< IsSampleType< T >::value, int > = 0 >
      Pixel( std::initializer_list< T > values ) {
         dip::uint N = values.size();
         tensor_.SetVector( N );
         dataType_ = dip::DataType( T( 0 ));
         SetInternalData();
         dip::uint sz = dataType_.SizeOf();
         uint8* dest = buffer_.data();
         for( auto it = values.begin(); it != values.end(); ++it ) {
            std::memcpy( dest, &*it, sz );
            dest += sz;
         }
      }

      /// A `dip::Image`, when cast to a `%Pixel`, references the first pixel in the image.
      explicit Pixel( Image const& image ) :
            origin_( image.Origin() ),
            dataType_( image.DataType() ),
            tensor_( image.Tensor() ),
            tensorStride_( image.TensorStride() ) {}

      /// Swaps `*this` and `other`.
      void swap( Pixel& other ) {
         using std::swap;
         bool thisInternal = origin_ == buffer_.data();
         bool otherInternal = other.origin_ == other.buffer_.data();
         if( thisInternal ) {
            if( otherInternal ) {
               swap( buffer_, other.buffer_ );
               origin_ = buffer_.data();
               other.origin_ = other.buffer_.data();
            } else {
               origin_ = other.origin_;
               other.buffer_ = std::move( buffer_ );
               other.origin_ = other.buffer_.data();
            }
         } else {
            if( otherInternal ) {
               other.origin_ = origin_;
               buffer_ = std::move( other.buffer_ );
               origin_ = buffer_.data();
            } else {
               swap( origin_, other.origin_ );
            }
         }
         swap( dataType_, other.dataType_ );
         swap( tensor_, other.tensor_ );
         swap( tensorStride_, other.tensorStride_ );
      }

      void swap( Pixel&& other ) { swap( other ); }

      /// Returns the value of the first sample in the pixel as the given numeric type, similar to using `static_cast`.
      template< typename T, typename = std::enable_if_t< IsNumericType< T >::value >>
      T As() const { return detail::CastSample< T >( dataType_, origin_ ); }

      /// A `%Pixel` can be cast to basic numerical types. The first sample in the pixel is used.
      explicit operator bool() const { return As< bin >(); }
      /// A `%Pixel` can be cast to basic numerical types. The first sample in the pixel is used.
      explicit operator dip::uint() const { return As< dip::uint >(); }
      /// A `%Pixel` can be cast to basic numerical types. The first sample in the pixel is used.
      explicit operator dip::sint() const { return As< dip::sint >(); }
      /// A `%Pixel` can be cast to basic numerical types. The first sample in the pixel is used.
      explicit operator sfloat() const { return As< sfloat >(); }
      /// A `%Pixel` can be cast to basic numerical types. The first sample in the pixel is used.
      explicit operator dfloat() const { return As< dfloat >(); }
      /// A `%Pixel` can be cast to basic numerical types. The first sample in the pixel is used.
      explicit operator scomplex() const { return As< scomplex >(); }
      /// A `%Pixel` can be cast to basic numerical types. The first sample in the pixel is used.
      explicit operator dcomplex() const { return As< dcomplex >(); }

      /// \brief Returns a FloatArray containing the sample values of the pixel.
      /// For a complex-valued pixel, the modulus (absolute value) is returned.
      DIP_EXPORT operator FloatArray() const;

      /// Assigning a number or sample to a `%Pixel` copies the value over each of the samples in the pixel.
      Pixel& operator=( Sample const& sample ) {
         dip::uint N = tensor_.Elements();
         dip::uint sz = dataType_.SizeOf();
         uint8* dest = static_cast< uint8* >( origin_ );
         detail::CastSample( sample.DataType(), sample.Origin(), dataType_, dest );
         uint8* src = dest;
         for( dip::uint ii = 1; ii < N; ++ii ) {
            dest += static_cast< dip::sint >( sz ) * tensorStride_;
            std::memcpy( dest, src, sz );
         }
         return *this;
      }
      /// Assigning to a `%Pixel` copies the values over to the pixel referenced.
      Pixel& operator=( Pixel const& pixel ) {
         dip::uint N = tensor_.Elements();
         DIP_THROW_IF( pixel.TensorElements() != N, E::NTENSORELEM_DONT_MATCH );
         dip::sint srcSz = static_cast< dip::sint >( pixel.DataType().SizeOf() );
         dip::sint destSz = static_cast< dip::sint >( dataType_.SizeOf() );
         uint8* src = static_cast< uint8* >( pixel.Origin() );
         uint8* dest = static_cast< uint8* >( origin_ );
         for( dip::uint ii = 0; ii < N; ++ii ) {
            detail::CastSample( pixel.DataType(), src, dataType_, dest );
            src += srcSz * pixel.TensorStride();
            dest += destSz * tensorStride_;
         }
         return *this;
      }
      Pixel& operator=( Pixel&& pixel ) {
         return operator=( const_cast< Pixel const& >( pixel )); // Call copy assignment instead
      }
      template< typename T >
      Pixel& operator=( CastPixel< T > const& pixel ) {
         return operator=( static_cast< Pixel const& >( pixel ));
      }
      /// It is also possible to assign from an initializer list.
      template< typename T, typename = std::enable_if_t< IsSampleType< T >::value >>
      Pixel& operator=( std::initializer_list< T > values ) {
         dip::uint N = tensor_.Elements();
         DIP_THROW_IF( values.size() != N, E::NTENSORELEM_DONT_MATCH );
         dip::DataType srcDT = dip::DataType( T( 0 ));
         dip::sint destSz = static_cast< dip::sint >( dataType_.SizeOf() );
         uint8* dest = static_cast< uint8* >( origin_ );
         for( auto it = values.begin(); it != values.end(); ++it ) {
            detail::CastSample( srcDT, &(*it), dataType_, dest );
            dest += destSz * tensorStride_;
         }
         return *this;
      }

      /// Returns a pointer to the first sample referenced.
      void* Origin() const { return origin_; }
      /// The data type of the pixel referenced.
      dip::DataType DataType() const { return dataType_; }
      /// The tensor shape for the pixel referenced.
      dip::Tensor const& Tensor() const { return tensor_; }
      /// The number of samples in the pixel referenced.
      dip::uint TensorElements() const { return tensor_.Elements(); }
      /// Is it a scalar pixel?
      bool IsScalar() const { return tensor_.IsScalar(); }
      /// The stride to use to access the various samples in the pixel referenced.
      dip::sint TensorStride() const { return tensorStride_; }

      /// \brief Change the tensor shape, without changing the number of tensor elements.
      Pixel& ReshapeTensor( dip::uint rows, dip::uint cols ) {
         DIP_THROW_IF( tensor_.Elements() != rows * cols, "Cannot reshape tensor to requested sizes" );
         tensor_.ChangeShape( rows );
         return *this;
      }

      /// \brief Change the tensor shape, without changing the number of tensor elements.
      Pixel& ReshapeTensor( dip::Tensor const& other ) {
         tensor_.ChangeShape( other );
         return *this;
      }

      /// \brief Change the tensor to a vector, without changing the number of tensor elements.
      Pixel& ReshapeTensorAsVector() {
         tensor_.ChangeShape();
         return *this;
      }

      /// \brief Change the tensor to a diagonal matrix, without changing the number of tensor elements.
      Pixel& ReshapeTensorAsDiagonal() {
         dip::Tensor other{ dip::Tensor::Shape::DIAGONAL_MATRIX, tensor_.Elements(), tensor_.Elements() };
         tensor_.ChangeShape( other );
         return *this;
      }

      /// Indexing into a `%Pixel` retrieves a reference to the specific sample.
      Sample operator[]( dip::uint index ) const {
         DIP_ASSERT( index < tensor_.Elements() );
         dip::uint sz = dataType_.SizeOf();
         return Sample(
               static_cast< uint8* >( origin_ ) + static_cast< dip::sint >( sz * index ) * tensorStride_,
               dataType_ );
      }
      /// Indexing into a `%Pixel` retrieves a reference to the specific sample, `indices` must have one or two elements.
      Sample operator[]( UnsignedArray const& indices ) const {
         DIP_START_STACK_TRACE
            dip::uint index = tensor_.Index( indices );
            return operator[]( index );
         DIP_END_STACK_TRACE
      }

      /// \brief Extracts the tensor elements along the diagonal.
      Pixel Diagonal() const {
         Pixel out( *this );
         out.tensor_.ExtractDiagonal( out.tensorStride_ );
         return out;
      }

      /// \brief Extracts the tensor elements along the given row. The tensor representation must be full
      /// (i.e. no symmetric or triangular matrices).
      Pixel TensorRow( dip::uint index ) const {
         DIP_THROW_IF( index >= tensor_.Rows(), E::INDEX_OUT_OF_RANGE );
         Pixel out( *this );
         DIP_START_STACK_TRACE
            dip::sint offset = out.tensor_.ExtractRow( index, out.tensorStride_ );
            out.origin_ = static_cast< uint8* >( out.origin_ ) + offset * static_cast< dip::sint >( dataType_.SizeOf() );
         DIP_END_STACK_TRACE
         return out;
      }

      /// \brief Extracts the tensor elements along the given column. The tensor representation must be full
      /// (i.e. no symmetric or triangular matrices).
      Pixel TensorColumn( dip::uint index ) const {
         DIP_THROW_IF( index >= tensor_.Columns(), E::INDEX_OUT_OF_RANGE );
         Pixel out( *this );
         DIP_START_STACK_TRACE
            dip::sint offset = out.tensor_.ExtractColumn( index, out.tensorStride_ );
            out.origin_ = static_cast< uint8* >( out.origin_ ) + offset * static_cast< dip::sint >( dataType_.SizeOf() );
         DIP_END_STACK_TRACE
         return out;
      }

      /// \brief Extracts the real component of the pixel values, returns an identical copy if the data type is
      /// not complex.
      Pixel Real() const {
         Pixel out = *this;
         if( dataType_.IsComplex() ) {
            // Change data type
            out.dataType_ = dataType_ == DT_SCOMPLEX ? DT_SFLOAT : DT_DFLOAT;
            // Sample size is halved, meaning stride must be doubled
            out.tensorStride_ *= 2;
         }
         return out;
      }

      /// \brief Extracts the imaginary component of the pixel values, throws an exception if the data type
      /// is not complex.
      Pixel Imaginary() const {
         DIP_THROW_IF( !dataType_.IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
         Pixel out = *this;
         // Change data type
         out.dataType_ = dataType_ == DT_SCOMPLEX ? DT_SFLOAT : DT_DFLOAT;
         // Sample size is halved, meaning stride must be doubled
         out.tensorStride_ *= 2;
         // Change the offset
         out.origin_ = static_cast< uint8* >( out.origin_ ) + out.dataType_.SizeOf();
         return out;
      }

      /// \brief An iterator to iterate over the samples in the pixel. Mutable forward iterator.
      class Iterator {
         public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = Sample;
            using difference_type = dip::sint;
            using reference = value_type&;
            using pointer = value_type*;

            Iterator() : value_( nullptr, DT_BIN ), tensorStride_( 0 ) {}

            void swap( Iterator& other ) {
               value_.swap( other.value_ );
               std::swap( tensorStride_, other.tensorStride_ );
            }

            reference operator*() { return value_; }
            pointer operator->() { return &value_; }

            Iterator& operator++() {
               value_.origin_ = static_cast< uint8* >( value_.origin_ ) +
                                tensorStride_ * static_cast< dip::sint >( value_.dataType_.SizeOf() );
               return *this;
            }
            Iterator operator++( int ) { Iterator tmp( *this ); operator++(); return tmp; }

            bool operator==( Iterator const& other ) const { return value_.Origin() == other.value_.Origin(); }
            bool operator!=( Iterator const& other ) const { return !operator==( other ); }

         protected:
            value_type value_;
            dip::sint tensorStride_;

            // These classes need to use the private constructors:
            friend class Pixel;
            friend class View;
            template< typename T > friend class dip::GenericImageIterator;
            template< dip::uint N, typename T > friend class dip::GenericJointImageIterator;

            Iterator( void* origin, dip::DataType dataType, dip::sint tensorStride ):
                  value_( origin, dataType ),
                  tensorStride_( tensorStride ) {}
            Iterator( void* origin, dip::DataType dataType, dip::sint tensorStride, dip::uint index ) :
                  value_( static_cast< uint8* >( origin ) + tensorStride * static_cast< dip::sint >( index * dataType.SizeOf() ), dataType ),
                  tensorStride_( tensorStride ) {}
      };

      /// Returns an iterator to the first sample in the pixel.
      Iterator begin() const { return Iterator( origin_, dataType_, tensorStride_ ); }
      /// Returns an iterator to one past the last sample in the pixel.
      Iterator end() const { return Iterator( origin_, dataType_, tensorStride_, tensor_.Elements() ); }

      /// True if all tensor elements are non-zero.
      bool All() const {
         for( auto it = begin(); it != end(); ++it ) {
            if( !( it->As< bool >() )) {
               return false;
            }
         }
         return true;
      }

      /// True if one tensor element is non-zero.
      bool Any() const {
         for( auto it = begin(); it != end(); ++it ) {
            if( it->As< bool >() ) {
               return true;
            }
         }
         return false;
      }

      /// \brief Compound assignment operator.
      template< typename T >
      Pixel& operator+=( T const& rhs );
      /// \brief Compound assignment operator.
      template< typename T >
      Pixel& operator-=( T const& rhs );
      /// \brief Compound assignment operator.
      template< typename T >
      Pixel& operator*=( T const& rhs );
      /// \brief Compound assignment operator.
      template< typename T >
      Pixel& operator/=( T const& rhs );
      /// \brief Compound assignment operator.
      template< typename T >
      Pixel& operator%=( T const& rhs );
      /// \brief Bit-wise compound assignment operator.
      template< typename T >
      Pixel& operator&=( T const& rhs );
      /// \brief Bit-wise compound assignment operator.
      template< typename T >
      Pixel& operator|=( T const& rhs );
      /// \brief Bit-wise compound assignment operator.
      template< typename T >
      Pixel& operator^=( T const& rhs );

   protected:
      std::vector< uint8 > buffer_;
      void* origin_;
      dip::DataType dataType_;
      dip::Tensor tensor_;
      dip::sint tensorStride_ = 1;

      void SetInternalData() {
         buffer_.resize( dataType_.SizeOf() * tensor_.Elements() );
         origin_ = buffer_.data();
      }
};

inline void swap( Image::Pixel& v1, Image::Pixel& v2 ) { v1.swap( v2 ); }
inline void swap( Image::Pixel::Iterator& v1, Image::Pixel::Iterator& v2 ) { v1.swap( v2 ); }

/// \brief Arithmetic operator, element-wise.
DIP_EXPORT Image::Pixel operator+( Image::Pixel const& lhs, Image::Pixel const& rhs );
template< typename T, typename = std::enable_if_t< IsNumericType< T >::value >>
Image::Pixel operator+( Image::Pixel const& lhs, T const& rhs ) { return operator+( lhs, Image::Pixel{ rhs } ); }

/// \brief Arithmetic operator, element-wise.
DIP_EXPORT Image::Pixel operator-( Image::Pixel const& lhs, Image::Pixel const& rhs );
template< typename T, typename = std::enable_if_t< IsNumericType< T >::value >>
Image::Pixel operator-( Image::Pixel const& lhs, T const& rhs ) { return operator-( lhs, Image::Pixel{ rhs } ); }

/// \brief Arithmetic operator, tensor multiplication.
DIP_EXPORT Image::Pixel operator*( Image::Pixel const& lhs, Image::Pixel const& rhs );
template< typename T, typename = std::enable_if_t< IsNumericType< T >::value >>
Image::Pixel operator*( Image::Pixel const& lhs, T const& rhs ) { return operator*( lhs, Image::Pixel{ rhs } ); }

/// \brief Arithmetic operator, element-wise.
DIP_EXPORT Image::Pixel operator/( Image::Pixel const& lhs, Image::Pixel const& rhs );
template< typename T, typename = std::enable_if_t< IsNumericType< T >::value >>
Image::Pixel operator/( Image::Pixel const& lhs, T const& rhs ) { return operator/( lhs, Image::Pixel{ rhs } ); }

/// \brief Arithmetic operator, element-wise.
DIP_EXPORT Image::Pixel operator%( Image::Pixel const& lhs, Image::Pixel const& rhs );
template< typename T, typename = std::enable_if_t< IsNumericType< T >::value >>
Image::Pixel operator%( Image::Pixel const& lhs, T const& rhs ) { return operator%( lhs, Image::Pixel{ rhs } ); }

/// \brief Bit-wise operator, element-wise.
DIP_EXPORT Image::Pixel operator&( Image::Pixel const& lhs, Image::Pixel const& rhs );
template< typename T, typename = std::enable_if_t< IsNumericType< T >::value >>
Image::Pixel operator&( Image::Pixel const& lhs, T const& rhs ) { return operator&( lhs, Image::Pixel{ rhs } ); }

/// \brief Bit-wise operator, element-wise.
DIP_EXPORT Image::Pixel operator|( Image::Pixel const& lhs, Image::Pixel const& rhs );
template< typename T, typename = std::enable_if_t< IsNumericType< T >::value >>
Image::Pixel operator|( Image::Pixel const& lhs, T const& rhs ) { return operator|( lhs, Image::Pixel{ rhs } ); }

/// \brief Bit-wise operator, element-wise.
DIP_EXPORT Image::Pixel operator^( Image::Pixel const& lhs, Image::Pixel const& rhs );
template< typename T, typename = std::enable_if_t< IsNumericType< T >::value >>
Image::Pixel operator^( Image::Pixel const& lhs, T const& rhs ) { return operator^( lhs, Image::Pixel{ rhs } ); }

/// \brief Unary operator, element-wise.
DIP_EXPORT Image::Pixel operator-( Image::Pixel const& in );

/// \brief Bit-wise unary operator operator.
DIP_EXPORT Image::Pixel operator~( Image::Pixel const& in );

/// \brief Boolean unary operator, element-wise.
DIP_EXPORT Image::Pixel operator!( Image::Pixel const& in );

/// \brief Comparison operator, can only be true if the two pixels have compatible number of tensor elements.
DIP_EXPORT bool operator==( Image::Pixel const& lhs, Image::Pixel const& rhs );
template< typename T, typename = std::enable_if_t< IsNumericType< T >::value >>
bool operator==( Image::Pixel const& lhs, T const& rhs ) { return operator==( lhs, Image::Pixel{ rhs } ); }

/// \brief Comparison operator, equivalent to `!(lhs==rhs)`.
template< typename T, typename = std::enable_if_t< IsNumericType< T >::value >>
bool operator!=( Image::Pixel const& lhs, T const& rhs ) { return !operator==( lhs, rhs ); }

/// \brief Comparison operator, can only be true if the two pixels have compatible number of tensor elements.
DIP_EXPORT bool operator< ( Image::Pixel const& lhs, Image::Pixel const& rhs );
template< typename T, typename = std::enable_if_t< IsNumericType< T >::value >>
bool operator< ( Image::Pixel const& lhs, T const& rhs ) { return operator< ( lhs, Image::Pixel{ rhs } ); }

/// \brief Comparison operator, can only be true if the two pixels have compatible number of tensor elements.
DIP_EXPORT bool operator> ( Image::Pixel const& lhs, Image::Pixel const& rhs );
template< typename T, typename = std::enable_if_t< IsNumericType< T >::value >>
bool operator> ( Image::Pixel const& lhs, T const& rhs ) { return operator> ( lhs, Image::Pixel{ rhs } ); }

/// \brief Comparison operator, can only be true if the two pixels have compatible number of tensor elements.
DIP_EXPORT bool operator<=( Image::Pixel const& lhs, Image::Pixel const& rhs );
template< typename T, typename = std::enable_if_t< IsNumericType< T >::value >>
bool operator<=( Image::Pixel const& lhs, T const& rhs ) { return operator<=( lhs, Image::Pixel{ rhs } ); }

/// \brief Comparison operator, can only be true if the two pixels have compatible number of tensor elements.
DIP_EXPORT bool operator>=( Image::Pixel const& lhs, Image::Pixel const& rhs );
template< typename T, typename = std::enable_if_t< IsNumericType< T >::value >>
bool operator>=( Image::Pixel const& lhs, T const& rhs ) { return operator>=( lhs, Image::Pixel{ rhs } ); }

template< typename T >
Image::Pixel& Image::Pixel::operator+=( T const& rhs ) { return *this = operator+( *this, rhs ); }
template< typename T >
Image::Pixel& Image::Pixel::operator-=( T const& rhs ) { return *this = operator-( *this, rhs ); }
template< typename T >
Image::Pixel& Image::Pixel::operator*=( T const& rhs ) { return *this = operator*( *this, rhs ); }
template< typename T >
Image::Pixel& Image::Pixel::operator/=( T const& rhs ) { return *this = operator/( *this, rhs ); }
template< typename T >
Image::Pixel& Image::Pixel::operator%=( T const& rhs ) { return *this = operator%( *this, rhs ); }
template< typename T >
Image::Pixel& Image::Pixel::operator&=( T const& rhs ) { return *this = operator&( *this, rhs ); }
template< typename T >
Image::Pixel& Image::Pixel::operator|=( T const& rhs ) { return *this = operator|( *this, rhs ); }
template< typename T >
Image::Pixel& Image::Pixel::operator^=( T const& rhs ) { return *this = operator^( *this, rhs ); }

// dip::Image::Sample operators below are too difficult to implement with only dcomplex and dfloat types.
template< typename T >
Image::Sample& Image::Sample::operator%=( T const& rhs ) { return *this = operator%( Image::Pixel( *this ), rhs )[ 0 ]; }
template< typename T >
Image::Sample& Image::Sample::operator&=( T const& rhs ) { return *this = operator&( Image::Pixel( *this ), rhs )[ 0 ]; }
template< typename T >
Image::Sample& Image::Sample::operator|=( T const& rhs ) { return *this = operator|( Image::Pixel( *this ), rhs )[ 0 ]; }
template< typename T >
Image::Sample& Image::Sample::operator^=( T const& rhs ) { return *this = operator^( Image::Pixel( *this ), rhs )[ 0 ]; }

// This dip::Image::Sample constructor depends on the definition of dip::Image::Pixel
inline Image::Sample::Sample( Image::Pixel const& pixel ) : origin_( pixel.Origin() ), dataType_( pixel.DataType() ) {}

/// \brief You can output a `dip::Image::Pixel` to `std::cout` or any other stream.
/// It is printed as a sequence of values, prepended with "Pixel with values:".
inline std::ostream& operator<<(
      std::ostream& os,
      Image::Pixel const& pixel
) {
   dip::uint N = pixel.TensorElements();
   if( N == 1 ) {
      os << "Pixel with value: " << pixel[ 0 ];
   } else {
      os << "Pixel with values: " << pixel[ 0 ];
      for( dip::uint ii = 1; ii < N; ++ii ) {
         os << ", " << pixel[ ii ];
      }
   }
   return os;
}


//
// dip::Image::CastSample and dip::Image::CastPixel
//

/// \brief Derived from `dip::Image::Sample`, works identically except it implicitly converts to type `T`.
template< class T >
class Image::CastSample : public Image::Sample {
   public:
      using Sample::Sample;
      CastSample( Sample&& sample ) : Sample( std::move( sample )) {}
      using Sample::operator=;
      operator T() const { return As< T >(); }

      /// For some reason, MSVC needs this for disambiguation; the cast operator is not enough
      bool operator==( T value ) const { return As< T >() == value; }
};

/// \brief Derived from `dip::Image::Pixel`, works identically except it implicitly converts to type `T`.
template< class T >
class Image::CastPixel : public Image::Pixel {
      friend class Image;
      template< typename S > friend class dip::GenericImageIterator;
      template< dip::uint N, typename S > friend class dip::GenericJointImageIterator;
   public:
      using Pixel::Pixel;
      CastPixel( Pixel&& pixel ) : Pixel( std::move( pixel )) {}
      using Pixel::operator=;
      operator T() const { return As< T >(); }
      CastSample< T > operator[]( dip::uint index ) const { return Pixel::operator[]( index ); }
      CastSample< T > operator[]( UnsignedArray const& indices ) const { return Pixel::operator[]( indices ); }
};


//
// dip::Image::View
//

/// \brief A view represents a subset of samples in an image. It can be assigned to to change those samples.
///
/// Objects of this class are meant as an interface for indexing into images. These objects reference a subset
/// of pixels or samples within the image, and when assigned to, change those values. All indexing operators,
/// by returning a view rather than a new image, thus allow also *subscripted assignment*, assignment of new
/// values to a subset of pixels. For example:
///
/// ```cpp
///     dip::Image img = ...;
///     dip::Image mask = img < 0;
///     img.At( mask ) = -img.At( mask );
/// ```
///
/// In the code above, `img.At( mask )` returns a `%View` object. Manipulating this object yields a new image
/// with modified values. This image is then assigned into another `%View` object, changing the values of the
/// image `img`.
///
/// A `%dip::Image::View` behaves just like a `dip::Image`, except for the assignment operator. It can be indexed
/// using the `%At` method in most of the same ways as a `dip::Image`, yielding a new `%View` or a `dip::Image::Pixel`.
/// It implicitly casts to a `dip::Image`, so it can be used as arguments to image analysis functions. But methods
/// to `dip::Image` cannot be called on a `%View`, cast to `dip::Image` first!
///
/// Note that when an irregular view (i.e. generated by a coordinate array or a mask image) is cast to a `dip::Image`,
/// a 1D image is generated which does not share data with the original image. That is, the sample values are copied
/// to the new image. When the view is regular (i.e. generated by a range per dimension), then the view is cast to
/// an image that shares the data with the original image, and no data is copied.
///
/// See \ref indexing for more information.
///
/// Compound assignment operators are defined, but they currently do not do the computation in-place.
///
/// \see dip::Image::Pixel, dip::Image::Sample
class Image::View {
      friend class Image;
   public:

      // Public constructors, you can only make a new one by copy:
      View() = delete;                          // No default constructor
      View( View const& ) = default;            // Default copy constructor is OK
      View( View&& ) = default;                 // Default move constructor is OK

      // Assignment into `dip::Image::View`:
      View& operator=( View&& ) = delete;       // No move assignment
      View& operator=( View const& ) = delete;  // Copy assignment through cast to dip::Image
      // TODO: Assignment operators should call appropriate Copy function.

      /// \brief Assigning an image `source` to a view causes the pixels from `source` to be copied to the view.
      ///
      /// `source` must have the same number of tensor elements as the image, and be forged.
      /// For the case of a regular view, the view and `src` must have identical sizes, except that
      /// trailing singleton dimensions are ignored. For non-regular views, `src` must have the same
      /// number of pixels as the view, its shape is ignored.
      /// `source` pixel values are cast in the usual way to the type of `this`.
      View& operator=( Image const& source ) {
         Copy( source );
         return *this;
      }

      /// \brief Assigning a pixel to a view causes all pixels in the view to be set to the same value.
      View& operator=( Pixel const& pixel ) {
         Fill( pixel );
         return *this;
      }

      /// \brief Assigning a sample to a view causes all samples in the view to be set to the same value.
      View& operator=( Sample const& sample ) {
         Fill( sample );
         return *this;
      }

      /// \brief Copy the pixels from `src` to the view.
      ///
      /// `source` must have the same number of tensor elements as the image, and be forged.
      /// For the case of a regular view, the view and `src` must have identical sizes, except that
      /// trailing singleton dimensions are ignored. For non-regular views, `src` must have the same
      /// number of pixels as the view, its shape is ignored.
      /// `source` pixel values are cast in the usual way to the type of `this`.
      DIP_EXPORT void Copy( Image const& source );
      // TODO: Copy( View const& source ); -> would prevent one copy
      //    Note that currently View is cast to Image (copies pixels if view is irregular), and then the image
      //    pixels are copied to the view.

      /// \brief Creates a copy of the view as a new image. The output will not share data with the view.
      DIP_EXPORT Image Copy() const;

      /// \brief Sets all pixels in the view to the value `pixel`.
      ///
      /// `pixel` must have the same number of tensor elements as the image, or be a scalar.
      /// Its values will be clipped to the target range and/or truncated, as applicable.
      DIP_EXPORT void Fill( Pixel const& pixel );

      /// \brief Sets all samples in the view to the value `sample`.
      ///
      /// The value will be clipped to the target range and/or truncated, as applicable.
      DIP_EXPORT void Fill( Sample const& sample );

      /// \brief Extract a tensor element, `indices` must have one or two elements.
      View operator[]( UnsignedArray const& indices ) const {
         return operator[]( reference_.tensor_.Index( indices ));
      }

      /// \brief Extract a tensor element using linear indexing.
      View operator[]( dip::uint index ) const {
         return operator[]( Range( static_cast< dip::sint >( index )));
      }

      /// \brief Extract tensor elements using linear indexing.
      View operator[]( Range range ) const {
         View out( reference_, range );
         out.mask_ = mask_;
         out.offsets_ = offsets_;
         return out;
      }

      /// \brief Extracts the pixel at the given coordinates.
      DIP_EXPORT Pixel At( UnsignedArray const& coords ) const;

      /// \brief Same as above, but returns a type that implicitly casts to `T`.
      template< typename T >
      CastPixel< T > At( UnsignedArray const& coords ) const {
         return CastPixel< T >( At( coords ));
      }

      /// \brief Extracts the pixel at the given linear index (inefficient if image is not 1D!).
      DIP_EXPORT Pixel At( dip::uint index ) const;

      /// \brief Same as above, but returns a type that implicitly casts to `T`.
      template< typename T >
      CastPixel< T > At( dip::uint index ) const {
         return CastPixel< T >( At( index ));
      }

      /// \brief Extracts the pixel at the given coordinates from a 2D image.
      Pixel At( dip::uint x_index, dip::uint y_index ) const {
         return At( UnsignedArray( { x_index, y_index } ));
      }

      /// \brief Same as above, but returns a type that implicitly casts to `T`.
      template< typename T >
      CastPixel< T > At( dip::uint x_index, dip::uint y_index ) const {
         return CastPixel< T >( At( x_index, y_index ));
      }

      /// \brief Extracts the pixel at the given coordinates from a 3D image.
      Pixel At( dip::uint x_index, dip::uint y_index, dip::uint z_index ) const {
         return At( UnsignedArray( { x_index, y_index, z_index } ));
      }

      /// \brief Same as above, but returns a type that implicitly casts to `T`.
      template< typename T >
      CastPixel< T > At( dip::uint x_index, dip::uint y_index, dip::uint z_index ) const {
         return CastPixel< T >( At( x_index, y_index, z_index ));
      }

      /// \brief Extracts a subset of pixels from a 1D image.
      DIP_EXPORT View At( Range x_range ) const ;

      /// \brief Extracts a subset of pixels from a 2D image.
      View At( Range x_range, Range y_range ) const {
         DIP_THROW_IF( Dimensionality() != 2, E::ILLEGAL_DIMENSIONALITY );
         return At( RangeArray{ x_range, y_range } );
      }

      /// \brief Extracts a subset of pixels from a 3D image.
      View At( Range x_range, Range y_range, Range z_range ) const {
         DIP_THROW_IF( Dimensionality() != 3, E::ILLEGAL_DIMENSIONALITY );
         return At( RangeArray{ x_range, y_range, z_range } );
      }

      /// \brief Extracts a subset of pixels from an image.
      DIP_EXPORT View At( RangeArray const& ranges ) const;

      /// \brief Returns the dimensionality of the view. Non-regular views (created by indexing using a mask image
      /// or a coordinate array) are always 1D.
      dip::uint Dimensionality() const {
         if( mask_.IsForged() || !offsets_.empty() ) {
            return 1;
         }
         return reference_.Dimensionality();
      }

      /// \brief Returns the number of tensor elements of the view.
      dip::uint TensorElements() const {
         return reference_.TensorElements();
      }

      /// \brief View iterator, similar in functionality to `dip::GenericImageIterator`.
      class Iterator;

      /// \brief Returns an iterator to the first pixel in the view.
      Iterator begin() const;

      /// \brief Returns an iterator to one past the last pixel in the view.
      Iterator end() const;

   private:
      Image reference_;       // The image being indexed.
      Image mask_;            // A mask image to indicate which samples are indexed
      IntegerArray offsets_;  // A set of offsets to indicate which samples and/or pixels are indexed

      // If `mask_` is forged:
      //  - ignore `offsets_`
      //  - it is scalar
      //  - it is the same size as `reference_`
      //  - it indexes pixels
      //  - if it was a tensor image, we convert it and `reference_` to scalar, we index samples
      // Else if `offsets_` is not empty:
      //  - it indexes pixels
      // Else:
      //  - we're indexing a regular grid, which is already applied to `reference_`
      //
      // To index samples using the offset array, do `reference_.TensorToSpatial( 0 )`.
      //
      // If we index samples in a tensor image, the tensor dimension is seen as the first spatial dimension,
      //    and samples are taken in linear index order. Thus, two selected samples in the same pixel will
      //    appear together in the sample list.

      // Private constructors, only `dip::Image` can construct one of these:
      View( Image const& reference ) : reference_( reference ) {                    // a view over the full image
         DIP_THROW_IF( !reference_.IsForged(), E::IMAGE_NOT_FORGED );
      }
      View( Image&& reference ) : reference_( std::move( reference )) {             // a view over the full image
         DIP_THROW_IF( !reference_.IsForged(), E::IMAGE_NOT_FORGED );
      }
      DIP_EXPORT View( Image const& reference, Range range );                       // index tensor elements using range
      DIP_EXPORT View( Image const& reference, RangeArray ranges );                 // index pixels using regular grid
      DIP_EXPORT View( Image const& reference, Image const& mask );                 // index pixels or samples using mask
      DIP_EXPORT View( Image const& reference, UnsignedArray const& indices );      // index pixels using linear indices
      DIP_EXPORT View( Image const& reference, CoordinateArray const& coordinates );// index pixels using coordinates
};


class Image::View::Iterator {
   public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = Image::Pixel;
      using difference_type = dip::sint;
      using reference = value_type;
      using pointer = value_type*;

      /// Default constructor yields an invalid iterator that cannot be dereferenced, and is equivalent to an end iterator
      Iterator();
      /// To construct a useful iterator, provide a view
      explicit Iterator( View const& view );
      /// To construct a useful iterator, provide a view
      explicit Iterator( View&& view );
      // Define move constructor (not generated automatically because destructor is defined)
      Iterator( Iterator &&iterator );
      // Don't generate default destructor until GenericImageIterator is complete
      ~Iterator();

      /// Dereference
      value_type operator*() const {
         return value_type( Pointer(), view_.reference_.DataType(), view_.reference_.Tensor(), view_.reference_.TensorStride() );
      }
      /// Dereference
      value_type operator->() const {
         return operator*();
      }
      /// Index into tensor, `it[index]` is equal to `(*it)[index]`.
      Image::Sample operator[]( dip::uint index ) const {
         return operator*()[ index ];
      }

      /// Increment
      Iterator& operator++();

      /// Get an iterator over the tensor for the current pixel, `it.begin()` is equal to `(*it).begin()`.
      value_type::Iterator begin() const {
         return value_type::Iterator( Pointer(), view_.reference_.DataType(), view_.reference_.TensorStride() );
      }
      /// Get an end iterator over the tensor for the current pixel
      value_type::Iterator end() const {
         return value_type::Iterator( Pointer(), view_.reference_.DataType(), view_.reference_.TensorStride(), view_.reference_.TensorElements() );
      }

      /// Equality comparison, is equal if the two iterators have the same position.
      bool operator==( Iterator const& other ) const {
         return ( atEnd_ == other.atEnd_ ) && ( position_ == other.position_ );
      }
      /// Inequality comparison
      bool operator!=( Iterator const& other ) const {
         return !operator==( other );
      }

      /// Test to see if the iterator reached past the last pixel
      bool IsAtEnd() const { return atEnd_; }
      /// Test to see if the iterator is still pointing at a pixel
      explicit operator bool() const { return !atEnd_; }

      /// Return the current pointer
      void* Pointer() const;
      /// Return a pointer to the tensor element `index`
      void* Pointer( dip::uint index ) const;
      /// Return the current offset
      dip::sint Offset() const;

      /// Return the current position within the view (i.e. how many times we've advanced the iterator)
      dip::uint Position() const { return position_; }

      /// Reset the iterator to the first pixel in the image (as it was when first created)
      void Reset();

   private:
      View view_;                // A copy of the view object that we're iterating over.
      dip::uint position_ = 0;   // Counts how many elements we've advanced past.
      bool atEnd_ = false;       // true when we're done iterating
      std::unique_ptr< GenericImageIterator< dip::dfloat >> refIt_;           // Using pointers to incomplete type here
      std::unique_ptr< GenericJointImageIterator< 2, dip::dfloat >> maskIt_;  // Using pointers to incomplete type here

      void Initialize();

      // NOTE that we take a copy of the dip::Image::View object because we want to support syntax like:
      //    auto it = img.At( smth ).begin()
      //    do{ ... } while( ++it );
      // TODO: maybe we can have a different version of the iterator that takes a reference (or pointer).
};

/// \}


//
// dip::Image methods that depend on the definition of the classes in this file
//

inline Image::Image( Image::Pixel const& pixel ) : dataType_( pixel.DataType() ),
                                                   tensor_( pixel.Tensor() ),
                                                   tensorStride_( 1 ) {
   Forge();
   uint8 const* src = static_cast< uint8 const* >( pixel.Origin() );
   uint8* dest = static_cast< uint8* >( origin_ );
   dip::uint sz = dataType_.SizeOf();
   dip::sint srcStep = pixel.TensorStride() * static_cast< dip::sint >( sz );
   dip::sint destStep = tensorStride_ * static_cast< dip::sint >( sz );
   for( dip::uint ii = 0; ii < tensor_.Elements(); ++ii ) {
      std::memcpy( dest, src, sz );
      src += srcStep;
      dest += destStep;
   }
}

inline Image::Image( Image::Pixel const& pixel, dip::DataType dt ) : dataType_( dt ),
                                                                     tensor_( pixel.Tensor() ),
                                                                     tensorStride_( 1 ) {
   Forge();
   uint8 const* src = static_cast< uint8 const* >( pixel.Origin() );
   uint8* dest = static_cast< uint8* >( origin_ );
   dip::sint srcStep = pixel.TensorStride() * static_cast< dip::sint >( pixel.DataType().SizeOf() );
   dip::sint destStep = tensorStride_ * static_cast< dip::sint >( dataType_.SizeOf() );
   for( dip::uint ii = 0; ii < tensor_.Elements(); ++ii ) {
      detail::CastSample( pixel.DataType(), src, dataType_, dest );
      src += srcStep;
      dest += destStep;
   }
}

inline Image::Image( Image::Sample const& sample ) : dataType_( sample.DataType() ) {
   Forge();
   uint8 const* src = static_cast< uint8 const* >( sample.Origin() );
   dip::uint sz = dataType_.SizeOf();
   std::memcpy( origin_, src, sz );
}

inline Image::Image( Image::Sample const& sample, dip::DataType dt ) : dataType_( dt ) {
   Forge();
   detail::CastSample( sample.DataType(), sample.Origin(), dataType_, origin_ );
}

inline Image::Image( Image::View const& view ) {
   if( view.mask_.IsForged() ) {
      CopyFrom( view.reference_, *this, view.mask_ );
   } else if( !view.offsets_.empty() ) {
      CopyFrom( view.reference_, *this, view.offsets_ );
   } else {
      *this = view.reference_;
   }
}

inline Image::Image( Image::View&& view ) {
   if( view.mask_.IsForged() ) {
      CopyFrom( view.reference_, *this, view.mask_ );
   } else if( !view.offsets_.empty() ) {
      CopyFrom( view.reference_, *this, view.offsets_ );
   } else {
      this->move( std::move( view.reference_ ));
   }
}

inline Image::Pixel Image::At( UnsignedArray const& coords ) const {
   DIP_STACK_TRACE_THIS( return Pixel( Pointer( coords ), dataType_, tensor_, tensorStride_ ));
}

template< typename T >
Image::CastPixel< T > Image::At( UnsignedArray const& coords ) const {
   return CastPixel< T >( At( coords ));
}

template< typename T >
Image::CastPixel< T > Image::At( dip::uint index ) const {
   return CastPixel< T >( At( index ));
}

template< typename T >
Image::CastPixel< T > Image::At( dip::uint x_index, dip::uint y_index ) const {
   return CastPixel< T >( At( x_index, y_index ));
}

template< typename T >
Image::CastPixel< T > Image::At( dip::uint x_index, dip::uint y_index, dip::uint z_index ) const {
   return CastPixel< T >( At( x_index, y_index, z_index ));
}

inline Image::View Image::operator[]( UnsignedArray const& indices ) const {
   return operator[]( static_cast< dip::sint >( tensor_.Index( indices )));
}

template< typename T, typename >
inline Image::View Image::operator[]( T index ) const {
   return operator[]( Range( static_cast< dip::sint >( index )));
}

inline Image::View Image::operator[]( Range const& range ) const {
   DIP_STACK_TRACE_THIS( return Image::View( *this, range ));
}

inline Image::View Image::At( Range const& x_range ) const {
   DIP_THROW_IF( Dimensionality() != 1, E::ILLEGAL_DIMENSIONALITY );
   return At( RangeArray{ x_range } );
}

inline Image::View Image::At( Range const& x_range, Range const& y_range ) const {
   DIP_THROW_IF( Dimensionality() != 2, E::ILLEGAL_DIMENSIONALITY );
   return At( RangeArray{ x_range, y_range } );
}

inline Image::View Image::At( Range const& x_range, Range const& y_range, Range const& z_range ) const {
   DIP_THROW_IF( Dimensionality() != 3, E::ILLEGAL_DIMENSIONALITY );
   return At( RangeArray{ x_range, y_range, z_range } );
}

inline Image::View Image::At( RangeArray const& ranges ) const {
   DIP_STACK_TRACE_THIS( return Image::View( *this, ranges ));
}

inline Image::View Image::At( Image const& mask ) const {
   DIP_STACK_TRACE_THIS( return Image::View( *this, mask ));
}

inline Image::View Image::At( CoordinateArray const& coordinates ) const {
   DIP_STACK_TRACE_THIS( return Image::View( *this, coordinates ));
}

inline Image::View Image::AtIndices( UnsignedArray const& indices ) const {
   DIP_STACK_TRACE_THIS( return Image::View( *this, indices ));
}

inline Image::Image( FloatArray const& values, dip::DataType dt ) : Image( Pixel( values, dt )) {}

inline Image::operator FloatArray() const { return static_cast< Pixel >( *this ); }

} // namespace dip

#endif // DIP_IMAGE_VIEWS_H
