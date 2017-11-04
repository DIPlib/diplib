#include "diplib.h"
#include "diplib/generation.h"
#include "diplib/chain_code.h"
//#include "diplib/math.h"
//#include "diplib/distance.h"
//#include "diplib/binary.h"
//#include "diplib/morphology.h"

#include "diplib/testing.h"

int main() {
   dip::Image image( { 30, 20 }, 1, dip::DT_SFLOAT );
   image.Fill( 0 );
   image.At( 15, 10 ) = 1;
   image.At( 20, 10 ) = 1;
   image.At( 10, 6 ) = 1;

   //image.Fill( 0 );
   //dip::Random random;
   //dip::BinaryNoise( image, image, random, 1.0, 0.99 ); // High-density random binary image.

   //dip::Image grey( { 1600, 2201 }, 1, dip::DT_UINT16 );
   //dip::Random random;
   //dip::UniformNoise( grey, grey, random, 0, 10000 );
   //dip::FillRadiusCoordinate( grey );

   //dip::Image gt = dip::EuclideanDistanceTransform( image, "background", "brute force" );
   //dip::Image result = dip::EuclideanDistanceTransform( image, "background", "fast" );
   //result -= gt;
   //dip::Image result = dip::VectorDistanceTransform( image, "background", "fast" );
   //result = dip::Norm( result ) - gt;

   //dip::Image result = dip::GreyWeightedDistanceTransform( grey, image, { "chamfer", 1 } );

   //dip::Image result = dip::EuclideanSkeleton( image );

   //dip::testing::PrintPixelValues< dip::bin, 1 >( image );
   //dip::Image mask = dip::BinaryDilation( image, -1, 7 );
   //dip::testing::PrintPixelValues< dip::bin, 1 >( mask );
   //dip::BinaryPropagation( mask, mask, mask, -1, 3 );
   //dip::testing::PrintPixelValues< dip::bin, 1 >( mask );

   //dip::Polygon points;
   //points.vertices.resize( 5 );
   //points.vertices[ 0 ] = { 3, 3 };
   //points.vertices[ 1 ] = { 25, 3  };
   //points.vertices[ 2 ] = { 20, 15  };
   //points.vertices[ 3 ] = { 5, 18  };
   //points.vertices[ 4 ] = { 5, 8  };

   //dip::testing::PrintPixelValues< dip::uint8, 1 >( image );
   //dip::DrawPolygon2D( image, points, {2}, "closed" );
   //dip::testing::PrintPixelValues< dip::uint8, 1 >( image );
   //dip::DrawPolygon2D( image, points, {4}, "filled" );
   //dip::testing::PrintPixelValues< dip::uint8, 1 >( image );

   dip::testing::PrintPixelValues< dip::sfloat, 4 >( image );
   //dip::DrawBandlimitedBox( image, {7.0}, {20.1,10.9}, {2}, "filled", 1.0 );
   //dip::DrawBandlimitedBox( image, {7.0}, {10.5,6.5}, {4}, "empty", 1.0 );
   dip::DrawBandlimitedLine( image, {5,17}, {25,5}, {4}, 2.0 );
   dip::testing::PrintPixelValues< dip::sfloat, 4 >( image );

   return 0;
}
