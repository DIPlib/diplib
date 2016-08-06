/*
 * Testing MEX-file functionality
 *
 * Compile with:
 *   mex -setup C++
 *   mex -largeArrayDims mextest.cpp -I./include/ ./src/image.cpp ./src/error.cpp ./src/datatypes.cpp CXXFLAGS='$CXXFLAGS -std=c++11'
 */

#include "dip_matlab.h"
#include <iostream>

void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
   if( nrhs != 2 ) {
      mexErrMsgTxt( "Two input images expected" );
   }

   try {

      dml::MatlabInterface mi;

      //mexPrintf( "About to get input images:\n" );
      dip::Image in1 = dml::GetImage( prhs[0] );
      //std::cout << in1 << std::endl;

      dip::Image in2 = dml::GetImage( prhs[1] );
      //std::cout << in2 << std::endl;

      //mexPrintf( "About to create output images:\n" );
      dip::Image out = mi.NewImage();
      //out.CopyProperties( in1 );

      //mexPrintf( "About to call Forge() on output image:\n" );
      //out.Forge();
      //out.Copy( in1.At( 0 ) );
      //out.Copy( in1 );
      //out = out.At( 0 );

      //mexPrintf( "About to call the DIPlib function:\n" );
      //out.Set(56.0e12);
      dip::Add( in1, in2, out, dip::DataType::SuggestArithmetic( in1, in1 ) );

      //std::cout << out << std::endl;

      //mexPrintf( "About to extract mxArray from output image:\n" );
      plhs[0] = mi.GetArray( out );

      //mexPrintf( "End of scope for interface object\n" );

   } catch( const dip::Error& e ) {

      mexErrMsgTxt( e.what() );

   }
}
