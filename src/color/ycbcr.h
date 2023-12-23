/*
 * (c)2017-2023, Cris Luengo.
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

constexpr char const* YPbPr_name = "Y'PbPr";
constexpr char const* YCbCr_name = "Y'CbCr";

class srgb2ypbpr : public ColorSpaceConverter {
   public:
      String InputColorSpace() const override { return sRGB_name; }
      String OutputColorSpace() const override { return YPbPr_name; }
      void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            double R = input[ 0 ] / 255.0;
            double G = input[ 1 ] / 255.0;
            double B = input[ 2 ] / 255.0;
            double Y = Kr * R + Kg * G + Kb * B;
            output[ 0 ] = Y;
            output[ 1 ] = 0.5 * ( B - Y ) / ( 1 - Kb );
            output[ 2 ] = 0.5 * ( R - Y ) / ( 1 - Kr );
         } while( ++input, ++output );
      }
      void SetWhitePoint( XYZ const&, XYZMatrix const& matrix, XYZMatrix const& ) override {
         Kr = matrix[ 1 ];
         Kg = matrix[ 4 ];
         Kb = matrix[ 7 ];
      }
   private:
      double Kr = 0.2126729;
      double Kg = 0.7151521;
      double Kb = 0.072175; // The Y row of the XYZ matrix
};

class ypbpr2srgb : public ColorSpaceConverter {
   public:
      String InputColorSpace() const override { return YPbPr_name; }
      String OutputColorSpace() const override { return sRGB_name; }
      void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            double B = 2 * input[ 1 ] * ( 1 - Kb ) + input[ 0 ];
            double R = 2 * input[ 2 ] * ( 1 - Kr ) + input[ 0 ];
            double G = ( input[ 0 ] - R * Kr - B * Kb ) / Kg;
            output[ 0 ] = R * 255;
            output[ 1 ] = G * 255;
            output[ 2 ] = B * 255;
         } while( ++input, ++output );
      }
      void SetWhitePoint( XYZ const&, XYZMatrix const& matrix, XYZMatrix const& ) override {
         Kr = matrix[ 1 ];
         Kg = matrix[ 4 ];
         Kb = matrix[ 7 ];
      }
   private:
      double Kr = 0.2126729;
      double Kg = 0.7151521;
      double Kb = 0.072175; // The Y row of the XYZ matrix
};

class ypbpr2ycbcr : public ColorSpaceConverter {
   public:
      String InputColorSpace() const override { return YPbPr_name; }
      String OutputColorSpace() const override { return YCbCr_name; }
      void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = input[ 0 ] * 255;
            output[ 1 ] = input[ 1 ] * 255 + 128;
            output[ 2 ] = input[ 2 ] * 255 + 128;
         } while( ++input, ++output );
      }
};

class ycbcr2ypbpr : public ColorSpaceConverter {
   public:
      String InputColorSpace() const override { return YCbCr_name; }
      String OutputColorSpace() const override { return YPbPr_name; }
      void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const override {
         do {
            output[ 0 ] = input[ 0 ] / 255;
            output[ 1 ] = ( input[ 1 ] - 128 ) / 255;
            output[ 2 ] = ( input[ 2 ] - 128 ) / 255;
         } while( ++input, ++output );
      }
};

} // namespace

} // namespace dip
