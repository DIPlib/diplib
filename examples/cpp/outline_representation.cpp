/*
 * This program shows how to use dip::ChainCode::Polygon and dip::RefinePolygon.
 * It measures accuracy (bias, mean error) and precision (standard deviation of the error) of different
 * methods to represent an outline as a polygon.
 */

#include "diplib.h"
#include "diplib/polygon.h"
#include "diplib/chain_code.h"
#include "diplib/generation.h"
#include "diplib/linear.h"
#include "diplib/regions.h"

int main() {
   constexpr dip::dfloat threshold = 0.5;
   dip::Random rng;
   dip::UniformRandomGenerator unif( rng );
   constexpr dip::uint n = 1000;
   dip::String interpolation[ 2 ] = { "linear", "3-cubic" };

   for( bool square : { false, true } ) {
      std::cout << "- Shape: " << ( square ? "square" : "disk" ) << '\n';
      for( dip::dfloat sigma : { 1.0, 3.0 } ) {
         std::cout << "  - Sigma = " << sigma << '\n';
         dip::StatisticsAccumulator area_error[ 8 ];
         dip::StatisticsAccumulator perimeter_error[ 8 ];
         dip::StatisticsAccumulator radius_cv[ 8 ];
         for( dip::uint ii = 0; ii < n; ++ii ) {
            dip::Image img = dip::Image( { 256, 256 }, 1 );
            img.Fill( 0 );
            dip::FloatArray center = { 128.0 + unif( 0, 1 ), 128.0 + unif( 0, 1 ) };
            double diameter = 200 + unif( 0, 1 );
            double expected_area{};
            double expected_perimeter{};
            if( square ) { // diameter == side of square
               dip::DrawBandlimitedBox( img, { diameter }, center, { 2 * threshold }, "filled", sigma );
               expected_area = diameter * diameter;
               expected_perimeter = 4 * diameter;
            } else {
               dip::DrawBandlimitedBall( img, diameter, center, { 2 * threshold }, "filled", sigma );
               expected_area = dip::pi / 4 * diameter * diameter;
               expected_perimeter = dip::pi * diameter;
            }
            dip::Image grad = dip::Gradient( img );
            dip::Image labels = dip::Label( img > threshold );
            dip::ChainCodeArray ccs = dip::GetImageChainCodes( labels ); // Should only be one chain code
            area_error[ 0 ].Push( ccs[ 0 ].Area() - expected_area );
            perimeter_error[ 0 ].Push( ccs[ 0 ].Length() - expected_perimeter );
            dip::Polygon p = ccs[ 0 ].Polygon();
            area_error[ 1 ].Push( p.Area() - expected_area ); // Bias is half a pixel
            perimeter_error[ 1 ].Push( p.Perimeter() - expected_perimeter );
            auto radiusStats = p.RadiusStatistics();
            radius_cv[ 1 ].Push( radiusStats.StandardDeviation() / radiusStats.Mean() );
            for( dip::uint jj = 0; jj < 2; ++jj ) {
               p = ccs[ 0 ].Polygon( img, threshold, interpolation[ jj ] );
               area_error[ jj * 3 + 2 ].Push( p.Area() - expected_area );
               perimeter_error[ jj * 3 + 2 ].Push( p.Perimeter() - expected_perimeter );
               radiusStats = p.RadiusStatistics();
               radius_cv[ jj * 3 + 2 ].Push( radiusStats.StandardDeviation() / radiusStats.Mean() );
               p = ccs[ 0 ].Polygon();
               //p.Smooth( 2 );
               if( dip::RefinePolygon( p, img, grad, threshold, interpolation[ jj ] ) != 0 ) {
                  std::cout << "Error refining polygon with shape = " << ( square ? "square" : "disk" )
                            << ", sigma = " << sigma
                            << ", interpolation = " << interpolation[ jj ] << '\n';
               }
               area_error[ jj * 3 + 3 ].Push( p.Area() - expected_area );
               perimeter_error[ jj * 3 + 3 ].Push( p.Perimeter() - expected_perimeter );
               radiusStats = p.RadiusStatistics();
               radius_cv[ jj * 3 + 3 ].Push( radiusStats.StandardDeviation() / radiusStats.Mean() );
               p = ccs[ 0 ].Polygon();
               p.Augment( 0.3 );
               if( dip::RefinePolygon( p, img, grad, threshold, interpolation[ jj ] ) != 0 ) {
                  std::cout << "Error refining augmented polygon with shape = " << ( square ? "square" : "disk" )
                            << ", sigma = " << sigma
                            << ", interpolation = " << interpolation[ jj ] << '\n';
               }
               area_error[ jj * 3 + 4 ].Push( p.Area() - expected_area );
               perimeter_error[ jj * 3 + 4 ].Push( p.Perimeter() - expected_perimeter );
               radiusStats = p.RadiusStatistics();
               radius_cv[ jj * 3 + 4 ].Push( radiusStats.StandardDeviation() / radiusStats.Mean() );
            }
         }
         std::cout << "    - Area error:\n";
         std::cout << "      - Chain code:                         mean = " << area_error[ 0 ].Mean() << ", std dev = " << area_error[ 0 ].StandardDeviation() << '\n';
         std::cout << "      - Binary:                             mean = " << area_error[ 1 ].Mean() << ", std dev = " << area_error[ 1 ].StandardDeviation() << '\n';
         for( dip::uint jj = 0; jj < 2; ++jj ) {
            std::cout << "      - Polygon('" << interpolation[ jj ] << "'):                  mean = " << area_error[ jj * 3 + 2 ].Mean() << ", std dev = " << area_error[ jj * 3 + 2 ].StandardDeviation() << '\n';
            std::cout << "      - RefinePolygon('" << interpolation[ jj ] << "'):            mean = " << area_error[ jj * 3 + 3 ].Mean() << ", std dev = " << area_error[ jj * 3 + 3 ].StandardDeviation() << '\n';
            std::cout << "      - RefinePolygon('" << interpolation[ jj ] << "'), augmented: mean = " << area_error[ jj * 3 + 4 ].Mean() << ", std dev = " << area_error[ jj * 3 + 4 ].StandardDeviation() << '\n';
         }
         std::cout << "    - Perimeter error:\n";
         std::cout << "      - Chain code:                         mean = " << perimeter_error[ 0 ].Mean() << ", std dev = " << perimeter_error[ 0 ].StandardDeviation() << '\n';
         std::cout << "      - Binary:                             mean = " << perimeter_error[ 1 ].Mean() << ", std dev = " << perimeter_error[ 1 ].StandardDeviation() << '\n';
         for( dip::uint jj = 0; jj < 2; ++jj ) {
            std::cout << "      - Polygon('" << interpolation[ jj ] << "'):                  mean = " << perimeter_error[ jj * 3 + 2 ].Mean() << ", std dev = " << perimeter_error[ jj * 3 + 2 ].StandardDeviation() << '\n';
            std::cout << "      - RefinePolygon('" << interpolation[ jj ] << "'):            mean = " << perimeter_error[ jj * 3 + 3 ].Mean() << ", std dev = " << perimeter_error[ jj * 3 + 3 ].StandardDeviation() << '\n';
            std::cout << "      - RefinePolygon('" << interpolation[ jj ] << "'), augmented: mean = " << perimeter_error[ jj * 3 + 4 ].Mean() << ", std dev = " << perimeter_error[ jj * 3 + 4 ].StandardDeviation() << '\n';
         }
         std::cout << "    - Radius CV:\n";
         std::cout << "      - Binary:                             mean = " << radius_cv[ 1 ].Mean() << ", std dev = " << radius_cv[ 1 ].StandardDeviation() << '\n';
         for( dip::uint jj = 0; jj < 2; ++jj ) {
            std::cout << "      - Polygon('" << interpolation[ jj ] << "'):                  mean = " << radius_cv[ jj * 3 + 2 ].Mean() << ", std dev = " << radius_cv[ jj * 3 + 2 ].StandardDeviation() << '\n';
            std::cout << "      - RefinePolygon('" << interpolation[ jj ] << "'):            mean = " << radius_cv[ jj * 3 + 3 ].Mean() << ", std dev = " << radius_cv[ jj * 3 + 3 ].StandardDeviation() << '\n';
            std::cout << "      - RefinePolygon('" << interpolation[ jj ] << "'), augmented: mean = " << radius_cv[ jj * 3 + 4 ].Mean() << ", std dev = " << radius_cv[ jj * 3 + 4 ].StandardDeviation() << '\n';
         }
      }
   }
}
