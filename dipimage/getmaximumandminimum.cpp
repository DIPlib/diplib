/*
 * DIPimage 3.0
 * This MEX-file implements the `getmaximumandminimum` function
 *
 * (c)2017, Cris Luengo.
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

#undef DIP__ENABLE_DOCTEST
#include "dip_matlab_interface.h"
#include "diplib/statistics.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 2 );

      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::MinMaxAccumulator minmax;

      if( nrhs > 1 ) {
         dip::Image const mask = dml::GetImage( prhs[ 1 ] );
         minmax = dip::MaximumAndMinimum( in, mask );
      } else {
         minmax = dip::MaximumAndMinimum( in );
      }

      plhs[ 0 ] = mxCreateDoubleMatrix( 1, 2, mxREAL );
      auto data = mxGetPr( plhs[ 0 ] );
      data[ 0 ] = minmax.Minimum();
      data[ 1 ] = minmax.Maximum();

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
