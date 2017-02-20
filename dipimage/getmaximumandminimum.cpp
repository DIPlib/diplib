/*
 * DIPimage 3.0
 * This MEX-file implements the `getmaximumandminimum` function
 *
 * (c)2017, Cris Luengo.
 */


#define DOCTEST_CONFIG_IMPLEMENT

#include "dip_matlab_interface.h"
#include "diplib/math.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 2 );

      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::MinMaxAccumulator minmax;

      if( nrhs > 1 ) {
         dip::Image const mask = dml::GetImage( prhs[ 1 ] );
         minmax = dip::GetMaximumAndMinimum( in, mask );
      } else {
         minmax = dip::GetMaximumAndMinimum( in );
      }

      plhs[ 0 ] = mxCreateDoubleMatrix( 1, 2, mxREAL );
      auto data = mxGetPr( plhs[ 0 ] );
      data[ 0 ] = minmax.Minimum();
      data[ 1 ] = minmax.Maximum();

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
