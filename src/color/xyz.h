/*
 * DIPlib 3.0
 * This file defines the "XYZ" and "Yxy" color spaces
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

class xyz2grey : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return "XYZ"; }
      virtual String OutputColorSpace() const override { return "grey"; }
      virtual dip::uint Cost() const override { return 100; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = input[ 1 ] * 255;
         } while( ++input, ++output );
      }
};

class yxy2grey : public xyz2grey {
   public:
      virtual String InputColorSpace() const override { return "Yxy"; }
};

class grey2xyz : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return "grey"; }
      virtual String OutputColorSpace() const override { return "XYZ"; }
      virtual dip::uint Cost() const override { return 100; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            // TODO: configure white point
            output[ 0 ] = input[ 0 ] * 0.95046 / 255;
            output[ 1 ] = input[ 0 ] * 1.00000 / 255;
            output[ 2 ] = input[ 0 ] * 1.08875 / 255;
         } while( ++input, ++output );
      }
};

class rgb2xyz : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return "RGB"; }
      virtual String OutputColorSpace() const override { return "XYZ"; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            // TODO: configure white point
            output[ 0 ] = ( input[ 0 ] * 0.412453 + input[ 1 ] * 0.357580 + input[ 2 ] * 0.180423 ) / 255;
            output[ 1 ] = ( input[ 0 ] * 0.212671 + input[ 1 ] * 0.715160 + input[ 2 ] * 0.072169 ) / 255;
            output[ 2 ] = ( input[ 0 ] * 0.019334 + input[ 1 ] * 0.119193 + input[ 2 ] * 0.950227 ) / 255;
         } while( ++input, ++output );
      }
};

class xyz2rgb : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return "XYZ"; }
      virtual String OutputColorSpace() const override { return "RGB"; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            // TODO: configure white point
            output[ 0 ] = ( input[ 0 ] *  3.240479 + input[ 1 ] * -1.537150 + input[ 2 ] * -0.498535 ) * 255;
            output[ 1 ] = ( input[ 0 ] * -0.969256 + input[ 1 ] *  1.875992 + input[ 2 ] *  0.041556 ) * 255;
            output[ 2 ] = ( input[ 0 ] *  0.055648 + input[ 1 ] * -0.204043 + input[ 2 ] *  1.057311 ) * 255;
         } while( ++input, ++output );
      }
};

class yxy2xyz : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return "Yxy"; }
      virtual String OutputColorSpace() const override { return "XYZ"; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = input[ 1 ];
            dfloat sum = input[ 0 ] + input[ 1 ] + input[ 2 ];
            output[ 1 ] = sum == 0 ? 0 : input[ 0 ] / sum;
            output[ 2 ] = sum == 0 ? 0 : input[ 1 ] / sum;
         } while( ++input, ++output );
      }
};

class xyz2yxy : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return "XYZ"; }
      virtual String OutputColorSpace() const override { return "Yxy"; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            dfloat sum = input[ 2 ] == 0 ? 0 : input[ 0 ] / input[ 2 ];
            output[ 0 ] = input[ 1 ] * sum;
            output[ 1 ] = input[ 0 ];
            output[ 2 ] = sum - output[ 0 ] - output[ 1 ];
         } while( ++input, ++output );
      }
};

} // namespace

} // namespace dip
