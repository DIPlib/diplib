/*
 * DIPimage 3.0
 * This MEX-file implements the `select` function
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#define DOCTEST_CONFIG_IMPLEMENT

#include "dip_matlab_interface.h"
#include "diplib/math.h"


void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {

   try {

      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();

      if( nrhs == 3 ) {

         dip::Image const in1 = dml::GetImage( prhs[ 0 ] );
         dip::Image const in2 = dml::GetImage( prhs[ 1 ] );
         dip::Image const mask = dml::GetImage( prhs[ 2 ] );
         dip::Select( in1, in2, mask, out );

      } else if( nrhs == 5 ) {

         dip::Image const in1 = dml::GetImage( prhs[ 0 ] );
         dip::Image const in2 = dml::GetImage( prhs[ 1 ] );
         dip::Image const in3 = dml::GetImage( prhs[ 2 ] );
         dip::Image const in4 = dml::GetImage( prhs[ 3 ] );
         dip::String selector = dml::GetString( prhs[ 4 ] );
         if( selector == "~=" ) {
            selector = "!=";
         }
         dip::Select( in1, in2, in3, in4, out, selector );

      } else {
         DIP_THROW( "Need either 3 or 5 input arguments." );
      }

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
