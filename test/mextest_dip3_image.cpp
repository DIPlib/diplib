/*
 * Testing MEX-file functionality
 *
 */

#define DOCTEST_CONFIG_IMPLEMENT
#include "dip_matlab_interface.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, mxArray const* prhs[] ) {
   dml::streambuf streambuf;
   try {
      mxArray* data = mxCreateDoubleMatrix( 5, 6, mxREAL );
      std::cout << "Array pointer = " << data
                << " -- Array data pointer = " << mxGetData( data )
                << std::endl;

      mexCallMATLAB( 1, plhs, 1, &data, "dip3_image" );
      std::cout << "Called MATLAB" << std::endl;

      mxArray* data1 = mxGetPropertyShared( plhs[ 0 ], 0, "Array" );
      std::cout << "Array pointer = " << data1
                << " -- Array data pointer = " << ( data1 ? mxGetData( data1 ) : nullptr )
                << std::endl;

      mxArray* data2 = mxCreateDoubleMatrix( 2, 1, mxREAL );
      std::cout << "Array pointer = " << data2
                << " -- Array data pointer = " << ( data2 ? mxGetData( data2 ) : nullptr )
                << std::endl;

      mxSetPropertyShared( plhs[ 0 ], 0, "Array", data2 );
      std::cout << "Property set" << std::endl;

      mxArray* data3 = mxGetPropertyShared( plhs[ 0 ], 0, "Array" );
      std::cout << "Array pointer = " << data3
                << " -- Array data pointer = " << ( data3 ? mxGetData( data3 ) : nullptr )
                << std::endl;

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
