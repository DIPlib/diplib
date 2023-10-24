/*
 * (c)2023, Cris Luengo.
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

constexpr char const* Oklab_name = "Oklab";
constexpr char const* Oklch_name = "Oklch";

class oklab2grey : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return Oklab_name; }
      virtual String OutputColorSpace() const override { return dip::S::GREY; }
      virtual dip::uint Cost() const override { return 101; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            dfloat l = input[ 0 ];
            l = l * l * l;
            output[ 0 ] = l * 255;
         } while( ++input, ++output );
      }
};

class grey2oklab : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return dip::S::GREY; }
      virtual String OutputColorSpace() const override { return Oklab_name; }
      virtual dip::uint Cost() const override { return 3; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            dfloat l = input[ 0 ] / 255;  // Yn == 1.000 by definition
            l = std::cbrt( l );
            output[ 0 ] = l;
            output[ 1 ] = 0;
            output[ 2 ] = 0;
         } while( ++input, ++output );
      }
};

class oklab2xyz : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return Oklab_name; }
      virtual String OutputColorSpace() const override { return XYZ_name; }
      virtual dip::uint Cost() const override { return 2; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            dfloat l = 0.999999998450520 * input[ 0 ] + 0.396337792173768 * input[ 1 ] + 0.215803758060759 * input[ 2 ];
            dfloat m = 1.000000008881761 * input[ 0 ] - 0.105561342323656 * input[ 1 ] - 0.063854174771706 * input[ 2 ];
            dfloat s = 1.000000054672411 * input[ 0 ] - 0.089484182094966 * input[ 1 ] - 1.291485537864092 * input[ 2 ];
            l = l * l * l;
            m = m * m * m;
            s = s * s * s;
            output[ 0 ] =  1.227013851103521 * l - 0.557799980651822 * m + 0.281256148966468 * s;
            output[ 1 ] = -0.040580178423281 * l + 1.112256869616830 * m - 0.071676678665601 * s;
            output[ 2 ] = -0.076381284505707 * l - 0.421481978418013 * m + 1.586163220440795 * s;
         } while( ++input, ++output );
      }
};

class xyz2oklab : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return XYZ_name; }
      virtual String OutputColorSpace() const override { return Oklab_name; }
      virtual dip::uint Cost() const override { return 3; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            dfloat l = 0.8189330101 * input[ 0 ] + 0.3618667424 * input[ 1 ] - 0.1288597137 * input[ 2 ];
            dfloat m = 0.0329845436 * input[ 0 ] + 0.9293118715 * input[ 1 ] + 0.0361456387 * input[ 2 ];
            dfloat s = 0.0482003018 * input[ 0 ] + 0.2643662691 * input[ 1 ] + 0.6338517070 * input[ 2 ];
            l = std::cbrt( l );
            m = std::cbrt( m );
            s = std::cbrt( s );
            output[ 0 ] = 0.2104542553 * l + 0.7936177850 * m - 0.0040720468 * s;
            output[ 1 ] = 1.9779984951 * l - 2.4285922050 * m + 0.4505937099 * s;
            output[ 2 ] = 0.0259040371 * l + 0.7827717662 * m - 0.8086757660 * s;
         } while( ++input, ++output );
      }
};

class oklch2grey : public oklab2grey {
   public:
      virtual String InputColorSpace() const override { return Oklch_name; }
      // Oklch to grey is identical to Oklab to grey, so we re-use its code.
};

class grey2oklch : public grey2oklab {
   public:
      virtual String OutputColorSpace() const override { return Oklch_name; }
      // Grey to Oklch is identical to grey to Oklab, so we re-use its code.
};

class oklch2oklab : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return Oklch_name; }
      virtual String OutputColorSpace() const override { return Oklab_name; }
      virtual dip::uint Cost() const override { return 2; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = input[ 0 ];
            dfloat h = pi * input[ 2 ] / 180.0;
            output[ 1 ] = input[ 1 ] * std::cos( h );
            output[ 2 ] = input[ 1 ] * std::sin( h );
         } while( ++input, ++output );
      }
};

class oklab2oklch : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return Oklab_name; }
      virtual String OutputColorSpace() const override { return Oklch_name; }
      virtual dip::uint Cost() const override { return 2; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = input[ 0 ];
            output[ 1 ] = std::hypot( input[ 1 ], input[ 2 ] );
            dfloat H = 180.0 * std::atan2( input[ 2 ], input[ 1 ] ) / pi;
            if( H < 0.0 ) {
               H += 360.0;
            }
            output[ 2 ] = H;
         } while( ++input, ++output );
      }
};

} // namespace

} // namespace dip
