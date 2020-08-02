/*
 * DIPlib 3.0
 * This file defines the "ISH" and "ICH" color spaces
 *
 * (c)2017, Cris Luengo.
 * (c)2016, Flagship Biosciences, Inc.
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

constexpr char const* ICH_name = "ICH";
constexpr char const* ISH_name = "ISH";

// sqrt(3)/2 = 0.866025403784439
constexpr std::array< dfloat, 9 > rotationMatrix = {{ 0.333333333333333, 1.0, 0.0, 0.333333333333333, -0.5, 0.866025403784439, 0.333333333333333, -0.5, -0.866025403784439 }};
constexpr std::array< dfloat, 9 > invRotMatrix = {{ 1.0, 1.0, 1.0, 0.666666666666667, -0.333333333333333, -0.333333333333333, 0.0, 0.577350269189626, -0.577350269189626 }};

class ich2grey : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return ICH_name; }
      virtual String OutputColorSpace() const override { return dip::S::GREY; }
      virtual dip::uint Cost() const override { return 100; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = input[ 0 ];
         } while( ++input, ++output );
      }
};

class grey2ich : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return dip::S::GREY; }
      virtual String OutputColorSpace() const override { return ICH_name; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = input[ 0 ];
            output[ 1 ] = 0;
            output[ 2 ] = 0;
         } while( ++input, ++output );
      }
};

class rgb2ich : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return RGB_name; }
      virtual String OutputColorSpace() const override { return ICH_name; }
      virtual dip::uint Cost() const override { return 2; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            // Input
            dfloat R = input[ 0 ];
            dfloat G = input[ 1 ];
            dfloat B = input[ 2 ];
            // Rotation
            dfloat I = rotationMatrix[ 0 ] * R + rotationMatrix[ 3 ] * G + rotationMatrix[ 6 ] * B;
            dfloat a = rotationMatrix[ 1 ] * R + rotationMatrix[ 4 ] * G + rotationMatrix[ 7 ] * B;
            dfloat b = rotationMatrix[ 2 ] * R + rotationMatrix[ 5 ] * G + rotationMatrix[ 8 ] * B;
            // Output
            output[ 0 ] = I;
            output[ 1 ] = std::hypot( a, b );
            output[ 2 ] = std::atan2( b, a ) / pi * 180.0;
         } while( ++input, ++output );
      }
};

class ich2rgb : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return ICH_name; }
      virtual String OutputColorSpace() const override { return RGB_name; }
      virtual dip::uint Cost() const override { return 2; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            // Input
            dfloat I = input[ 0 ];
            dfloat C = input[ 1 ];
            dfloat H = input[ 2 ] * pi / 180.0;
            // Chromatic plane coordinates
            dfloat a = C * std::cos( H );
            dfloat b = C * std::sin( H );
            // Rotation, output
            output[ 0 ] = invRotMatrix[ 0 ] * I + invRotMatrix[ 3 ] * a + invRotMatrix[ 6 ] * b;
            output[ 1 ] = invRotMatrix[ 1 ] * I + invRotMatrix[ 4 ] * a + invRotMatrix[ 7 ] * b;
            output[ 2 ] = invRotMatrix[ 2 ] * I + invRotMatrix[ 5 ] * a + invRotMatrix[ 8 ] * b;
         } while( ++input, ++output );
      }
};

class ish2grey : public ich2grey {
   public:
      virtual String InputColorSpace() const override { return ISH_name; }
      // ISH to grey is identical to ICH to grey, so we re-use its code.
};

class grey2ish : public grey2ich {
   public:
      virtual String OutputColorSpace() const override { return ISH_name; }
      // Grey to ISH is identical to grey to ICH, so we re-use its code.
};

class ich2ish : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return ICH_name; }
      virtual String OutputColorSpace() const override { return ISH_name; }
      virtual dip::uint Cost() const override { return 2; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = input[ 0 ];
            dfloat H = output[ 2 ] = input[ 2 ];
            H *= pi / 180.0;
            output[ 1 ] = input[ 1 ] * 2.0 / std::sqrt( 3.0 ) * std::sin( 2.0 / 3.0 * pi - std::fmod( H, pi / 3.0 ));
         } while( ++input, ++output );
      }
};

class ish2ich : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return ISH_name; }
      virtual String OutputColorSpace() const override { return ICH_name; }
      virtual dip::uint Cost() const override { return 2; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = input[ 0 ];
            dfloat H = output[ 2 ] = input[ 2 ];
            H *= pi / 180.0;
            output[ 1 ] = input[ 1 ] * std::sqrt( 3.0 ) / 2.0 / std::sin( 2.0 / 3.0 * pi - std::fmod( H, pi / 3.0 ));
         } while( ++input, ++output );
      }
};

} // namespace

} // namespace dip
