/*
 * DIPlib 3.0
 * This file contains the declaration of functions to help test your code
 *
 * (c)2017, Cris Luengo.
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

#ifndef DIP_TESTING_H
#define DIP_TESTING_H

#include <chrono>
#include <ctime>
#include <iomanip>

#include "diplib.h"
#include "diplib/statistics.h"
#include "diplib/iterators.h"


/// \file
/// \brief Declares functions to help test your code.


namespace dip {

namespace Option {
/// \brief How to compare images in `dip::testing::CompareImages`.
enum class DIP_NO_EXPORT CompareImagesMode {
      APPROX,  ///< Compare the sample values (and image sizes), to match within
      EXACT,   ///< Compare only the sample values (and image sizes)
      FULL     ///< Compare for identical sample values as well as tensor shape, color space, and pixel size
};
}

/// \brief Tools for testing and debugging.
namespace testing {

namespace detail {

// For integral types (uint, sint) -- dip::sint can hold the value of any integer-valued pixel.
template< typename T, typename std::enable_if< std::is_integral< T >::value, int >::type = 0 >
dip::sint Round( T v, int /*digits*/) {
   return v;
}

// For type `bin`
inline dip::sint Round( bin v, int /*digits*/) {
   return v;
}

// For floating-point types
template< typename T, typename std::enable_if< !std::is_integral< T >::value, int >::type = 0 >
T Round( T v, int digits ) {
   int intDigits = std::abs( v ) < 10.0 ? 1 : static_cast< int >( std::floor( std::log10( std::abs( v ))));
   if( v < 0 ) {
      ++intDigits; // we need space for the minus sign also.
   }
   if( intDigits < digits ) {
      T multiplier = static_cast< T >( pow10( digits - intDigits - 1 ));
      return std::round( v * multiplier ) / multiplier;
   } else {
      // We've got more digits to the left of the decimal dot than can fit in the display, this will not look pretty...
      return std::round( v );
   }
}

// For complex types
template< typename T >
std::complex< T > Round( std::complex< T > v, int digits ) {
   return { Round( v.real(), digits ), Round( v.imag(), digits ) };
}

} // namespace detail

/// \defgroup testing Testing and debugging
/// \brief Tools for testing and debugging.
/// \ingroup infrastructure
/// \{

/// \brief Outputs pixel values of a small image to `stdout`.
///
/// If the image is a tensor image, shows only the first tensor component.
///
/// The first template parameter must match the image's data type.
///
/// An optional second template parameter determines the precision for displaying floating-point values.
template< typename TPI, int DIGITS = 4 >
void PrintPixelValues( Image const& img ) {
   DIP_THROW_IF( img.DataType() != DataType( TPI() ), "Wrong template parameter to PrintPixelValues() used" );
   dip::uint lineLength = img.Size( 0 );
   std::cout << "Image of size " << lineLength << " x " << img.Sizes().product() / lineLength << ":\n";
   dip::ImageIterator< TPI > it( img, 0 );
   do {
      auto lit = it.GetLineIterator();
      std::cout << "[i";
      for( dip::uint ii = 1; ii < img.Dimensionality(); ++ii ) {
         std::cout << "," << std::setw( 2 ) << it.Coordinates()[ ii ];
      }
      std::cout << "] : ";
      std::cout << std::setw( DIGITS + 1 ) << detail::Round( *lit, DIGITS );
      while( ++lit ) {
         std::cout << ", " << std::setw( DIGITS + 1 ) << detail::Round( *lit, DIGITS );
      }
      std::cout << std::endl;
   } while( ++it );
}


/// \brief Compares two images. Returns test result and prints to `stdout` the reason of failure
/// if the test fails.
///
/// Returns `true` only if they have the same sizes, number of tensor elements, and sample values.
/// If the result is `false`, it prints a message to `stdout` that starts with
/// `[dip::testing::CompareImages]` and gives the reason that the test failed.
///
/// If `mode` is `dip::Option::CompareImagesMode::APPROX`, the sample values must all be within
/// `epsilon`, which defaults to 1e-6. For this mode of operation there is an overloaded function
/// that takes `epsilon` as the 3rd argument (i.e. you can skip the `mode` parameter):
/// ```cpp
///     dip::CompareImages( img1, img2 );       // samples must be identical
///     dip::CompareImages( img1, img2, 1e-3 ); // samples must be within 1e-3 of each other
/// ```
///
/// If `mode` is `dip::Option::CompareImagesMode::FULL`, the sample values must match exactly, and
/// non-data properties (tensor shape, color space and pixel size) must also match exactly.
///
/// This function does not compare strides.
inline bool CompareImages(
      Image const& img1,
      Image const& img2,
      Option::CompareImagesMode mode = Option::CompareImagesMode::EXACT,
      dfloat epsilon = 1e-6
) {
   if( &img1 == &img2 ) { return true; }
   if( img1.TensorElements() != img2.TensorElements() ) {
      std::cout << "[dip::testing::CompareImages] Number of tensor elements doesn't match\n";
      return false;
   }
   if( img1.Sizes() != img2.Sizes() ) {
      std::cout << "[dip::testing::CompareImages] Image sizes don't match\n";
      return false;
   }
   if( mode == Option::CompareImagesMode::APPROX ) {
      dfloat mae = MaximumAbsoluteError( img1, img2 );
      if( mae > epsilon ) {
         std::cout << "[dip::testing::CompareImages] Maximum absolute error = " << mae << " > " << epsilon << std::endl;
      }
      return mae <= epsilon;
   }
   if( img1.TensorElements() > 1 ) {
      Image tmp1 = img1.QuickCopy(); tmp1.TensorToSpatial();
      Image tmp2 = img2.QuickCopy(); tmp2.TensorToSpatial();
      if( !All( tmp1 == tmp2 ).As< bool >()) {
         std::cout << "[dip::testing::CompareImages] At least one sample value differs\n";
         return false;
      }
   } else {
      if( !All( img1 == img2 ).As< bool >()) {
         std::cout << "[dip::testing::CompareImages] At least one sample value differs\n";
         return false;
      }
   }
   if( mode == Option::CompareImagesMode::FULL ) {
      if( img1.TensorShape() != img2.TensorShape() ) {
         std::cout << "[dip::testing::CompareImages] Tensor shape doesn't match\n";
         return false;
      }
      if( img1.ColorSpace() != img2.ColorSpace() ) {
         std::cout << "[dip::testing::CompareImages] Color space doesn't match\n";
         return false;
      }
      if( img1.PixelSize() != img2.PixelSize() ) {
         std::cout << "[dip::testing::CompareImages] Pixel size doesn't match\n";
         return false;
      }
   }
   return true;
}
inline bool CompareImages(
      Image const& img1,
      Image const& img2,
      dfloat epsilon
) {
   return CompareImages( img1, img2, Option::CompareImagesMode::APPROX, epsilon );
}

/// \brief A timer object to help time algorithm execution.
///
/// The methods `GetCpu` and `GetWall` return the CPU and wall time, respectively, in seconds that passed in
/// between object creation and the last call to `Stop`. `Stop` does not actually stop the timer, it just
/// records the time it was last called. `Reset` resets the timer, as if it had just been created.
///
/// ```cpp
///     dip::Timer timer;
///     // do some computation
///     timer.Stop();
///     std::cout << "Computation 1: Wall time = " << timer.GetWall() << " s. CPU time = " << timer.GetCpu() << " s.\n";
///     timer.Reset();
///     // do some other computation
///     timer.Stop();
///     std::cout << "Computation 2: Wall time = " << timer.GetWall() << " s. CPU time = " << timer.GetCpu() << " s.\n";
/// ```
///
/// Note that it is also possible to directly put the timer object to the output stream:
///
/// ```cpp
///     dip::Timer timer;
///     // do some computation
///     timer.Stop();
///     std::cout << "Computation 1: " << timer << std::endl;
/// ```
///
/// Wall time is the real-world time that elapsed. CPU time is the time that the CPU spent working for the current
/// program. These differ in two ways: CPU time might pass slower if the program has to share resources with other
/// running programs; and CPU time might pass faster if there are multiple CPUs working for the same program. The
/// latter case means that, on a multi-threaded environment, CPU time is the sum of times for each of the executed
/// threads.
///
/// Wall time is obtained through `std::chrono::steady_clock`, and CPU time through `std::clock`. This object does
/// not do anything special with these standar library routines, except for providing a simpler interface.
class DIP_NO_EXPORT Timer {
   public:

      /// \brief The default-constructed object records its creation time as the start time for the timer.
      Timer() {
         Reset();
      }

      /// \brief Records the current time as the start time for the timer.
      void Reset() {
         startWall_ = endWall_ = std::chrono::steady_clock::now();
         startCpu_ = endCpu_ = clock();
      }

      /// \brief Records the current time as the stop time for the timer.
      void Stop() {
         endCpu_ = clock();
         endWall_ = std::chrono::steady_clock::now();
      }

      /// \brief Returns the CPI time elapsed in between the creation of the timer (or the last call to `Reset`) and the last call to `Stop`.
      dfloat GetCpu() const {
         return static_cast< dfloat >( endCpu_ - startCpu_ ) / static_cast< dfloat >( CLOCKS_PER_SEC );
      }

      /// \brief Returns the wall time elapsed in between the creation of the timer (or the last call to `Reset`) and the last call to `Stop`.
      dfloat GetWall() const {
         std::chrono::duration< dfloat > time = endWall_ - startWall_;
         return time.count();
      }

      /// \brief Returns the number of clock ticks per second for the CPU clock.
      dfloat CpuResolution() const {
         return 1.0 / static_cast< dfloat >( CLOCKS_PER_SEC );
      }

      /// \brief Returns the number of clock ticks per second for the wall clock.
      dfloat WallResolution() const {
         return static_cast< dfloat >( std::chrono::steady_clock::period::num ) / static_cast< dfloat >( std::chrono::steady_clock::period::den );
      }

   private:
      std::chrono::time_point< std::chrono::steady_clock > startWall_;
      std::chrono::time_point< std::chrono::steady_clock > endWall_;
      std::clock_t startCpu_;
      std::clock_t endCpu_;
};

/// \brief Reports elapsed time to a stream.
inline std::ostream& operator<<(
      std::ostream& os,
      Timer const& timer
) {
   os << "elapsed time = " << timer.GetWall() << " s (wall), " << timer.GetCpu() << " s (CPU)";
   return os;
}

/// \}

} // namespace testing

} // namespace dip

#endif // DIP_TESTING_H
