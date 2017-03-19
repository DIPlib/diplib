/*
 * DIPlib 3.0
 * This file defines the "RGB" and "nlRGB" color spaces
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

class rgb2grey : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return "RGB"; }
      virtual String OutputColorSpace() const override { return "grey"; }
      virtual dip::uint Cost() const override { return 100; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            // TODO: configure white point
            // We use the second row (Y) of the XYZ matrix here
            output[ 0 ] = input[ 0 ] * 0.212671 +
                          input[ 1 ] * 0.715160 +
                          input[ 2 ] * 0.072169;
         } while( ++input, ++output );
      }
};

class grey2rgb : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return "grey"; }
      virtual String OutputColorSpace() const override { return "RGB"; }
      virtual dip::uint Cost() const override { return 100; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = input[ 0 ];
            output[ 1 ] = input[ 0 ];
            output[ 2 ] = input[ 0 ];
         } while( ++input, ++output );
      }
};

class rgb2nlrgb : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return "RGB"; }
      virtual String OutputColorSpace() const override { return "nlRGB"; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = std::pow( input[ 0 ] / 255.0, 2.5 ) * 255.0;
            output[ 1 ] = std::pow( input[ 1 ] / 255.0, 2.5 ) * 255.0;
            output[ 2 ] = std::pow( input[ 2 ] / 255.0, 2.5 ) * 255.0;
         } while( ++input, ++output );
      }
};

class nlrgb2rgb : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return "nlRGB"; }
      virtual String OutputColorSpace() const override { return "RGB"; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = std::pow( input[ 0 ] / 255.0, 0.4 ) * 255.0;
            output[ 1 ] = std::pow( input[ 1 ] / 255.0, 0.4 ) * 255.0;
            output[ 2 ] = std::pow( input[ 2 ] / 255.0, 0.4 ) * 255.0;
         } while( ++input, ++output );
      }
};

} // namespace

} // namespace dip
