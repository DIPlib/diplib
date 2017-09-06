/*
 * DIPlib 3.0
 * This file contains an interface to a DFT function
 *
 * (c)2017, Cris Luengo.
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

#include <vector>
#include <complex>
#include <limits>
#include "dip_export.h"

/// \file
/// \brief Declares an interface to a DFT function.


namespace dip {


/// \brief An object that encapsulates the Discrete Fourier Transform (DFT).
///
/// Usage:
/// ```cpp
///     DFT dft( size, inverse );               // creates the object with all the data ready to start running DFTs.
///     std::vector< std::complex< T >> buf( opts.BufferSize() ); // creates a buffer
///     dft.Apply( in, out, buf.data() );                         // runs a DFT, repeat as necessary
///     dft.Initialize( size2, inverse );                         // changes the options for the new size / direction
///     buf.resize( opts.BufferSize() );                          // resizes the buffer
///     dft.Apply( in, out, buf.data() );                         // runs a different DFT, repeat as necessary
/// ```
///
/// Note that this code uses `int` for sizes, rather than `dip::uint`. `maximumDFTSize` is the largest length
/// of the transform.
///
/// The template can be instantiated for `T = float` or `T = double`. Linker errors will result for other types.
template< typename T >
class DFT {
   public:

      /// \brief A default-initialized `%DFT` object is useless. Call `Initialize` to make it useful.
      DFT() {}

      /// \brief Construct a `%DFT` object by specifying the size and direction of the transform.
      /// Note that this is not a trivial operation.
      DFT( int size, bool inverse ) {
         Initialize( size, inverse );
      }

      /// \brief Re-configure a `%DFT` object to the given transform size and direction.
      /// Note that this is not a trivial operation.
      DIP_EXPORT void Initialize( int size, bool inverse );

      /// \brief Apply the transform that the `%DFT` object is configured for.
      ///
      /// `source` and `destination` are pointers to contiguous buffers with `TransformSize` elements.
      /// This is the value of the `size` parameter of the constructor or `Initialize`. `buffer` is a pointer
      /// to a contiguous buffer used for intermediate data. It should have `BufferSize` elements.
      ///
      /// `scale` is a real scalar that the output values are multiplied by. It is typically set to `1/size` for
      /// the inverse transform, and 1 for the forward transform.
      DIP_EXPORT void Apply(
            const std::complex< T >* source,
            std::complex< T >* destination,
            std::complex< T >* buffer,
            T scale
      ) const;

      /// \brief Returns true if this represents an inverse transform, false for a forward transform.
      bool IsInverse() const { return inverse_; }

      /// \brief Returns the size that the transform is configured for.
      int TransformSize() const { return nfft_; }

      /// \brief Returns the size of the buffer expected by `Apply`.
      int BufferSize() const { return sz_; }

   private:
      int nfft_ = 0;
      bool inverse_ = false;
      std::vector< int > factors_;
      std::vector< int > itab_;
      std::vector< std::complex< T >> wave_;
      int sz_ = 0; // Size of the buffer to be passed to DFT.
};

/// \brief Returns a size equal or larger to `size0` that is efficient for our DFT implementation.
///
/// Returns 0 if `size0` is too large for our DFT implementation.
///
/// Prefer to use `dip::OptimalFourierTransformSize` in your applications, it will throw an error if
/// the transform size is too large.
DIP_EXPORT size_t GetOptimalDFTSize( size_t size0 );

/// \brief The largest size supported by the DFT.
constexpr size_t maximumDFTSize = static_cast< size_t >( std::numeric_limits< int >::max() );

} // namespace dip

#endif // DIP_DFT_H
