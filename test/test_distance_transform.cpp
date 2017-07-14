#undef DIP__ENABLE_DOCTEST

#include "diplib.h"
#include "diplib/generation.h"
#include "diplib/distance.h"
#include "diplib/math.h"

#ifdef DIP__HAS_GLFW
   #include <diplib/viewer/glfw.h>
   using ViewerManager = GLFWManager;
#else
   #include <diplib/viewer/glut.h>
   using ViewerManager = GLUTManager;
#endif

#include <diplib/viewer/image.h>
#include <diplib/viewer/slice.h>

int main( void ) {
   ViewerManager manager;

   dip::Image binary( { 200, 200 }, 1, dip::DT_BIN );
   binary.Fill( 0 );
   dip::Random random;
   dip::BinaryNoise( binary, binary, random, 1.0, 0.999 ); // High-density random binary image.

   dip::Image grey( binary.Sizes(), 1, dip::DT_SFLOAT );
   dip::FillRadiusCoordinate( grey );

   //dip::Image gt = dip::EuclideanDistanceTransform( binary, "background", "brute force" );
   //dip::Image result = dip::EuclideanDistanceTransform( binary, "background", "fast" );
   //result -= gt;
   //dip::Image result = dip::VectorDistanceTransform( binary, "background", "fast" );
   //result = dip::Norm( result ) - gt;

   dip::Image result = dip::GreyWeightedDistanceTransform( grey, binary, { "chamfer", 1 } );

   binary.ExpandDimensionality( 3 ).Convert( dip::DT_UINT8 );
   manager.createWindow( WindowPtr( new SliceViewer( binary )));
   grey.ExpandDimensionality( 3 );
   manager.createWindow( WindowPtr( new SliceViewer( grey )));
   result.ExpandDimensionality( 3 );
   manager.createWindow( WindowPtr( new SliceViewer( result )));
   while( manager.activeWindows()) {
      // Only necessary for GLFW
      manager.processEvents();
      usleep( 10 );
   }

   return 0;
}
