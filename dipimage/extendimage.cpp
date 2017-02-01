/*
 * DIPimage 3.0
 * This MEX-file implements the `extendimage` function
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 * Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
 */


#define DOCTEST_CONFIG_IMPLEMENT

#include "dip_matlab_interface.h"
#include "diplib/linear.h"


void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   dml::streambuf streambuf;

   try {

      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 3 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image out = mi.NewImage();

      dip::UnsignedArray border = dml::GetUnsignedArray( prhs[ 1 ] );

      dip::StringArray bc;
      if( nrhs > 2 ) {
         bc = dml::GetStringArray( prhs[ 2 ]);
      }

      dip::ExtendImage( in, out, border, bc );

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
