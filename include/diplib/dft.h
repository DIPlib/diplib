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
#include <limits>

#include "diplib/library/export.h"

/// \file
/// \brief Declares an interface to a DFT function.
/// See \ref transform.


namespace dip {


/// \addtogroup transform


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
      DFT( std::size_t size, bool inverse, bool inplace = false ) {
         Initialize( size, inverse, inplace );
      }

      // Destructor.
      ~DFT() {
         Destroy();
      }

      // Copying is creating a new plan -- allow plan creation code to buffer plans
      DFT( DFT const& other ) {
         Initialize( other.nfft_, other.inverse_, other.inplace_ );
      }
      DFT& operator=( DFT const& other ) {
         Destroy();
         Initialize( other.nfft_, other.inverse_, other.inplace_ );
         return *this;
      }

      // Allow moving
      DFT( DFT&& other ) noexcept
            : plan_( other.plan_ ), nfft_( other.nfft_ ), inverse_( other.inverse_ ), inplace_( other.inplace_ ) {
         other.plan_ = nullptr;
      }
      DFT& operator=( DFT&& other ) noexcept {
         Destroy();
         plan_ = other.plan_;
         other.plan_ = nullptr;
         nfft_ = other.nfft_;
         inverse_ = other.inverse_;
         inplace_ = other.inplace_;
         return *this;
      }

      /// \brief Re-configure a `DFT` object to the given transform size and direction.
      ///
      /// `size` is the size of the transform. The two pointers passed to \ref Apply are expected to point at
      /// buffers with this length. If `inverse` is `true`, an inverse transform will be computed. If `inplace`
      /// is `true`, the two pointers passed to \ref Apply must be the same, otherwise they must be different.
      /// However, when using PocketFFT this value is ignored.
      ///
      /// Note that this is not a trivial operation.
      ///
      /// This operation is not thread safe.
      DIP_EXPORT void Initialize( std::size_t size, bool inverse, bool inplace = false );

      /// \brief Apply the transform that the `DFT` object is configured for.
      ///
      /// `source` and `destination` are pointers to contiguous buffers with \ref TransformSize elements.
      /// This is the value of the `size` parameter of the constructor or \ref Initialize. These two pointers
      /// can point to the same address for in-place operation; otherwise they must point to non-overlapping
      /// regions of memory. When using FFTW, the `inplace` parameter to the constructor or \ref Initialize
      /// must be `true` if the two pointers here are the same, or `false` if they are different.
      ///
      /// `scale` is a real scalar that the output values are multiplied by. It is typically set to `1/size` for
      /// the inverse transform, and 1 for the forward transform.
      DIP_EXPORT void Apply(
            const std::complex< T >* source,
            std::complex< T >* destination,
            T scale
      ) const;

      [[ deprecated( "A buffer is no longer necessary." ) ]]
      void Apply(
            const std::complex< T >* source,
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
      bool IsInplace() const { return inplace_; }

      /// \brief Returns the size that the transform is configured for.
      std::size_t TransformSize() const { return nfft_; }

      /// \brief Returns the size of the buffer expected by `Apply`.
      [[ deprecated( "A buffer is no longer necessary." ) ]]
      std::size_t BufferSize() const { return 0; }

   private:
      void* plan_ = nullptr;
      std::size_t nfft_ = 0;
      bool inverse_ = false;
      bool inplace_ = false;

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
      RDFT( std::size_t size, bool inverse ) {
         Initialize( size, inverse );
      }

      // Destructor.
      ~RDFT() {
         Destroy();
      }

      // Copying is creating a new plan -- allow plan creation code to buffer plans
      RDFT( RDFT const& other ) {
         Initialize( other.nfft_, other.inverse_ );
      }
      RDFT& operator=( RDFT const& other ) {
         Destroy();
         Initialize( other.nfft_, other.inverse_ );
         return *this;
      }

      // Allow moving
      RDFT( RDFT&& other ) noexcept
            : plan_( other.plan_ ), nfft_( other.nfft_ ), inverse_( other.inverse_ ) {
         other.plan_ = nullptr;
      }
      RDFT& operator=( RDFT&& other ) noexcept {
         Destroy();
         plan_ = other.plan_;
         other.plan_ = nullptr;
         nfft_ = other.nfft_;
         inverse_ = other.inverse_;
         return *this;
      }

      /// \brief Re-configure a `RDFT` object to the given transform size and direction.
      ///
      /// `size` is the size of the transform. The real-valued pointer passed to \ref Apply is expected to point at
      /// a buffer with this length. If `inverse` is `true`, an inverse transform will be computed (complex to real).
      ///
      /// Note that this is not a trivial operation.
      ///
      /// This operation is not thread safe.
      DIP_EXPORT void Initialize( std::size_t size, bool inverse );

      /// \brief Apply the transform that the `RDFT` object is configured for.
      ///
      /// `source` and `destination` are pointers to contiguous buffers.
      /// If configured as a forward transform, `source` is the real-valued array with \ref TransformSize elements,
      /// and `destination` is the complex-valued array with `TransformSize() / 2 + 1` elements (presented as a
      /// pointer to a real-valued array with twice the number of elements). If configured as an inverse transform,
      /// the two descriptions are swapped. The two arrays must not overlap.
      ///
      /// In the above description, `TransformSize()` is the value of the `size` parameter of the constructor or
      /// \ref Initialize.
      ///
      /// `complex` has `TransformSize() / 2 + 1` elements. If configured as a forward transform, `real` is
      /// the input and `complex` is the output. If configured for an inverse transform, `real` is
      /// the output and `complex` is the input. The two buffers should not overlap. Though neither pointer
      /// is marked `const`, the input array will not be modified.
      ///
      /// `scale` is a real scalar that the output values are multiplied by. It is typically set to `1/size` for
      /// the inverse transform, and 1 for the forward transform.
      DIP_EXPORT void Apply(
            T const* source,
            T* destination,
            T scale
      ) const;

      /// \brief Returns `true` if this represents an inverse transform, `false` for a forward transform.
      bool IsInverse() const { return inverse_; }

      /// \brief Returns the size that the transform is configured for.
      std::size_t TransformSize() const { return nfft_; }

   private:
      void* plan_ = nullptr;
      std::size_t nfft_ = 0;
      bool inverse_ = false;

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
DIP_EXPORT std::size_t GetOptimalDFTSize( std::size_t size0, bool larger = true );

/// \brief The largest size supported by \ref DFT and \ref FourierTransform. Is equal to 2^31^-1 when using FFTW,
/// or 2^64-1 when using PocketFFT.
extern std::size_t const maximumDFTSize;


/// \endgroup

} // namespace dip

#endif // DIP_DFT_H
