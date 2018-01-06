/*
 * DIPimage 3.0
 * This MEX-file implements the `subpixlocation` function
 *
 * (c)2010, 2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dip_matlab_interface.h"
#include "diplib/analysis.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 4 );

      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::CoordinateArray coords = dml::GetCoordinateArray( prhs[ 1 ] );

      dip::String method = dip::S::PARABOLIC_SEPARABLE;
      if( nrhs > 2 ) {
         method = dml::GetString( prhs[ 2 ] );
         // Method names are different in the DIPimage interface...
         if(( method == "parabolic nonseparable" ) || ( method == "parabolic_nonseparable" )) {
            method = dip::S::PARABOLIC;
         } else if(( method == "gaussian nonseparable" ) || ( method == "gaussian_nonseparable" )) {
            method = dip::S::GAUSSIAN;
         } else if( method == "parabolic" ) {
            method = dip::S::PARABOLIC_SEPARABLE;
         } else if( method == "gaussian" ) {
            method = dip::S::GAUSSIAN_SEPARABLE;
         }
      }
      dip::String polarity = dip::S::MAXIMUM;
      if( nrhs > 3 ) {
         polarity = dml::GetString( prhs[ 3 ] );
      }

      dip::uint N = coords.size();
      dip::uint nDims = in.Dimensionality();
      plhs[ 0 ] = mxCreateDoubleMatrix( N, nDims, mxREAL );
      double* coords_data = mxGetPr( plhs[ 0 ] );

      double* vals_data = nullptr;
      if( nlhs > 1 ) {
         plhs[ 1 ] = mxCreateDoubleMatrix( N, 1, mxREAL );
         vals_data = mxGetPr( plhs[ 1 ] );
      }

      for( dip::uint ii = 0; ii < N; ++ii ) {
         dip::SubpixelLocationResult loc;
         bool use = true;
         for( dip::uint jj = 0; jj < nDims; ++jj ) {
            if(( coords[ ii ][ jj ] == 0 ) || ( coords[ ii ][ jj ] >= in.Size( jj ) - 1 )) {
               use = false;
            }
         }
         if( use ) {
            loc = dip::SubpixelLocation( in, coords[ ii ], polarity, method );
         } else {
            loc.coordinates = dip::FloatArray( coords[ ii ] );
            loc.value = 0.0;
         }
         for( dip::uint jj = 0; jj < nDims; ++jj ) {
            coords_data[ jj * N ] = loc.coordinates[ jj ];
         }
         ++coords_data;
         if( vals_data ) {
            *vals_data = loc.value;
            ++vals_data;
         }
      }

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
