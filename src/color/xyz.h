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
            output[ 0 ] = input[ 0 ] * whitePoint_[ 0 ] / 255;
            output[ 1 ] = input[ 0 ] * whitePoint_[ 1 ] / 255;
            output[ 2 ] = input[ 0 ] * whitePoint_[ 2 ] / 255;
         } while( ++input, ++output );
      }
      void SetWhitePoint( ColorSpaceManager::XYZ const& whitePoint ) {
         whitePoint_ = whitePoint;
      }
   private:
      ColorSpaceManager::XYZ whitePoint_ = ColorSpaceManager::IlluminantD65;
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
      void SetWhitePoint( XYZMatrix const& matrix ) {
         matrix_ = matrix;
         /*
         std::cout << "matrix = "
                   << matrix_[0] << ","
                   << matrix_[1] << ","
                   << matrix_[2] << ","
                   << matrix_[3] << ","
                   << matrix_[4] << ","
                   << matrix_[5] << ","
                   << matrix_[6] << ","
                   << matrix_[7] << ","
                   << matrix_[8] << std::endl;
         */
      }
   private:
      XYZMatrix matrix_{{ 0.412348, 0.212617, 0.0193288, 0.357601, 0.715203, 0.119200, 0.180450, 0.0721801, 0.950371 }};
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
      void SetWhitePoint( XYZMatrix const& matrix ) {
         Inverse( 3, matrix.data(), invMatrix_.data() );
         /*
         std::cout << "invMatrix_ = "
                   << invMatrix_[0] << ","
                   << invMatrix_[1] << ","
                   << invMatrix_[2] << ","
                   << invMatrix_[3] << ","
                   << invMatrix_[4] << ","
                   << invMatrix_[5] << ","
                   << invMatrix_[6] << ","
                   << invMatrix_[7] << ","
                   << invMatrix_[8] << std::endl;
         */
      }
   private:
      XYZMatrix invMatrix_{{ 3.241300, -0.969197, 0.0556395, -1.53754, 1.87588, -0.204012, -0.498662, 0.0415531, 1.05715 }};
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
