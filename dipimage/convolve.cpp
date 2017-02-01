/*
 * DIPimage 3.0
 * This MEX-file implements the `convolve` function
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

      if( nrhs != 2 ) {
         DIP_THROW( "Two inputs expected" );
      }

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ]);
      dip::Image out = mi.NewImage();

      DIP_THROW_IF( !mxIsCell( prhs[ 1 ] ) || !dml::IsVector( prhs[ 1 ] ), "Filter must be a cell array" );
      dip::uint n = mxGetNumberOfElements( prhs[ 1 ] );
      dip::OneDimensionalFilterArray filterArray( n );
      for( dip::uint ii = 0; ii < n; ++ii ) {
         mxArray const* elem = mxGetCell( prhs[ 1 ], ii );
         try {
            filterArray[ ii ].filter = dml::GetFloatArray( elem );
            filterArray[ ii ].symmetry = "";
         } catch( dip::Error& ) {
            DIP_THROW( "Filter must be a cell array with vectors." );
         }
      }

      dip::SeparableConvolution( in, out, filterArray );

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
