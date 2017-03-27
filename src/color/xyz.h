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
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = input[ 0 ] * Xn_ / 255;
            output[ 1 ] = input[ 0 ] * Yn_ / 255;
            output[ 2 ] = input[ 0 ] * Zn_ / 255;
         } while( ++input, ++output );
      }
      void SetWhitePoint( ColorSpaceManager::WhitePointMatrix const& whitePoint ) {
         Xn_ = whitePoint[ 0 ] + whitePoint[ 3 ] + whitePoint[ 6 ];
         Yn_ = whitePoint[ 1 ] + whitePoint[ 4 ] + whitePoint[ 7 ];
         Zn_ = whitePoint[ 2 ] + whitePoint[ 5 ] + whitePoint[ 8 ];
      }
   private:
      dfloat Xn_ = 0.9504700;
      dfloat Yn_ = 1.0000000;
      dfloat Zn_ = 1.0888299;
};

class rgb2xyz : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return "RGB"; }
      virtual String OutputColorSpace() const override { return "XYZ"; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = ( input[ 0 ] * matrix_[ 0 ] + input[ 1 ] * matrix_[ 3 ] + input[ 2 ] * matrix_[ 6 ] ) / 255;
            output[ 1 ] = ( input[ 0 ] * matrix_[ 1 ] + input[ 1 ] * matrix_[ 4 ] + input[ 2 ] * matrix_[ 7 ] ) / 255;
            output[ 2 ] = ( input[ 0 ] * matrix_[ 2 ] + input[ 1 ] * matrix_[ 5 ] + input[ 2 ] * matrix_[ 8 ] ) / 255;
         } while( ++input, ++output );
      }
      void SetWhitePoint( ColorSpaceManager::WhitePointMatrix const& whitePoint ) {
         matrix_ = whitePoint;
      }
   private:
      ColorSpaceManager::WhitePointMatrix matrix_{{ 0.4124564, 0.2126729, 0.0193339, 0.3575761, 0.7151521, 0.1191920, 0.1804375, 0.0721750, 0.9503040 }};
};

class xyz2rgb : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return "XYZ"; }
      virtual String OutputColorSpace() const override { return "RGB"; }
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = ( input[ 0 ] * invMatrix_[ 0 ] + input[ 1 ] * invMatrix_[ 3 ] + input[ 2 ] * invMatrix_[ 6 ] ) * 255;
            output[ 1 ] = ( input[ 0 ] * invMatrix_[ 1 ] + input[ 1 ] * invMatrix_[ 4 ] + input[ 2 ] * invMatrix_[ 7 ] ) * 255;
            output[ 2 ] = ( input[ 0 ] * invMatrix_[ 2 ] + input[ 1 ] * invMatrix_[ 5 ] + input[ 2 ] * invMatrix_[ 8 ] ) * 255;
         } while( ++input, ++output );
      }
      void SetWhitePoint( ColorSpaceManager::WhitePointMatrix const& whitePoint ) {
         Inverse( 3, whitePoint.data(), invMatrix_.data() );
      }
   private:
      ColorSpaceManager::WhitePointMatrix invMatrix_{{ 3.2404550, -0.9692666, 0.0556434, -1.5371391, 1.8760113, -0.2040259, -0.4985316, 0.0415561, 1.0572253 }};
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
