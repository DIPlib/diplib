/*
 * DIPimage 3.0
 * This MEX-file implements the `findminima` function
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 * Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
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

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 3 );

      dip::Image const in = dml::GetImage( prhs[ 0 ] );

      dip::Image mask;
      int index = 1;
      if(( nrhs > index ) && ( !mxIsChar( prhs[ index ] ))) {
         mask = dml::GetImage( prhs[ index ] );
         ++index;
      }

      dip::String method = dip::S::PARABOLIC_SEPARABLE;
      if( nrhs > index ) {
         method = dml::GetString( prhs[ index ] );
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

      dip::SubpixelLocationArray out = dip::SubpixelMinima( in, mask, method );

      dip::uint N = out.size();
      dip::uint nDims = in.Dimensionality();
      plhs[ 0 ] = mxCreateDoubleMatrix( N, nDims, mxREAL );
      double* data = mxGetPr( plhs[ 0 ] );
      for( auto const& loc : out ) {
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            data[ ii * N ] = loc.coordinates[ ii ];
         }
         ++data;
      }

      if( nlhs > 1 ) {
         plhs[ 1 ] = mxCreateDoubleMatrix( N, 1, mxREAL );
         data = mxGetPr( plhs[ 1 ] );
         for( auto const& loc : out ) {
            *data = loc.value;
            ++data;
         }
      }

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
