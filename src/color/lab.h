/*
 * DIPlib 3.0
 * This file defines the "Lab", "Luv" and LCH color spaces
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

constexpr dfloat epsilon = 0.008856;
constexpr dfloat epsilon1_3 = 0.206893; // ==  std::cbrt( epsilon );
constexpr dfloat kappa = 903.3;

class lab2grey : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return "Lab"; }
      virtual String OutputColorSpace() const override { return "grey"; }
      virtual dip::uint Cost() const override { return 100; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            // TODO: configure white point
            constexpr dfloat Yn = 1.00000;
            dfloat L = input[ 0 ];
            dfloat y;
            if( L > kappa * epsilon ) {
               y = ( L + 16.0 ) / 116.0;
               y = y * y * y;
            } else {
               y = L / kappa;
            }
            output[ 0 ] = y * Yn * 255;
         } while( ++input, ++output );
      }
};

class grey2lab : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return "grey"; }
      virtual String OutputColorSpace() const override { return "Lab"; }
      virtual dip::uint Cost() const override { return 100; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            // TODO: configure white point
            constexpr dfloat Yn = 1.00000;
            dfloat y = input[ 0 ] / ( 255 * Yn );
            output[ 0 ] = y > epsilon
                          ? 116.0 * std::cbrt( y ) - 16.0
                          : kappa * y;
            output[ 1 ] = 0;
            output[ 2 ] = 0;
         } while( ++input, ++output );
      }
};

class lab2xyz : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return "Lab"; }
      virtual String OutputColorSpace() const override { return "XYZ"; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            // TODO: configure white point
            constexpr dfloat Xn = 0.95046;
            constexpr dfloat Yn = 1.00000;
            constexpr dfloat Zn = 1.08875;
            dfloat fy = ( input[ 0 ] + 16.0 ) / 116.0;
            dfloat fx = input[ 1 ] / 500.0 + fy;
            dfloat fz = fy - input[ 2 ] / 200.0;
            dfloat x = fx > epsilon1_3
                       ? fx * fx * fx
                       : ( 116.0 * fx - 16.0 ) / kappa;
            dfloat y = fy > epsilon1_3
                       ? fy * fy * fy
                       : input[ 0 ] / kappa;
            dfloat z = fz > epsilon1_3
                       ? fz * fz * fz
                       : ( 116.0 * fz - 16.0 ) / kappa;
            output[ 0 ] = x * Xn;
            output[ 1 ] = y * Yn;
            output[ 2 ] = z * Zn;
         } while( ++input, ++output );
      }
};

class xyz2lab : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return "XYZ"; }
      virtual String OutputColorSpace() const override { return "Lab"; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            // TODO: configure white point
            constexpr dfloat Xn = 0.95046;
            constexpr dfloat Yn = 1.00000;
            constexpr dfloat Zn = 1.08875;
            dfloat x = input[ 0 ] / Xn;
            dfloat y = input[ 1 ] / Yn;
            dfloat z = input[ 2 ] / Zn;
            dfloat fx = x > epsilon
                        ? std::cbrt( x )
                        : ( kappa * x + 16.0 ) / 116.0;
            dfloat fy = y > epsilon
                        ? std::cbrt( y )
                        : ( kappa * y + 16.0 ) / 116.0;
            dfloat fz = z > epsilon
                        ? std::cbrt( z )
                        : ( kappa * z + 16.0 ) / 116.0;
            output[ 0 ] = 116.0 * fy - 16.0;
            output[ 1 ] = 500.0 * ( fx - fy );
            output[ 2 ] = 200.0 * ( fy - fz );
         } while( ++input, ++output );
      }
};

class luv2grey : public lab2grey {
   public:
      virtual String InputColorSpace() const override { return "Luv"; }
      // Luv to grey is identical to Lab to grey, so we re-use its code.
};

class grey2luv : public grey2lab {
   public:
      virtual String OutputColorSpace() const override { return "Luv"; }
      // Grey to Luv is identical to grey to Lab, so we re-use its code.
};

class luv2xyz : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return "Luv"; }
      virtual String OutputColorSpace() const override { return "XYZ"; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            // TODO: configure white point
            constexpr dfloat Xn = 0.95046;
            constexpr dfloat Yn = 1.00000;
            constexpr dfloat Zn = 1.08875;
            dfloat sum = Xn + 15 * Yn + 3 * Zn;
            dfloat un = 4 * Xn / sum;
            dfloat vn = 9 * Yn / sum;
            dfloat L = input[ 0 ];
            dfloat Y;
            if( L > kappa * epsilon ) {
               Y = ( L + 16.0 ) / 116.0;
               Y = Y * Y * Y * Yn;
            } else {
               Y = L / kappa * Yn;
            }
            dfloat a = 52 / 3.0 * L / ( input[ 1 ] + 13 * L * un );
            dfloat d = Y * 39 * L / ( input[ 2 ] + 13 * L * vn );
            dfloat X = d / a;
            output[ 0 ] = X;
            output[ 1 ] = Y;
            output[ 2 ] = X * ( a - 1.0/3.0 ) - 5 * Y;
         } while( ++input, ++output );
      }
};

class xyz2luv : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return "XYZ"; }
      virtual String OutputColorSpace() const override { return "Luv"; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            // TODO: configure white point
            constexpr dfloat Xn = 0.95046;
            constexpr dfloat Yn = 1.00000;
            constexpr dfloat Zn = 1.08875;
            dfloat sum = Xn + 15 * Yn + 3 * Zn;
            dfloat un = 4 * Xn / sum;
            dfloat vn = 9 * Yn / sum;
            sum = input[ 0 ] + 15 * input[ 1 ] + 3 * input[ 2 ];
            dfloat u = 4 * input[ 0 ] / sum;
            dfloat v = 9 * input[ 1 ] / sum;
            dfloat y = input[ 1 ] / Yn;
            dfloat L = y > epsilon
                          ? 116.0 * std::cbrt( y ) - 16.0
                          : kappa * y;
            output[ 0 ] = L;
            output[ 1 ] = 13.0 * L * ( u - un );
            output[ 2 ] = 13.0 * L * ( v - vn );
         } while( ++input, ++output );
      }
};

class lch2grey : public lab2grey {
   public:
      virtual String InputColorSpace() const override { return "LCH"; }
      // LCH to grey is identical to Lab to grey, so we re-use its code.
};

class grey2lch : public grey2lab {
   public:
      virtual String OutputColorSpace() const override { return "LCH"; }
      // Grey to LCH is identical to grey to Lab, so we re-use its code.
};

class lch2lab : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return "LCH"; }
      virtual String OutputColorSpace() const override { return "Lab"; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = input[ 0 ];
            dfloat h = pi * input[ 2 ] / 180.0;
            output[ 1 ] = input[ 1 ] * std::cos( h );
            output[ 2 ] = input[ 1 ] * std::sin( h );
         } while( ++input, ++output );
      }
};

class lab2lch : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return "Lab"; }
      virtual String OutputColorSpace() const override { return "LCH"; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = input[ 0 ];
            output[ 1 ] = hypot( input[ 1 ], input[ 2 ] );
            dfloat H = 180.0 * atan2( input[ 2 ], input[ 1 ] ) / pi;
            if( H < 0.0 ) {
               H += 360.0;
            }
            output[ 2 ] = H;
         } while( ++input, ++output );
      }
};

} // namespace

} // namespace dip
