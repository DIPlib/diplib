/*
 * This program shows how to apply various geometric transformations to an image.
 * It displays the result using dip::viewer::ShowSimple.
 */

#include <vector>
#include "diplib.h"
#include "dipviewer.h"
#include "diplib/simple_file_io.h"
#include "diplib/geometry.h"
#include "diplib/generation.h"

int main() {
   // A color input image
   dip::Image input = dip::ImageRead( DIP_EXAMPLES_DIR "/DIP.tif" );

   // Mirroring and 90 degree rotations
   // These transformations are trivial, data is not copied!
   dip::Image res1 = input;
   res1.Mirror( { true, false } );  // mirror first dimension
   dip::Image res2 = input;
   res2.Rotation90( 1 );            // rotate by 90 degrees once

   // Resampling
   dip::Image res3 = dip::Resampling( input, { 0.9, 1.1 }, { 0.5, 3.5 } ); // Scale by 0.9 along x and 1.1 along y, and translate a bit too.

   // Rotation
   dip::Image res4 = dip::Rotation2D( input, dip::pi / 8.0 );  // Rotate by pi/8 radian == 22.5 degrees

   // Affine transform
   // The affine transform matrix has the form:
   //         ⎡ matrix[0]  matrix[2]  matrix[4] ⎤
   //     T = ⎢ matrix[1]  matrix[3]  matrix[5] ⎥
   //         ⎣    0          0          1      ⎦
   dip::FloatArray matrix = { std::cos( dip::pi / 6.0 ), std::sin( dip::pi / 6.0 ) * 1.5,
                             -std::sin( dip::pi / 6.0 ), std::cos( dip::pi / 6.0 ),
                              15.2,                     -23.7 }; // Because of storage order, these values are arranged like the transposed matrix.
   dip::Image res5 = dip::AffineTransform( input, matrix );

   // Thin plate spline warping
   // 4 control points to move each corner of the image to a different corner, rotating
   // the image by 90 degrees. (This is expensive, it's cheaper to use dip::Image::Rotation90()!)
   dip::FloatCoordinateArray source = {
         { 0,   0 },
         { 255, 0 },
         { 0,   255 },
         { 255, 255 }
   };
   dip::FloatCoordinateArray destination = {
         { 255, 0 },
         { 255, 255 },
         { 0,   0 },
         { 0,   255 }
   };
   dip::Image res6 = WarpControlPoints( input, source, destination );

   // Thin plate spline warping
   // A grid of 4x4 control points, each shifted randomly
   source = dip::FloatCoordinateArray{
         { 31,  31 }, { 95,  31 }, { 159,  31 }, { 223,  31 },
         { 31,  95 }, { 95,  95 }, { 159,  95 }, { 223,  95 },
         { 31, 159 }, { 95, 159 }, { 159, 159 }, { 223, 159 },
         { 31, 223 }, { 95, 223 }, { 159, 223 }, { 223, 223 }
   };
   destination = source;
   dip::Random rng;
   dip::UniformRandomGenerator rand( rng );
   for( dip::uint ii = 0; ii < source.size(); ++ii ) {
      destination[ ii ][ 0 ] += rand( -20, 20 );
      destination[ ii ][ 1 ] += rand( -20, 20 );
   }
   dip::Image res7 = WarpControlPoints( input, source, destination, 0.1 );

   // Display
   dip::viewer::ShowSimple( input, "input image" );
   dip::viewer::ShowSimple( res1, "mirrored image" );
   dip::viewer::ShowSimple( res2, "90 degree rotation" );
   dip::viewer::ShowSimple( res3, "resampled image" );
   dip::viewer::ShowSimple( res4, "rotated image" );
   dip::viewer::ShowSimple( res5, "affine transformed image" );
   dip::viewer::ShowSimple( res6, "warped image (90 degree rotation)" );
   dip::viewer::ShowSimple( res7, "warped image" );
   dip::viewer::Spin();
}
