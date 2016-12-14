/*
 * Testing the measurement infrastructure
 *
 */

#include <iostream>
#include "dip_matlab_interface.h"
#include "diplib/measurement.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   if( nrhs != 2 ) {
      mexErrMsgTxt( "Two input images expected" );
   }

   try {

      dml::MatlabInterface mi;
      dip::Image const label = dml::GetImage( prhs[ 0 ] );
      dip::Image const grey = dml::GetImage( prhs[ 1 ] );

      dip::MeasurementTool tool;
      dip::StringArray features2D = { "Feret", "Gravity", "P2A" };
      dip::StringArray features3D = { "Gravity", "P2A" };
      dip::Measurement msr = tool.Measure( label, grey, (label.Dimensionality() == 2) ? features2D : features3D, {}, label.Dimensionality() );
      // "P2A" adds 2 of the other 3 features that we currently have defined: "Size", "Perimeter" (2D), "SurfaceArea" (3D)
      // Call with 2D inputs and 3D inputs to test all features, which tests all infrastructure functionality

      plhs[ 0 ] = mxCreateDoubleMatrix( msr.NumberOfValues(), msr.NumberOfObjects(), mxREAL );
      double* data = mxGetPr( plhs[ 0 ] );
      auto objIt = msr.FirstObject();
      do {
         auto ftrIt = objIt.FirstFeature();
         do {
            for( auto& value : ftrIt ) {
               *data = value;
               ++data;
            }
         } while( ++ftrIt );
      } while( ++objIt );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
