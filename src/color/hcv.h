/*
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

constexpr char const* HCV_name = "HCV";
constexpr char const* HSV_name = "HSV";

// A proper floating-point modulo operation, y is assumed positive.
inline double modulo(double x, double y) {
   double m = std::fmod(x, y);
   if( m < 0 ) {
      m += y;
   }
   return m;
}

// Ranges:
//    0 < Value < 255
//    0 < Chroma < 255
//    0 < Hue < 6
//    0 < Saturation < 1

class rgb2hcv : public ColorSpaceConverter {
   public:
      String InputColorSpace() const override { return RGB_name; }
      String OutputColorSpace() const override { return HCV_name; }
      void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            // Input
            dfloat R = input[ 0 ];
            dfloat G = input[ 1 ];
            dfloat B = input[ 2 ];
            // Sort RGB values
            dfloat RGBmin = R;
            dfloat RGBmed = G;
            dfloat RGBmax = B;
            if (RGBmin > RGBmed) {
               std::swap( RGBmin, RGBmed );
            }
            if (RGBmed > RGBmax) {
               std::swap( RGBmed, RGBmax );
               if (RGBmin > RGBmed) {
                  std::swap( RGBmin, RGBmed );
               }
            }
            // Value
            dfloat V = RGBmax;
            // Chroma
            dfloat C = RGBmax - RGBmin;
            // Hue sextant
            dfloat H = 0;
            if( C > 0 ) {
               if( RGBmax == R ) {
                  H = modulo(( G - B ) / C, 6.0 );
               } else if( RGBmax == G ) {
                  H = ( B - R ) / C + 2;
               } else { // RGBmax == B
                  H = ( R - G ) / C + 4;
               }
            }
            H *= 60.0;
            // Output
            output[ 0 ] = H;
            output[ 1 ] = C;
            output[ 2 ] = V;
         } while( ++input, ++output );
      }
};

class hcv2rgb : public ColorSpaceConverter {
   public:
      String InputColorSpace() const override { return HCV_name; }
      String OutputColorSpace() const override { return RGB_name; }
      void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            // Input
            dfloat H = input[ 0 ];
            dfloat C = input[ 1 ];
            dfloat V = input[ 2 ];
            dfloat RGBmin = V-C;
            dfloat RGBmax = V;
            // Hue sextant
            H = modulo( H / 60.0, 6 );
            int Hs = static_cast< int >( std::floor( H ));
            dfloat m = H - Hs;
            dfloat RGBmed = C * ( Hs & 1 ? 1 - m : m ) + RGBmin;
            dfloat R, G, B;
            // Unsort RGB values
            switch( Hs ) {
               default:
               case 0: R = RGBmax; G = RGBmed; B = RGBmin; break;
               case 1: R = RGBmed; G = RGBmax; B = RGBmin; break;
               case 2: R = RGBmin; G = RGBmax; B = RGBmed; break;
               case 3: R = RGBmin; G = RGBmed; B = RGBmax; break;
               case 4: R = RGBmed; G = RGBmin; B = RGBmax; break;
               case 5: R = RGBmax; G = RGBmin; B = RGBmed; break;
            }
            // Output
            output[ 0 ] = R;
            output[ 1 ] = G;
            output[ 2 ] = B;
         } while( ++input, ++output );
      }
};

class hcv2hsv : public ColorSpaceConverter {
   public:
      String InputColorSpace() const override { return HCV_name; }
      String OutputColorSpace() const override { return HSV_name; }
      void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = input[ 0 ];
            output[ 1 ] = input[ 2 ] == 0 ? 0 : input[ 1 ] / input[ 2 ]; // Chroma = Saturation * Value
            output[ 2 ] = input[ 2 ];
         } while( ++input, ++output );
      }
};

class hsv2hcv : public ColorSpaceConverter {
   public:
      String InputColorSpace() const override { return HSV_name; }
      String OutputColorSpace() const override { return HCV_name; }
      void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = input[ 0 ];
            output[ 1 ] = input[ 1 ] * input[ 2 ]; // Chroma = Saturation * Value
            output[ 2 ] = input[ 2 ];
         } while( ++input, ++output );
      }
};

} // namespace

} // namespace dip
