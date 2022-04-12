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


std::pair< ValueInformationArray, FloatArray > MuInformation( dip::uint nD, PixelSize const& pixelSize ) {
   dip::uint nOut = nD * ( nD + 1 ) / 2;
   ValueInformationArray out( nOut );
   FloatArray scales( nOut );
   for( dip::uint ii = 0; ii < nD; ++ii ) {
      PhysicalQuantity pq1 = pixelSize[ ii ];
      if( !pq1.IsPhysical()) {
         pq1 = PhysicalQuantity::Pixel();
      }
      scales[ ii ] = pq1.magnitude * pq1.magnitude;
      out[ ii ].units = pq1.units * pq1.units;
      out[ ii ].name = String( "Mu_" ) + std::to_string( ii ) + '_' + std::to_string( ii );
   }
   dip::uint kk = nD;
   for( dip::uint ii = 1; ii < nD; ++ii ) {
      for( dip::uint jj = 0; jj < ii; ++jj ) {
         PhysicalQuantity pq1 = pixelSize[ ii ];
         if( !pq1.IsPhysical()) {
            pq1 = PhysicalQuantity::Pixel();
         }
         PhysicalQuantity pq2 = pixelSize[ jj ];
         if( !pq2.IsPhysical()) {
            pq2 = PhysicalQuantity::Pixel();
         }
         scales[ kk ] = pq1.magnitude * pq2.magnitude;
         out[ kk ].units = pq1.units * pq2.units;
         out[ kk ].name = String( "Mu_" ) + std::to_string( ii ) + '_' + std::to_string( jj );
         ++kk;
      }
   }
   return std::make_pair( out, scales );
}

std::pair< Units, FloatArray > MuEigenDecompositionUnitsAndScaling( dip::uint nD, PixelSize const& pixelSize ) {
   DIP_THROW_IF(( nD < 2 ) || ( nD > 3 ), E::DIMENSIONALITY_NOT_SUPPORTED );
   dip::uint nOut = nD * ( nD + 1 ) / 2;
   FloatArray scales( nOut );
   scales[ 0 ] = pixelSize[ 0 ].magnitude;
   Units units = pixelSize[ 0 ].units;
   bool sameUnits = units.IsPhysical();
   if( sameUnits ) {
      for( dip::uint ii = 1; ii < nD; ++ii ) {
         scales[ ii ] = pixelSize[ ii ].magnitude;
         if( pixelSize[ ii ].units != units ) {
            // This tests false if the SI prefix differs. This is intentional, as the Mu values will be given
            // with different SI prefixes and we'd need complex logic here to fix it.
            sameUnits = false;
            break;
         }
      }
   }
   if( sameUnits ) {
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
   } else {
      units = Units::Pixel();
      scales.fill( 1 );
   }
   return std::make_pair( units, scales );
}


} // namespace feature
} // namespace dip


#endif //DIP_FEATURE_COMMON_STUFF_H
