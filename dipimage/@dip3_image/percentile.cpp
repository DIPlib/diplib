/*
 * DIPimage 3.0
 * This MEX-file implements the 'percentile' function
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#define DOCTEST_CONFIG_IMPLEMENT
#include "dip_matlab_interface.h"
#include "diplib/math.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, mxArray const* prhs[] ) {
   try {

      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 4 );

      dml::MatlabInterface mi;

      dip::Image in;
      dip::Image mask;
      dip::Image out = mi.NewImage();
      // TODO: second (optional) output is position

      // Get images
      in = dml::GetImage( prhs[ 0 ] );
      if( nrhs > 2 ) {
         mask = dml::GetImage( prhs[ 2 ] );
      }

      // Get parameter
      dip::dfloat percentile = dml::GetFloat( prhs[ 1 ] );

      // Get optional process array
      dip::BooleanArray process;
      if( nrhs > 3 ) {
         process = dml::GetProcessArray( prhs[ 3 ], in.Dimensionality() );
      }

      // Do the thing
      dip::Percentile( in, mask, out, percentile, process );

      // Done
      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
