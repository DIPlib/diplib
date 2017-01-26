/*
 * Testing MEX-file functionality
 *
 */

#define DOCTEST_CONFIG_IMPLEMENT
#include "dip_matlab_interface.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, mxArray const* prhs[] ) {
   dml::streambuf streambuf;
   try {

      if( nrhs > 0 ) {
         /*
         mexPrintf( "Obtaining input image img_in0\n" );
         dip::Image const img_in0 = dml::GetImage( prhs[ 0 ] );
         std::cout << img_in0;
         mexPrintf( "Exiting scope\n" );
         */

         /*
         dip::UnsignedArray bla = dml::GetUnsignedArray( prhs[ 0 ] );
         for( auto const& b : bla ) {
            std::cout << b << ",";
         }
         std::cout << std::endl;
         */

         /*
         dip::StringArray bla = dml::GetStringArray( prhs[ 0 ] );
         for( auto const& b : bla ) {
            std::cout << b << ",";
         }
         std::cout << std::endl;
         */

         //std::cout << dml::GetComplex( prhs[ 0 ] ) << std::endl;

         //std::cout << dip::DataType( dml::GetString( prhs[ 0 ] )) << std::endl;

         dml::GetCoordinateArray( prhs[ 0 ] );

         /*
         auto bla = dml::GetRangeArray( prhs[ 0 ] );
         for( auto const& b : bla ) {
            std::cout << "Range: " << b.start << "," << b.stop << "," << b.step << std::endl;
         }
         */

         //plhs[ 0 ] = dml::GetArray( dip::IntegerArray{5,2,6,3,1} );
         plhs[ 0 ] = dml::GetArray( "bla" );

      } else {

         std::cout << "Creating output image img_out0\n";
         dml::MatlabInterface mi;
         dip::Image img_out0 = mi.NewImage();
         img_out0.SetSizes( { 3, 5 } );
         std::cout << img_out0;
         img_out0.Forge();
         std::cout << img_out0;

         std::cout << "Reallocating output image img_out0\n";
         img_out0.Strip();
         img_out0.Forge();

         std::cout << "Copying output image img_out0 to img_out1\n";
         dip::Image img_out1;
         img_out1 = img_out0;
         std::cout << "Reallocating output image img_out1\n";
         img_out1.Strip();
         img_out1.SetSizes( { 2, 3 } );
         img_out1.Forge();

         std::cout << "The two output images:\n";
         std::cout << img_out0;
         std::cout << img_out1;

         std::cout << "Getting the array for img_out0\n";
         plhs[ 0 ] = mi.GetArray( img_out0 );

         std::cout << "Exiting scope\n";

      }

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
