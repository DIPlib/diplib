/*
 * Testing assorted DIPlib functionality
 *
 */

#define DOCTEST_CONFIG_IMPLEMENT
#include "dip_matlab_interface.h"
#include "diplib/pixel_table.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {
      dml::streambuf streambuf;
      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();
      if( nrhs == 0 ) {
         dip::PixelTable pt( "elliptic", dip::FloatArray{ 10.1, 12.7, 5.3 }, 0 );
         std::cout << "Origin: " << pt.Origin()[ 0 ] << "," << pt.Origin()[ 1 ] << "," << pt.Origin()[ 2 ] << std::endl;
         std::cout << "Sizes: " << pt.Sizes()[ 0 ] << "," << pt.Sizes()[ 1 ] << "," << pt.Sizes()[ 2 ] << std::endl;
         std::cout << "Runs: " << pt.Runs().size() << std::endl;
         pt.AsImage( out );
      }
      else {
         dip::Image const img = dml::GetImage( prhs[ 0 ] );
         dip::PixelTable pt( img, {}, 0 );
         std::cout << "Origin: " << pt.Origin()[ 0 ] << "," << pt.Origin()[ 1 ] << "," << pt.Origin()[ 2 ] << std::endl;
         std::cout << "Sizes: " << pt.Sizes()[ 0 ] << "," << pt.Sizes()[ 1 ] << "," << pt.Sizes()[ 2 ] << std::endl;
         std::cout << "Runs: " << pt.Runs().size() << std::endl;
         pt.AsImage( out );
      }
      plhs[ 0 ] = mi.GetArray( out );
   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
