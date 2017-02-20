/*
 * DIPimage 3.0
 * This MEX-file implements the 'min' function
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

      dip::Image in1;
      dip::Image in2; // either 2nd image or mask image
      dip::Image out = mi.NewImage();
      // TODO: second (optional) output is position

      // Get images
      in1 = dml::GetImage( prhs[ 0 ] );
      if( nrhs > 1 ) {
         in2 = dml::GetImage( prhs[ 1 ] );
      }

      // Get optional process array
      dip::BooleanArray process;
      if( nrhs > 2 ) {
         process = dml::GetProcessArray( prhs[ 2 ], in1.Dimensionality() );
      }

      // Operation mode
      if( !in2.IsForged() || in2.DataType().IsBinary() ) {
         // Maximum pixel projection
         if( nlhs > 1 ) {
            // Compute position also
            // TODO
            DIP_THROW( dip::E::NOT_IMPLEMENTED );
         } else {
            // Simply get the maximum projection
            dip::Minimum( in1, in2, out, process );
            if( nrhs > 2 ) {
               plhs[ 0 ] = mi.GetArray( out );
            } else {
               // TODO: we must either take the max over all tensor elements, or return the max for each tensor element.
               plhs[ 0 ] = dml::GetArray( static_cast< dip::dfloat >( out ));
            }
         }
      } else {
         // Maximum over two images
         // TODO
         DIP_THROW( dip::E::NOT_IMPLEMENTED );
         //plhs[ 0 ] = mi.GetArray( out );
      }

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
