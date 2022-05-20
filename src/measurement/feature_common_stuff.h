/*
 * (c)2022, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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


#ifndef DIP_FEATURE_COMMON_STUFF_H
#define DIP_FEATURE_COMMON_STUFF_H


namespace dip {
namespace Feature {


inline std::pair< Units, FloatArray > MuUnitsAndScaling( dip::uint nD, PixelSize const& pixelSize ) {
   DIP_THROW_IF(( nD < 2 ) || ( nD > 3 ), E::DIMENSIONALITY_NOT_SUPPORTED );
   dip::uint nOut = nD * ( nD + 1 ) / 2;
   FloatArray scales( nOut, 1 );
   Units units = Units::Pixel();
   if( pixelSize.SameUnits() ) {
      units = pixelSize[ 0 ].units;
      for( dip::uint ii = 0; ii < nD; ++ii ) {
         scales[ ii ] = pixelSize[ ii ].magnitude;
      }
      dip::uint kk = nD;
      for( dip::uint ii = 1; ii < nD; ++ii ) {
         for( dip::uint jj = 0; jj < ii; ++jj ) {
            scales[ kk ] = scales[ ii ] * scales[ jj ];
            ++kk;
         }
      }
      for( dip::uint ii = 0; ii < nD; ++ii ) {
         scales[ ii ] *= scales[ ii ];
      }
   }
   units.Power( 2 );
   return std::make_pair( units, scales );
}


inline std::pair< ValueInformationArray, FloatArray > MuInformation( dip::uint nD, PixelSize const& pixelSize ) {
   Units units;
   FloatArray scales;
   std::tie( units, scales ) = MuUnitsAndScaling( nD, pixelSize );
   dip::uint nOut = scales.size();
   ValueInformationArray out( nOut );
   for( dip::uint ii = 0; ii < nD; ++ii ) {
      out[ ii ].units = units;
      out[ ii ].name = String( "Mu_" ) + std::to_string( ii ) + '_' + std::to_string( ii );
   }
   dip::uint kk = nD;
   for( dip::uint ii = 1; ii < nD; ++ii ) {
      for( dip::uint jj = 0; jj < ii; ++jj ) {
         out[ kk ].units = units;
         out[ kk ].name = String( "Mu_" ) + std::to_string( ii ) + '_' + std::to_string( jj );
         ++kk;
      }
   }
   return std::make_pair( out, scales );
}


inline ValueInformationArray MuSqrtEigenValueInformation( dip::uint nD, PixelSize const& pixelSize ) {
   Units units = Units::Pixel();
   if( pixelSize.SameUnits() ) {
      units = pixelSize[ 0 ].units;
   }
   ValueInformationArray out( nD );
   for( dip::uint ii = 0; ii < nD; ++ii ) {
      out[ ii ].units = units;
      out[ ii ].name = String( "axis" ) + std::to_string( ii );
   }
   return out;
}


inline ValueInformationArray MuEigenValueInformation( dip::uint nD, PixelSize const& pixelSize ) {
   Units units = Units::Pixel();
   if( pixelSize.SameUnits() ) {
      units = pixelSize[ 0 ].units;
   }
   units.Power( 2 );
   ValueInformationArray out( nD );
   for( dip::uint ii = 0; ii < nD; ++ii ) {
      out[ ii ].units = units;
      out[ ii ].name = String( "lambda_" ) + std::to_string( ii );
   }
   return out;
}


inline ValueInformationArray MuEigenVectorInformation( dip::uint nD, PixelSize const& pixelSize ) {
   Units units = Units::Pixel();
   if( pixelSize.SameUnits() ) {
      units = pixelSize[ 0 ].units;
   }
   ValueInformationArray out( nD * nD );
   for( dip::uint ii = 0; ii < nD; ++ii ) {
      for( dip::uint jj = 0; jj < nD; ++jj ) {
         out[ ii * nD + jj ].units = units;
         out[ ii * nD + jj ].name = String( "v" ) + std::to_string( ii ) + '_' + std::to_string( jj );
      }
   }
   return out;
}


inline dfloat ReverseSizeScale( dip::uint nD, PixelSize const& pixelSize ) {
   // This function is used in some of the Composite features.
   // If pixels are anisotropic, then a "Size" or "Area" feature will still be in physical units,
   // but "Feret" or "Perimeter" or "SurfaceArea" will not. In this case, we need to scale the "Size"
   // feature back to pixels before combining it with the other features.
   PhysicalQuantity unitArea = pixelSize.UnitSize( nD );
   if( unitArea.IsPhysical() && !pixelSize.IsIsotropic() ) {
      return 1.0 / unitArea.magnitude;
   }
   return 1.0;
}


} // namespace feature
} // namespace dip


#endif //DIP_FEATURE_COMMON_STUFF_H
