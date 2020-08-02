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

namespace dip {

namespace {

constexpr char const* CMY_name = "CMY";
constexpr char const* CMYK_name = "CMYK";

class rgb2cmy : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return RGB_name; }
      virtual String OutputColorSpace() const override { return CMY_name; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = 255.0 - input[ 0 ];
            output[ 1 ] = 255.0 - input[ 1 ];
            output[ 2 ] = 255.0 - input[ 2 ];
         } while( ++input, ++output );
      }
};

class cmy2rgb : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return CMY_name; }
      virtual String OutputColorSpace() const override { return RGB_name; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = ( 255.0 - input[ 0 ] );
            output[ 1 ] = ( 255.0 - input[ 1 ] );
            output[ 2 ] = ( 255.0 - input[ 2 ] );
         } while( ++input, ++output );
      }
};

class cmy2cmyk : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return CMY_name; }
      virtual String OutputColorSpace() const override { return CMYK_name; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            dfloat k = std::min( std::min( input[ 0 ], input[ 1 ] ), input[ 2 ] );
            k = clamp( k, 0.0, 254.9999 );
            // The alternative definition doesn't divide by 1-k.
            output[ 0 ] = ( input[ 0 ] - k ) / ( 255.0 - k );
            output[ 1 ] = ( input[ 1 ] - k ) / ( 255.0 - k );
            output[ 2 ] = ( input[ 2 ] - k ) / ( 255.0 - k );
            output[ 3 ] = k;
         } while( ++input, ++output );
      }
};

class cmyk2cmy : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return CMYK_name; }
      virtual String OutputColorSpace() const override { return CMY_name; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            dfloat k = input[ 3 ];
            // The alternative definition doesn't multiply by 1-k, and therefore doesn't need the `std::min` either.
            output[ 0 ] = std::min( input[ 0 ] * ( 255.0 - k ) + k, 255.0 );
            output[ 1 ] = std::min( input[ 1 ] * ( 255.0 - k ) + k, 255.0 );
            output[ 2 ] = std::min( input[ 2 ] * ( 255.0 - k ) + k, 255.0 );
         } while( ++input, ++output );
      }
};

} // namespace

} // namespace dip
