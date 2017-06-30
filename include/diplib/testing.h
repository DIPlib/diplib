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

#include "diplib.h"
#include "diplib/iterators.h"


/// \file
/// \brief Declares functions to help test your code.


namespace dip {

/// \group testing Tools for testing and debugging.
/// \ingroup infrastructure
/// \{

/// \brief Tools for testing and debugging.
namespace testing {

namespace detail {

template< typename T, typename std::enable_if< std::is_integral< T >::value, int >::type = 0 >
T Round( T v, int /*digits*/) {
   return v;
}

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

template< typename T >
std::complex< T > Round( std::complex< T > v, int digits ) {
   return { Round( v.real(), digits ), Round( v.imag(), digits ) };
}

} // namespace detail


/// \brief Outputs pixel values of a small image to `stdout`.
///
/// If the image is a tensor image, shows only the first tensor component.
///
/// The first template parameter must match the image's data type.
///
/// An optional second template parameter determines the precision for displaying floating-point values.
template< typename TPI, int DIGITS = 4 >
void PrintPixelValues(
      dip::Image img
) {
   DIP_THROW_IF( img.DataType() != dip::DataType( TPI()), "Wrong template parameter to PrintPixelValues() used" );
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

} // namespace testing

/// \}

} // namespace dip

#endif // DIP_TESTING_H
