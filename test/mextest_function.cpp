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

      dml::MATLAB_Interface mi;

      mexPrintf( "About to get input images:\n" );
      dip::Image in1 = dml::GetImage( prhs[0] );
      dip::Image in2 = dml::GetImage( prhs[1] );

      dip::Image out( &mi );
      out.CopyProperties( in1 );
      mexPrintf( "About to call Forge() on output image:\n" );
      out.Forge();

      out.Set(56.0e12);
      //dip::Arithmetic( in1, in2, out, "+", dip::DataTypeSuggest_Arithmetic( in1, in2 ) );

      std::cout << out;

      mexPrintf( "About to extract mxArray from output image:\n" );
      plhs[0] = mi.GetArray( out );

      mexPrintf( "End of scope for interface object\n" );

   } catch( const dip::Error& e ) {

      mexErrMsgTxt( e.what() );

   }
}
