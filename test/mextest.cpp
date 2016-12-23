/*
 * Testing MEX-file functionality
 *
 */

#define DOCTEST_CONFIG_IMPLEMENT
#include "dip_matlab_interface.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, mxArray const* prhs[] ) {
   dml::streambuf streambuf;
   try {

      mexPrintf( "Creating output image img_out0\n" );
      dml::MatlabInterface mi;
      dip::Image img_out0 = mi.NewImage();
      img_out0.SetSizes( { 3, 5 } );
      std::cout << img_out0;
      img_out0.Forge();
      std::cout << img_out0;

      mexPrintf( "Reallocating output image img_out0\n" );
      img_out0.Strip();
      img_out0.Forge();

      mexPrintf( "Copying output image img_out0 to img_out1\n" );
      dip::Image img_out1; img_out1 = img_out0;
      mexPrintf( "Reallocating output image img_out1\n" );
      img_out1.Strip();
      img_out1.SetSizes( { 2, 3 } );
      img_out1.Forge();

      if( nrhs > 0 ) {
         mexPrintf( "Obtaining input image img_in0\n" );
         dip::Image const img_in0 = dml::GetImage( prhs[ 0 ] );
         std::cout << img_in0;
         mexPrintf( "Exiting scope\n" );
      }

      mexPrintf( "The two output images:\n" );
      std::cout << img_out0;
      std::cout << img_out1;

      mexPrintf( "Getting the array for img_out0\n" );
      plhs[ 0 ] = mi.GetArray( img_out0 );

      mexPrintf( "Exiting scope\n" );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
