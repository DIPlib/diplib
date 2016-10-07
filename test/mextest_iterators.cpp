/*
 * Testing MEX-file functionality
 *
 */

#include "dip_matlab_interface.h"
#include "diplib/iterators.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, mxArray const* prhs[] ) {
   try {

      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();

      if( nrhs > 0 ) {
         dip::Image const in = dml::GetImage( prhs[ 0 ] );
         dip::Convert( in, out, dip::DT_DFLOAT );
      } else {
         out.SetDataType( dip::DT_DFLOAT );
         out.SetSizes( { 10, 8 } );
         out.Forge();
      }

      dip::dfloat v = 0;
      for( dip::ImageIterator<dip::dfloat> it( out ); it; ++it ) {
         *it = v;
         ++v;
      }

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
