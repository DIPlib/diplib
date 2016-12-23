/*
 * Testing the measurement infrastructure
 *
 */

#define DOCTEST_CONFIG_IMPLEMENT
#include <iostream>
#include "dip_matlab_interface.h"
#include "diplib/measurement.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   dml::streambuf streambuf;
   if( nrhs != 2 ) {
      mexErrMsgTxt( "Two input images expected" );
   }

   try {

      dml::MatlabInterface mi;
      dip::Image label = dml::GetImage( prhs[ 0 ] );
      //label.SetPixelSize( dip::PhysicalQuantity( 0.5, dip::Units::Micrometer() ));
      dip::Image const grey = dml::GetImage( prhs[ 1 ] );

      dip::MeasurementTool tool;
      dip::StringArray features2D = { "Feret", "Gravity", "P2A" };
      dip::StringArray features3D = { "Gravity", "P2A" };
      dip::Measurement msr = tool.Measure( label, grey, (label.Dimensionality() == 2) ? features2D : features3D, {}, label.Dimensionality() );
      // "P2A" adds 2 of the other 3 features that we currently have defined: "Size", "Perimeter" (2D), "SurfaceArea" (3D)
      // Call with 2D inputs and 3D inputs to test all features, which tests all infrastructure functionality

      if( nlhs > 0 ) {

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

      } else {

         /*
         double* data = msr.Data();
         *data = 1.01234567e0;
         data += msr.Stride();
         *data = 1.01234567e1;
         data += msr.Stride();
         *data = 1.01234567e2;
         data += msr.Stride();
         *data = 1.01234567e3;
         data += msr.Stride();
         *data = 1.01234567e4;
         data += msr.Stride();
         *data = 1.01234567e5;
         data += msr.Stride();
         *data = 1.01234567e8;
         data += msr.Stride();
         *data = -1.01234567e13;
         data += msr.Stride();
         *data = 1.01234567e-10;
         data += msr.Stride();
         *data = 0.123456;
         */
         std::cout << msr << std::endl;

      }

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
