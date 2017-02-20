/*
 * DIPimage 3.0
 * This MEX-file implements the 'mean' function
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#define DOCTEST_CONFIG_IMPLEMENT
#include "dip_matlab_interface.h"
#include "diplib/math.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, mxArray const* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 3 );

      dml::MatlabInterface mi;

      dip::Image in;
      dip::Image mask;
      dip::Image out = mi.NewImage();

      // Get images
      in = dml::GetImage( prhs[ 0 ] );
      if( nrhs > 1 ) {
         mask = dml::GetImage( prhs[ 1 ] );
      }

      // Get optional process array
      dip::BooleanArray process;
      if( nrhs > 2 ) {
         process = dml::GetProcessArray( prhs[ 2 ], in.Dimensionality() );
      }

      // Do the thing
      dip::Mean( in, mask, out, "", process );

      // Done
      if( nrhs > 2 ) {
         plhs[ 0 ] = mi.GetArray( out );
      } else {
         // TODO: we must either take the max over all tensor elements, or return the max for each tensor element.
         plhs[ 0 ] = dml::GetArray( static_cast< dip::dfloat >( out ));
      }

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
