/*
 * DIPlib 3.0
 * This file defines the "HSI" color space
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

class hsi2grey : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return "HSI"; }
      virtual String OutputColorSpace() const override { return "grey"; }
      virtual dip::uint Cost() const override { return 100; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = input[ 2 ];
         } while( ++input, ++output );
      }
};

class grey2hsi : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return "grey"; }
      virtual String OutputColorSpace() const override { return "HSI"; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = 0;
            output[ 1 ] = 0;
            output[ 2 ] = input[ 0 ];
         } while( ++input, ++output );
      }
};

class rgb2hsi : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return "RGB"; }
      virtual String OutputColorSpace() const override { return "HSI"; }
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
            // Intensity
            dfloat I = RGBsum / 3.0;
            // Saturation
            dfloat S = ( I >= RGBmed ) ? ( 3.0 / 2.0 ) * ( RGBmax - I ) : ( 3.0 / 2.0 ) * ( I - RGBmin );
            if( S < 1e-6 ) { S = 0.0; } // Avoid rounding error causing a non-zero saturation
            // Hue sextant
            int lambda = 0;
          //if(( R > G ) && ( G >= B )) { lambda = 0; } else
            if(( G >= R ) && ( R > B )) { lambda = 1; } else
            if(( G > B ) && ( B >= R )) { lambda = 2; } else
            if(( B >= G ) && ( G > R )) { lambda = 3; } else
            if(( B > R ) && ( R >= G )) { lambda = 4; } else
            if(( R >= B ) && ( B > G )) { lambda = 5; }
            // Hue
            dfloat phi = S != 0.0 ? 0.5 - 3.0 / 2.0 * ( I - RGBmed ) / S : 0.0;
            if( lambda % 2 ) { phi = 1.0 - phi; }
            dfloat H = ( lambda + phi ) * 60.0;
            // Output
            output[ 0 ] = H;
            output[ 1 ] = S;
            output[ 2 ] = I;

         } while( ++input, ++output );
      }
};

class hsi2rgb : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return "HSI"; }
      virtual String OutputColorSpace() const override { return "RGB"; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            // Input
            dfloat H = input[ 0 ];
            dfloat S = input[ 1 ];
            dfloat I = input[ 2 ];
            // Hue sextant
            H /= 60.0;
            int lambda = static_cast< int >( std::floor( H ));
            double phi = H - lambda;
            lambda = lambda % 6;
            if( lambda % 2 ) { phi = 1.0 - phi; }
            // Sorted RGB values
            double RGBmed = I - 2.0 / 3.0 * ( 1.0 / 2.0 - phi ) * S;
            double RGBmax, RGBmin;
            if( phi < 0.5 ) {
               RGBmax = I + 2.0 / 3.0 * S;
               RGBmin = 3.0 * I - RGBmax - RGBmed;
            } else {
               RGBmin = I - 2.0 / 3.0 * S;
               RGBmax = 3.0 * I - RGBmin - RGBmed;
            }
            // Unsort RGB values
            dfloat R, G, B;
            switch( lambda ) {
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

} // namespace

} // namespace dip
