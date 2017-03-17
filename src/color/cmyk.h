/*
 * DIPlib 3.0
 * This file defines the "CMY" and "CMYK" color spaces
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
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

#include "diplib.h"
#include "diplib/measurement.h"

namespace dip {

namespace {

class rgb2cmy : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const { return "RGB"; }
      virtual String OutputColorSpace() const { return "CMY"; }
      virtual dip::uint Cost() const { return 100; }
      virtual void Convert( dfloat const* input, dfloat* output ) {
         output[ 0 ] = ( 255.0 - input[ 0 ] ) * ( 1.0 / 255.0 );
         output[ 1 ] = ( 255.0 - input[ 1 ] ) * ( 1.0 / 255.0 );
         output[ 2 ] = ( 255.0 - input[ 2 ] ) * ( 1.0 / 255.0 );
      }
};

class cmy2rgb : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const { return "CMY"; }
      virtual String OutputColorSpace() const { return "RGB"; }
      virtual dip::uint Cost() const { return 100; }
      virtual void Convert( dfloat const* input, dfloat* output ) {
         output[ 0 ] = ( 1.0 - input[ 0 ] ) * 255.0;
         output[ 1 ] = ( 1.0 - input[ 1 ] ) * 255.0;
         output[ 2 ] = ( 1.0 - input[ 2 ] ) * 255.0;
      }
};

class cmy2cmyk : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const { return "CMY"; }
      virtual String OutputColorSpace() const { return "CMYK"; }
      virtual dip::uint Cost() const { return 100; }
      virtual void Convert( dfloat const* input, dfloat* output ) {
         dfloat k = std::min( std::min( input[ 0 ], input[ 1 ] ), input[ 2 ] );
         k = clamp( k, 0.0, 0.99999 );
         // The alternative definition doesn't divide by 1-k.
         output[ 0 ] = ( input[ 0 ] - k ) / ( 1.0 - k );
         output[ 1 ] = ( input[ 1 ] - k ) / ( 1.0 - k );
         output[ 2 ] = ( input[ 2 ] - k ) / ( 1.0 - k );
         output[ 3 ] = k;
      }
};

class cmyk2cmy : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const { return "CMYK"; }
      virtual String OutputColorSpace() const { return "CMY"; }
      virtual dip::uint Cost() const { return 100; }
      virtual void Convert( dfloat const* input, dfloat* output ) {
         dfloat k = std::min( std::min( input[ 0 ], input[ 1 ] ), input[ 2 ] );
         k = clamp( k, 0.0, 0.99999 );
         // The alternative definition doesn't multiply by 1-k, and therefore doesn't need the `std::min` either.
         output[ 0 ] = std::min( input[ 0 ] * ( 1.0 - k ) + k, 1.0 );
         output[ 1 ] = std::min( input[ 1 ] * ( 1.0 - k ) + k, 1.0 );
         output[ 2 ] = std::min( input[ 2 ] * ( 1.0 - k ) + k, 1.0 );
      }
};

} // namespace

} // namespace dip
