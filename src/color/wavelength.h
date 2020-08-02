/*
 * DIPlib 3.0
 * This file defines the "wavelength" color space
 *
 * (c)2020, Cris Luengo.
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

// Color matching functions, CIE 1964 standard colorimetric observer.
// Data tables from http://files.cie.co.at/204.xls.

// {X[ii],Y[ii],Z[ii]} is the XYZ value for wavelength start_wl + ii * wl_step;

// We currently use linear interpolation into these tables. It is also possible
// to approximate these functions using a mixture of Gaussians (3 for the X
// component, 2 each for Y and Z). Which option is more precise? Does it matter?

constexpr std::array< dfloat, 81 > X = {{
      0.000160, 0.000662, 0.002362, 0.007242, 0.019110, 0.043400, 0.084736, 0.140638, 0.204492, 0.264737,
      0.314679, 0.357719, 0.383734, 0.386726, 0.370702, 0.342957, 0.302273, 0.254085, 0.195618, 0.132349,
      0.080507, 0.041072, 0.016172, 0.005132, 0.003816, 0.015444, 0.037465, 0.071358, 0.117749, 0.172953,
      0.236491, 0.304213, 0.376772, 0.451584, 0.529826, 0.616053, 0.705224, 0.793832, 0.878655, 0.951162,
      1.014160, 1.074300, 1.118520, 1.134300, 1.123990, 1.089100, 1.030480, 0.950740, 0.856297, 0.754930,
      0.647467, 0.535110, 0.431567, 0.343690, 0.268329, 0.204300, 0.152568, 0.112210, 0.081261, 0.057930,
      0.040851, 0.028623, 0.019941, 0.013842, 0.009577, 0.006605, 0.004553, 0.003145, 0.002175, 0.001506,
      0.001045, 0.000727, 0.000508, 0.000356, 0.000251, 0.000178, 0.000126, 0.000090, 0.000065, 0.000046,
      0.000033
}};

constexpr std::array< dfloat, 81 > Y = {{
      0.000017, 0.000072, 0.000253, 0.000769, 0.002004, 0.004509, 0.008756, 0.014456, 0.021391, 0.029497,
      0.038676, 0.049602, 0.062077, 0.074704, 0.089456, 0.106256, 0.128201, 0.152761, 0.185190, 0.219940,
      0.253589, 0.297665, 0.339133, 0.395379, 0.460777, 0.531360, 0.606741, 0.685660, 0.761757, 0.823330,
      0.875211, 0.923810, 0.961988, 0.982200, 0.991761, 0.999110, 0.997340, 0.982380, 0.955552, 0.915175,
      0.868934, 0.825623, 0.777405, 0.720353, 0.658341, 0.593878, 0.527963, 0.461834, 0.398057, 0.339554,
      0.283493, 0.228254, 0.179828, 0.140211, 0.107633, 0.081187, 0.060281, 0.044096, 0.031800, 0.022602,
      0.015905, 0.011130, 0.007749, 0.005375, 0.003718, 0.002565, 0.001768, 0.001222, 0.000846, 0.000586,
      0.000407, 0.000284, 0.000199, 0.000140, 0.000098, 0.000070, 0.000050, 0.000036, 0.000025, 0.000018,
      0.000013
}};

constexpr std::array< dfloat, 81 > Z = {{
      0.000705, 0.002928, 0.010482, 0.032344, 0.086011, 0.197120, 0.389366, 0.656760, 0.972542, 1.282500,
      1.553480, 1.798500, 1.967280, 2.027300, 1.994800, 1.900700, 1.745370, 1.554900, 1.317560, 1.030200,
      0.772125, 0.570060, 0.415254, 0.302356, 0.218502, 0.159249, 0.112044, 0.082248, 0.060709, 0.043050,
      0.030451, 0.020584, 0.013676, 0.007918, 0.003988, 0.001091, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000
}};

constexpr dfloat start_wl = 380.0;
constexpr dfloat end_wl = 780.0;
constexpr dfloat wl_step = 5.0;

static_assert( start_wl + ( X.size() - 1 ) * wl_step == end_wl, "Error in the definition of CIE data arrays" );

void ConvertWavelengthToXYZ( dfloat wavelength, dfloat& outX, dfloat& outY, dfloat& outZ ) {
   dfloat w = ( wavelength - start_wl ) / wl_step;
   dfloat d_index = std::floor( w );
   if(( d_index < 0 ) || ( d_index > static_cast< dfloat >( X.size() - 1 ))) {
      outX = outY = outZ = 0.0;
   } else {
      w -= d_index;
      dip::uint index = static_cast< dip::uint >( d_index );
      if( index + 1 >= X.size()) {
         outX = X[ index ];
         outY = Y[ index ];
         outZ = Z[ index ];
      } else {
         // Apply linear interpolation on our tables.
         outX = ( 1.0 - w ) * X[ index ] + w * X[ index + 1 ];
         outY = ( 1.0 - w ) * Y[ index ] + w * Y[ index + 1 ];
         outZ = ( 1.0 - w ) * Z[ index ] + w * Z[ index + 1 ];
      }
   }
}

constexpr char const* wavelength_name = "wavelength";

class wavelength2xyz : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return wavelength_name; }
      virtual String OutputColorSpace() const override { return XYZ_name; }
      virtual void Convert( ConstLineIterator <dfloat>& input, LineIterator <dfloat>& output ) const override {
         do {
            ConvertWavelengthToXYZ( input[ 0 ], output[ 0 ], output[ 1 ], output[ 2 ] );
         } while( ++input, ++output );
      }
};

void adjustToGammut( dfloat& R, dfloat& G, dfloat& B, dfloat Y, dfloat channel ) {
   dfloat F = Y / ( Y - channel );
   R = Y + F * ( R - Y );
   G = Y + F * ( G - Y );
   B = Y + F * ( B - Y );
}

class wavelength2rgb : public ColorSpaceConverter {
   public:
      virtual String InputColorSpace() const override { return wavelength_name; }
      virtual String OutputColorSpace() const override { return RGB_name; }
      virtual void Convert( ConstLineIterator <dfloat>& input, LineIterator <dfloat>& output ) const override {
         do {
            // Look up XYZ value for wavelength
            dfloat X, Y, Z;
            ConvertWavelengthToXYZ( input[ 0 ], X, Y, Z );
            // Convert XYZ to RGB, divide by 1.85
            dfloat R = ( X * invMatrix_[ 0 ] + Y * invMatrix_[ 3 ] + Z * invMatrix_[ 6 ] ) / 1.85; // Matrix and multiplication from xyz2rgb, but leaving out the multiplication by 255, instead dividing by 1.85.
            dfloat G = ( X * invMatrix_[ 1 ] + Y * invMatrix_[ 4 ] + Z * invMatrix_[ 7 ] ) / 1.85;
            dfloat B = ( X * invMatrix_[ 2 ] + Y * invMatrix_[ 5 ] + Z * invMatrix_[ 8 ] ) / 1.85;
            // Move the RGB value to be inside gamut
            if( R < 0 ) { adjustToGammut( R, G, B, Y, R ); }
            if( G < 0 ) { adjustToGammut( R, G, B, Y, G ); }
            if( B < 0 ) { adjustToGammut( R, G, B, Y, B ); }
            // Adjust brightness (saturation)
            R *= 1.85;
            G *= 1.85;
            B *= 1.85;
            dfloat m = std::max( std::max( R, G ), B );
            if( m > 1 ) {
               R /= m;
               G /= m;
               B /= m;
            }
            output[ 0 ] = R * 255;
            output[ 1 ] = G * 255;
            output[ 2 ] = B * 255;
         } while( ++input, ++output );
      }
      void SetWhitePoint( XYZMatrix const& matrix ) {
         Inverse( 3, matrix.data(), invMatrix_.data() );
      }
   private:
      XYZMatrix invMatrix_{{ 3.241300, -0.969197, 0.0556395, -1.53754, 1.87588, -0.204012, -0.498662, 0.0415531, 1.05715 }};
};

} // namespace

} // namespace dip
