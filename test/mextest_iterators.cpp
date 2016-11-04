/*
 * Testing iterators
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
         out.SetSizes( { 9, 8 } );
         out.Forge();
         out.Fill( 1.0 );
      }

      std::cout << "--- Testing pixel iterator ---\n";
      {
         dip::sint v = 0;
         dip::ImageIterator< dip::dfloat > it( out );
         do {
            if( v % 2 ) {
               *it = -( *it );
            }
            ++v;
         } while( ++it );
      }

      plhs[ 0 ] = mi.GetArray( out );

      std::cout << "--- Testing slice iterator ---\n";
      dip::Image img = mi.NewImage();
      img.SetSizes( { 256, 10, 512 } );
      img.Forge();
      std::cout << img;
      {
         dip::ImageSliceIterator it( img, 1 );
         do {
            //(*it).Fill( double(it.Coordinate()) );
            it->Fill( double( it.Coordinate() ) );
            std::cout << *it;
         } while( ++it );
      }

      if( nlhs > 1 ) {
         plhs[ 1 ] = mi.GetArray( img );
      }

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
