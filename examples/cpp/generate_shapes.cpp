/*
 * This is a program that demonstrates how to generate various shapes. We show off various functionality
 * in `generation.h`.
 */

#include "diplib.h"
#include "diplib/file_io.h"
#include "diplib/generation.h"
#include "diplib/geometry.h"
#include "diplib/math.h"

int main() {

   // === 2D ===

   dip::Image img2d( { 256, 256 }, 1, dip::DT_UINT8 );
   img2d.Protect(); // This prevents GaussianEdgeClip and similar from re-forging to a different data type.

   // -- direct drawing using DrawBandlimitedBall/DrawBandlimitedBox/DrawBandlimitedLine --

   // A disk
   img2d.Fill( 0 );
   dip::DrawBandlimitedBall( img2d, 128.0, img2d.GetCenter(), { 255 } );
   dip::ImageWriteTIFF( img2d, "disk" );

   // A rectangle
   img2d.Fill( 0 );
   dip::DrawBandlimitedBox( img2d, { 128.0, 64.0 }, img2d.GetCenter(), { 255 } );
   dip::ImageWriteTIFF( img2d, "rectangle" );

   // A line
   img2d.Fill( 0 );
   dip::DrawBandlimitedLine( img2d, {  64.0,  64.0 }, { 128.0,  64.0 }, { 255 * std::sqrt( 2 * dip::pi ) } );
   dip::DrawBandlimitedLine( img2d, { 130.0,  66.0 }, { 130.0, 130.0 }, { 255 * std::sqrt( 2 * dip::pi ) } );
   dip::DrawBandlimitedLine( img2d, { 128.0, 128.0 }, {  66.0,  66.0 }, { 255 * std::sqrt( 2 * dip::pi ) } );
   dip::ImageWriteTIFF( img2d, "lines" );

   // -- using GaussianEdgeClip/GaussianLineClip on a generated distance map --

   // A rotated coordinate system
   auto coords = dip::CreateCoordinates( img2d.Sizes() );
   coords = dip::RotationMatrix2D( dip::pi / 8.0 ) * coords;

   // A rotated rectangle
   auto distance = dip::Supremum( dip::Abs( coords[ 0 ] ) - 64.0, dip::Abs( coords[ 1 ] ) - 32.0 );
   dip::GaussianEdgeClip( -distance, img2d, { 255 } );
   dip::ImageWriteTIFF( img2d, "rotated_rectangle" );

   // A rotated diamond
   distance = dip::Abs( coords[ 0 ] ) + 0.5 * dip::Abs( coords[ 1 ] ) - 64.0;
   dip::GaussianLineClip( -distance, img2d, { 255 * std::sqrt( 2 * dip::pi ) } );
   dip::ImageWriteTIFF( img2d, "rotated_diamond" );

   // === 3D ===

   dip::Image img3d( { 80, 80, 40 }, 1, dip::DT_UINT8 );
   img3d.Protect(); // This prevents GaussianEdgeClip and similar from re-forging to a different data type.

   // -- direct drawing using DrawBandlimitedBall/DrawBandlimitedBox/DrawBandlimitedLine --

   // A ball
   img3d.Fill( 0 );
   dip::DrawBandlimitedBall( img3d, 25.0, img3d.GetCenter(), { 255 } );
   dip::ImageWriteICS( img3d, "ball" );

   // A box shell
   img3d.Fill( 0 );
   dip::DrawBandlimitedBox( img3d, { 60.0, 40.0, 20.0 }, img3d.GetCenter(), { 255 }, "empty" );
   dip::ImageWriteICS( img3d, "box_shell" );

   // A line
   img3d.Fill( 0 );
   dip::DrawBandlimitedLine( img3d, { 10.0, 30.0,  5.0 }, { 70.0, 30.0,  5.0 }, { 255 * std::sqrt( 2 * dip::pi ) } );
   dip::DrawBandlimitedLine( img3d, { 70.0, 30.0,  5.0 }, { 70.0, 30.0, 35.0 }, { 255 * std::sqrt( 2 * dip::pi ) } );
   dip::DrawBandlimitedLine( img3d, { 70.0, 30.0, 35.0 }, { 70.0, 50.0, 35.0 }, { 255 * std::sqrt( 2 * dip::pi ) } );
   dip::DrawBandlimitedLine( img3d, { 70.0, 50.0, 35.0 }, { 10.0, 50.0, 35.0 }, { 255 * std::sqrt( 2 * dip::pi ) } );
   dip::ImageWriteICS( img3d, "lines3d" );

   // -- using GaussianEdgeClip/GaussianLineClip on a generated distance map --

   // A rotated coordinate system
   coords = dip::CreateCoordinates( img3d.Sizes() );
   coords = dip::RotationMatrix3D( { 1.0, 0.0, 0.0 }, dip::pi / 6.0 ) * coords;

   // A rotated cylinder
   distance = dip::Supremum( dip::Norm( coords[ dip::Range( 0, 1 ) ] ) - 20, dip::Abs( coords[ 2 ] ) - 10 );
   dip::GaussianEdgeClip( -distance, img3d, { 255 } );
   dip::ImageWriteICS( img3d, "cylinder" );
}
