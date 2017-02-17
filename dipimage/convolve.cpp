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

namespace {

dip::OneDimensionalFilter GetFilter(
      mxArray const* mxFilter, // A struct array
      dip::uint ii
) {
   dip::OneDimensionalFilter out;
   mxArray const* elem = mxGetField( mxFilter, ii, "filter" );
   DIP_THROW_IF( !elem, "" );
   out.filter = dml::GetFloatArray( elem );

   elem = mxGetField( mxFilter, ii, "origin" );
   if( elem ) {
      out.origin = dml::GetInteger( elem );
   }

   elem = mxGetField( mxFilter, ii, "flags" );
   if( elem ) {
      out.symmetry = dml::GetString( elem );
   }

   return out;
}

} // namespace

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   //dml::streambuf streambuf;

   char const* wrongFilter = "Wrong filter definition";

   try {

      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 3 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image out = mi.NewImage();

      dip::OneDimensionalFilterArray filterArray;
      mxArray const* mxFilter = prhs[ 1 ];
      if( mxIsCell( mxFilter )) {
         DIP_THROW_IF( !dml::IsVector( mxFilter ), wrongFilter );
         dip::uint n = mxGetNumberOfElements( mxFilter );
         filterArray.resize( n );
         for( dip::uint ii = 0; ii < n; ++ii ) {
            mxArray const* elem = mxGetCell( mxFilter, ii );
            try {
               filterArray[ ii ].filter = dml::GetFloatArray( elem );
            } catch( dip::Error& ) {
               DIP_THROW( wrongFilter );
            }
         }
      } else if( mxIsNumeric( mxFilter )) {
         DIP_THROW_IF( !dml::IsVector( mxFilter ), wrongFilter );
         filterArray.resize( 1 );
         try {
            filterArray[ 0 ].filter = dml::GetFloatArray( mxFilter );
         } catch( dip::Error& ) {
            DIP_THROW( wrongFilter );
         }
      } else if( mxIsStruct( mxFilter )) {
         dip::uint n = mxGetNumberOfElements( mxFilter );
         filterArray.resize( n );
         for( dip::uint ii = 0; ii < n; ++ii ) {
            try {
               filterArray[ ii ] = GetFilter( mxFilter, ii );
            } catch( dip::Error& ) {
               DIP_THROW( wrongFilter );
            }
         }
      }

      dip::StringArray bc;
      if( nrhs > 2 ) {
         bc = dml::GetStringArray( prhs[ 2 ]);
      }

      dip::SeparableConvolution( in, out, filterArray, bc );

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
