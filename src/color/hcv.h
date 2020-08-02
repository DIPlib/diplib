/*
 * DIPlib 3.0
 * This file defines the "HCV" and "HSV" color spaces
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

constexpr char const* HCV_name = "HCV";
constexpr char const* HSV_name = "HSV";

// Ranges:
//    0 < Value < 255
//    0 < Chroma < 255
//    0 < Hue < 6
//    0 < Saturation < 1

class rgb2hcv : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return RGB_name; }
      virtual String OutputColorSpace() const override { return HCV_name; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            // Input
            dfloat R = input[ 0 ];
            dfloat G = input[ 1 ];
            dfloat B = input[ 2 ];
            // Sort RGB values
            dfloat RGBsum = R + G + B;
            dfloat RGBmin = std::min( R, std::min( G, B ) );
            dfloat RGBmax = std::max( R, std::max( G, B ) );
            dfloat RGBmed = RGBsum - RGBmin - RGBmax;
            // Value
            dfloat V = RGBmax;
            // Chroma
            dfloat C = RGBmax - RGBmin;
            // Hue sextant
            dfloat m = C == 0 ? 0 : ( RGBmed - RGBmin ) / C;
            dfloat H = m;
          //if( ( R == RGBmax ) && ( B == RGBmin ) ) { H =     m; } else
            if( ( G == RGBmax ) && ( B == RGBmin ) ) { H = 2 - m; } else
            if( ( G == RGBmax ) && ( R == RGBmin ) ) { H = 2 + m; } else
            if( ( B == RGBmax ) && ( R == RGBmin ) ) { H = 4 - m; } else
            if( ( B == RGBmax ) && ( G == RGBmin ) ) { H = 4 + m; } else
            if( ( R == RGBmax ) && ( G == RGBmin ) ) { H = 6 - m; }
            H = std::fmod( H, 6.0 ) * 60.0;
            // Output
            output[ 0 ] = H;
            output[ 1 ] = C;
            output[ 2 ] = V;

         } while( ++input, ++output );
      }
};

class hcv2rgb : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return HCV_name; }
      virtual String OutputColorSpace() const override { return RGB_name; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            // Input
            dfloat H = input[ 0 ];
            dfloat C = input[ 1 ];
            dfloat V = input[ 2 ];
            dfloat RGBmin = V-C;
            dfloat RGBmax = V;
            // Hue sextant
            H = std::fmod( H / 60.0, 6 );
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
      virtual String InputColorSpace() const override { return HCV_name; }
      virtual String OutputColorSpace() const override { return HSV_name; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = input[ 0 ];
            output[ 1 ] = input[ 2 ] == 0 ? 0 : input[ 1 ] / input[ 2 ]; // Chroma = Saturation * Value
            output[ 2 ] = input[ 2 ];
         } while( ++input, ++output );
      }
};

class hsv2hcv : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return HSV_name; }
      virtual String OutputColorSpace() const override { return HCV_name; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = input[ 0 ];
            output[ 1 ] = input[ 1 ] * input[ 2 ]; // Chroma = Saturation * Value
            output[ 2 ] = input[ 2 ];
         } while( ++input, ++output );
      }
};

} // namespace

} // namespace dip
