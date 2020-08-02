/*
 * DIPlib 3.0
 * This file defines the "RGB" and "sRGB" color spaces
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

constexpr char const* RGB_name = "RGB";
constexpr char const* sRGB_name = "sRGB";

class rgb2grey : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return RGB_name; }
      virtual String OutputColorSpace() const override { return dip::S::GREY; }
      virtual dip::uint Cost() const override { return 100; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = input[ 0 ] * Y_[ 0 ] +
                          input[ 1 ] * Y_[ 1 ] +
                          input[ 2 ] * Y_[ 2 ];
         } while( ++input, ++output );
      }
      void SetWhitePoint( XYZMatrix const& matrix ) {
         Y_[ 0 ] = matrix[ 1 ];
         Y_[ 1 ] = matrix[ 4 ];
         Y_[ 2 ] = matrix[ 7 ];
      }
   private:
      std::array< dfloat, 3 > Y_{{ 0.2126729, 0.7151521, 0.072175 }}; // The Y row of the XYZ matrix
};

class grey2rgb : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return dip::S::GREY; }
      virtual String OutputColorSpace() const override { return RGB_name; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = input[ 0 ];
            output[ 1 ] = input[ 0 ];
            output[ 2 ] = input[ 0 ];
         } while( ++input, ++output );
      }
};

// sRGB transformations

constexpr double a = 0.055;
constexpr double gamma = 2.4;
constexpr double K_0 = a / ( gamma - 1 );
constexpr double phi = 12.923210180787853; // == std::pow(( 1 + a ) / gamma, gamma ) * std::pow(( gamma - 1 ) / a, gamma - 1 );

// Linear to sRGB
inline dfloat LinearToS( dfloat in ) {
   if( in <= K_0 / phi ) {
      return in * phi;
   } else {
      return ( 1 + a ) * std::pow( in, 1.0 / gamma ) - a;
   }
}

// sRGB to Linear
inline dfloat SToLinear( dfloat in ) {
   if( in <= K_0 ) {
      return in / phi;
   } else {
      return std::pow(( in + a ) / ( 1 + a ), gamma );
   }
}

class rgb2srgb : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return RGB_name; }
      virtual String OutputColorSpace() const override { return sRGB_name; }
      virtual dip::uint Cost() const override { return 2; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = LinearToS( input[ 0 ] / 255.0 ) * 255.0;
            output[ 1 ] = LinearToS( input[ 1 ] / 255.0 ) * 255.0;
            output[ 2 ] = LinearToS( input[ 2 ] / 255.0 ) * 255.0;
         } while( ++input, ++output );
      }
};

class srgb2rgb : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return sRGB_name; }
      virtual String OutputColorSpace() const override { return RGB_name; }
      virtual dip::uint Cost() const override { return 2; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = SToLinear( input[ 0 ] / 255.0 ) * 255.0;
            output[ 1 ] = SToLinear( input[ 1 ] / 255.0 ) * 255.0;
            output[ 2 ] = SToLinear( input[ 2 ] / 255.0 ) * 255.0;
         } while( ++input, ++output );
      }
};

} // namespace

} // namespace dip
