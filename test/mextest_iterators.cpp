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

      dip::sint v = 0;
      dip::ImageIterator<dip::dfloat> it( out );
      do {
         if( v % 2 ) {
            *it = -( *it );
         }
         ++v;
      } while( ++it );

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
