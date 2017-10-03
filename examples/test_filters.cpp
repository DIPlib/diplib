#undef DIP__ENABLE_DOCTEST

#include "diplib.h"
#include "diplib/generation.h"
#include "diplib/math.h"
//#include "diplib/distance.h"
//#include "diplib/binary.h"
#include "diplib/morphology.h"

#include "diplib/testing.h"

int main() {
   dip::Image binary( { 30, 20 }, 1, dip::DT_BIN );
   binary.Fill( 0 );
   binary.At( 15, 10 ) = 1;
   binary.At( 20, 10 ) = 1;
   binary.At( 10, 6 ) = 1;

   //binary.Fill( 0 );
   //dip::Random random;
   //dip::BinaryNoise( binary, binary, random, 1.0, 0.99 ); // High-density random binary image.

   //dip::Image grey( binary.Sizes(), 1, dip::DT_SFLOAT );
   //dip::FillRadiusCoordinate( grey );

   //dip::Image gt = dip::EuclideanDistanceTransform( binary, "background", "brute force" );
   //dip::Image result = dip::EuclideanDistanceTransform( binary, "background", "fast" );
   //result -= gt;
   //dip::Image result = dip::VectorDistanceTransform( binary, "background", "fast" );
   //result = dip::Norm( result ) - gt;

   //dip::Image result = dip::GreyWeightedDistanceTransform( grey, binary, { "chamfer", 1 } );

   //dip::Image result = dip::EuclideanSkeleton( binary );

   dip::Image result = dip::Dilation( binary, dip::StructuringElement( { 10, 4 }, "line" ));

   dip::testing::PrintPixelValues<dip::bin,1>( binary );
   dip::testing::PrintPixelValues<dip::bin,1>( result );

   return 0;
}
