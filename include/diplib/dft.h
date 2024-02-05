/*
 * (c)2017-2022, Cris Luengo.
 *
 * Encapsulates code Taken from OpenCV 3.1: (c)2000, Intel Corporation.
 * (see src/transform/opencv_dxt.cpp)
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

#ifndef DIP_DFT_H
#define DIP_DFT_H

#include <complex>

#include "diplib/library/export.h"
#include "diplib/library/types.h"

/// \file
/// \brief Declares an interface to a DFT function.
/// See \ref transform.


namespace dip {


/// \addtogroup transform


namespace Option {

/// \brief Determines working mode for \ref DFT and \ref RDFT.
///
/// Implicitly casts to \ref dip::Option::DFTOptions. Combine constants together with the `+` operator.
enum class DIP_NO_EXPORT DFTOption : uint8 {
   InPlace,      ///< Work in place, the input and output buffers are the same.
   TrashInput,   ///< Allowed to trash the input buffer, we don't need to preserve it.
   Aligned       ///< Both buffers are aligned to 16-byte boundaries.
};

/// \class dip::Option::DFTOptions
/// \brief Determines working mode for \ref DFT and \ref RDFT. Combines multiple \ref DFTOption values,.
DIP_DECLARE_OPTIONS( DFTOption, DFTOptions )

} // namespace Option


/// \brief An object that encapsulates the Discrete Fourier Transform (DFT).
///
/// Usage:
///
/// ```cpp
/// DFT dft( size, inverse );          // creates the object with all the data ready to start running DFTs.
/// dft.Apply( in, out );              // computes a DFT, repeat as necessary
/// dft.Initialize( size2, inverse );  // changes the options for the new size / direction
/// dft.Apply( in, out );              // computes a different DFT, repeat as necessary
/// ```
///
/// The template can be instantiated for `T = float` or `T = double`. Linker errors will result for other types.
///
/// !!! attention
///     The object should not be created, copied or destroyed in multiple threads at the same time.
///
/// The DFT is computed using either PocketFFT or FFTW, depending on compile-time configuration (see the
/// `DIP_ENABLE_FFTW` CMake configuration option).
///
/// When using FFTW, \ref maximumDFTSize is the largest length of the transform. PocketFFT does not have this limit.
template< typename T >
class DFT {
   public:

      /// \brief A default-initialized `DFT` object is useless. Call `Initialize` to make it useful.
      DFT() = default;

      /// \brief Construct a `DFT` object, see \ref Initialize for the meaning of the parameters.
      /// Note that this is not a trivial operation. Not thread safe.
      DFT( dip::uint size, bool inverse, Option::DFTOptions options = {} ) {
         Initialize( size, inverse, options );
      }

      // Destructor.
      ~DFT() {
         Destroy();
      }

      // Copying is creating a new plan -- allow plan creation code to buffer plans
      DFT( DFT const& other ) {
         Initialize( other.nfft_, other.inverse_, other.options_ );
      }
      DFT& operator=( DFT const& other ) {
         if ( this != &other ) {
            Destroy();
            Initialize( other.nfft_, other.inverse_, other.options_ );
         }
         return *this;
      }

      // Allow moving
      DFT( DFT&& other ) noexcept
            : plan_( other.plan_ ), nfft_( other.nfft_ ), inverse_( other.inverse_ ), options_( other.options_ ) {
         other.plan_ = nullptr;
      }
      DFT& operator=( DFT&& other ) noexcept {
         using std::swap;
         swap( plan_, other.plan_ );
         swap( nfft_, other.nfft_ );
         swap( inverse_, other.inverse_ );
         swap( options_, other.options_ );
         return *this;
      }

      /// \brief Re-configure a `DFT` object to the given transform size and direction.
      ///
      /// `size` is the size of the transform. The two pointers passed to \ref Apply are expected to point at
      /// buffers with this length. If `inverse` is `true`, an inverse transform will be computed.
      ///
      /// `options` determines some properties for the algorithm that will compute the DFT.
      ///  - \ref Option::DFTOption::InPlace means the input and output pointers passed to \ref Apply must be the same.
      ///  - \ref Option::DFTOption::TrashInput means that the algorithm is free to overwrite the input array.
      ///    Ignored when working in place.
      ///  - \ref Option::DFTOption::Aligned means that the input and output buffers are aligned to 16-bit boundaries,
      ///    which can significantly improve the speed of the algorithm.
      ///
      /// When using PocketFFT, all these options are ignored.
      ///
      /// Note that this is not a trivial operation, planning an FFT costs time.
      ///
      /// This operation is not thread safe.
      DIP_EXPORT void Initialize( dip::uint size, bool inverse, Option::DFTOptions options = {} );

      [[ deprecated( "Use the dip::Option::DFTOptions flags." ) ]]
      DIP_EXPORT void Initialize( dip::uint size, bool inverse, bool inplace ) {
         if( inplace ) {
            Initialize( size, inverse, Option::DFTOption::InPlace );
         } else {
            Initialize( size, inverse );
         }
      }

      /// \brief Apply the transform that the `DFT` object is configured for.
      ///
      /// `source` and `destination` are pointers to contiguous buffers with \ref TransformSize elements.
      /// This is the value of the `size` parameter of the constructor or \ref Initialize. These two pointers
      /// can point to the same address for in-place operation; otherwise they must point to non-overlapping
      /// regions of memory. When using FFTW, the `inplace` parameter to the constructor or \ref Initialize
      /// must be `true` if the two pointers here are the same, or `false` if they are different.
      ///
      /// The input array is not marked `const`. If \ref Option::DFTOption::TrashInput` is given when planning,
      /// the input array can be overwritten with intermediate data, but otherwise will be left intact.
      ///
      /// `scale` is a real scalar that the output values are multiplied by. It is typically set to `1/size` for
      /// the inverse transform, and 1 for the forward transform.
      DIP_EXPORT void Apply(
            std::complex< T >* source,
            std::complex< T >* destination,
            T scale
      ) const;

      [[ deprecated( "A buffer is no longer necessary." ) ]]
      void Apply(
            std::complex< T >* source,
            std::complex< T >* destination,
            std::complex< T >* buffer,
            T scale
      ) const {
         (void) buffer;
         Apply( source, destination, scale );
      }

      /// \brief Returns `true` if this represents an inverse transform, `false` for a forward transform.
      bool IsInverse() const { return inverse_; }

      /// \brief Returns whether the transform is configured to work in place or not. Not meaningful when using PocketFFT.
      bool IsInplace() const { return options_.Contains( Option::DFTOption::InPlace ); }

      /// \brief Returns whether the transform is configured to work on aligned buffers or not. Not meaningful when using PocketFFT.
      bool IsAligned() const { return options_.Contains( Option::DFTOption::Aligned ); }

      /// \brief Returns the size that the transform is configured for.
      dip::uint TransformSize() const { return nfft_; }

      /// \brief Returns the size of the buffer expected by `Apply`.
      [[ deprecated( "A buffer is no longer necessary." ) ]]
      dip::uint BufferSize() const { return 0; }

   private:
      void* plan_ = nullptr; // Using void* to avoid referencing types in external libraries that the user might not want to include directly.
      dip::uint nfft_ = 0;
      bool inverse_ = false;
      Option::DFTOptions options_ = {};

      /// \brief Frees memory
      DIP_EXPORT void Destroy();

};


/// \brief An object that encapsulates the real-valued Discrete Fourier Transform (DFT).
///
/// Usage:
///
/// ```cpp
/// RDFT rdft( size, inverse );        // creates the object with all the data ready to start running DFTs.
/// dft.Apply( in, out );              // computes a DFT, repeat as necessary
/// dft.Initialize( size2, inverse );  // changes the options for the new size / direction
/// dft.Apply( in, out );              // computes a different DFT, repeat as necessary
/// ```
///
/// Here, `in` is a real-valued array with `size` elements , and `out` is a complex-valued array with
/// `size/2+1` elements, containing only the non-redundant values of the transform; the remaining values
/// can be trivially computed if needed using `std::conj`. For the inverse transform, the output is the real-valued
/// array. Both arrays are passed into \ref Apply using a real-valued pointer.
///
/// The template can be instantiated for `T = float` or `T = double`. Linker errors will result for other types.
///
/// !!! attention
///     The object should not be created, copied or destroyed in multiple threads at the same time.
///
/// The DFT is computed using either PocketFFT or FFTW, depending on compile-time configuration (see the
/// `DIP_ENABLE_FFTW` CMake configuration option).
///
/// When using FFTW, \ref maximumDFTSize is the largest length of the transform. PocketFFT does not have this limit.
template< typename T >
class RDFT {
   public:

      /// \brief A default-initialized `DFT` object is useless. Call `Initialize` to make it useful.
      RDFT() = default;

      /// \brief Construct a `DFT` object, see \ref Initialize for the meaning of the parameters.
      /// Note that this is not a trivial operation. Not thread safe.
      RDFT( dip::uint size, bool inverse, Option::DFTOptions options = {} ) {
         Initialize( size, inverse, options );
      }

      // Destructor.
      ~RDFT() {
         Destroy();
      }

      // Copying is creating a new plan -- allow plan creation code to buffer plans
      RDFT( RDFT const& other ) {
         Initialize( other.nfft_, other.inverse_, other.options_ );
      }
      RDFT& operator=( RDFT const& other ) {
         if ( this != &other ) {
            Destroy();
            Initialize( other.nfft_, other.inverse_, other.options_ );
         }
         return *this;
      }

      // Allow moving
      RDFT( RDFT&& other ) noexcept
            : plan_( other.plan_ ), nfft_( other.nfft_ ), inverse_( other.inverse_ ), options_( other.options_ ) {
         other.plan_ = nullptr;
      }
      RDFT& operator=( RDFT&& other ) noexcept {
         using std::swap;
         swap( plan_, other.plan_ );
         swap( nfft_, other.nfft_ );
         swap( inverse_, other.inverse_ );
         swap( options_, other.options_ );
         return *this;
      }

      /// \brief Re-configure a `RDFT` object to the given transform size and direction.
      ///
      /// `size` is the size of the transform. The real-valued pointer passed to \ref Apply is expected to point at
      /// a buffer with this length. If `inverse` is `true`, an inverse transform will be computed (complex to real).
      /// The complex buffer has a size of `size/2+1`.
      ///
      /// `options` determines some properties for the algorithm that will compute the DFT.
      ///  - \ref Option::DFTOption::InPlace means the input and output pointers passed to \ref Apply must be the same.
      ///    Do note that the complex array has one or two floats more than the real array, the buffer must be large
      ///    enough.
      ///  - \ref Option::DFTOption::TrashInput means that the algorithm is free to overwrite the input array.
      ///    Ignored when working in place.
      ///  - \ref Option::DFTOption::Aligned means that the input and output buffers are aligned to 16-bit boundaries,
      ///    which can significantly improve the speed of the algorithm.
      ///
      /// When using PocketFFT, all these options are ignored.
      ///
      /// Note that this is not a trivial operation, planning an FFT costs time.
      ///
      /// This operation is not thread safe.
      DIP_EXPORT void Initialize( dip::uint size, bool inverse, Option::DFTOptions options = {} );

      /// \brief Apply the transform that the `RDFT` object is configured for.
      ///
      /// `source` and `destination` are pointers to contiguous buffers.
      /// If configured as a forward transform, `source` is the real-valued array with \ref TransformSize elements,
      /// and `destination` is the complex-valued array with `TransformSize() / 2 + 1` elements (presented as a
      /// pointer to a real-valued array with twice the number of elements). If configured as an inverse transform,
      /// the two descriptions are swapped. These two pointers can point to the same address for in-place operation;
      /// otherwise they must point to non-overlapping regions of memory. When using FFTW, the `inplace` parameter
      /// to the constructor or \ref Initialize must be `true` if the two pointers here are the same, or `false`
      /// if they are different.
      ///
      /// In the above description, `TransformSize()` is the value of the `size` parameter of the constructor or
      /// \ref Initialize.
      ///
      /// The input array is not marked `const`. If \ref Option::DFTOption::TrashInput` is given when planning,
      /// the input array can be overwritten with intermediate data, but otherwise will be left intact.
      ///
      /// `scale` is a real scalar that the output values are multiplied by. It is typically set to `1/size` for
      /// the inverse transform, and 1 for the forward transform.
      DIP_EXPORT void Apply(
            T* source,
            T* destination,
            T scale
      ) const;

      /// \brief Returns `true` if this represents an inverse transform, `false` for a forward transform.
      bool IsInverse() const { return inverse_; }

      /// \brief Returns whether the transform is configured to work in place or not. Not meaningful when using PocketFFT.
      bool IsInplace() const { return options_.Contains( Option::DFTOption::InPlace ); }

      /// \brief Returns whether the transform is configured to work on aligned buffers or not. Not meaningful when using PocketFFT.
      bool IsAligned() const { return options_.Contains( Option::DFTOption::Aligned ); }

      /// \brief Returns the size that the transform is configured for.
      dip::uint TransformSize() const { return nfft_; }

   private:
      void* plan_ = nullptr;
      dip::uint nfft_ = 0;
      bool inverse_ = false;
      Option::DFTOptions options_ = {};

      /// \brief Frees memory
      DIP_EXPORT void Destroy();

};


/// \brief Returns a size equal or larger to `size0` that is efficient for the DFT implementation.
///
/// Set `larger` to false to return a size equal or smaller instead.
///
/// Returns 0 if `size0` is too large for the DFT implementation.
///
/// Prefer to use \ref dip::OptimalFourierTransformSize in your applications, it will throw an error if
/// the transform size is too large.
DIP_EXPORT dip::uint GetOptimalDFTSize( dip::uint size0, bool larger = true );


/// \brief The largest size supported by \ref DFT and \ref FourierTransform. Is equal to 2^31^-1 when using FFTW,
/// or 2^64^-1 when using PocketFFT.
DIP_EXPORT extern dip::uint const maximumDFTSize;

/// \brief Is `true` if `dip::DFT` and `dip::RDFT` use the FFTW library, or false if they use PocketFFT.
DIP_EXPORT extern bool const usingFFTW;


/// \endgroup

} // namespace dip

#endif // DIP_DFT_H
