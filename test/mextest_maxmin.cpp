/*
 * Testing assorted DIPlib functionality
 *
 */

#define DOCTEST_CONFIG_IMPLEMENT
#include "dip_matlab_interface.h"
#include "diplib/math.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   if(( nrhs < 1 ) || ( nrhs > 2 )) {
      mexErrMsgTxt( "One or two input images expected" );
   }
   try {
      dml::MatlabInterface mi;
      dip::Image const img = dml::GetImage( prhs[ 0 ] );
      dip::Image const mask = nrhs > 1 ? dml::GetImage( prhs[ 1 ] ) : dip::Image();
      dip::MaximumAndMinimum res = dip::GetMaximumAndMinimum( img, mask );
      plhs[ 0 ] = mxCreateDoubleMatrix( 1, 2, mxREAL );
      double* ptr = mxGetPr( plhs[ 0 ] );
      ptr[ 0 ] = res.min;
      ptr[ 1 ] = res.max;
   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
