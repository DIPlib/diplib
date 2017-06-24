/*
 * DIPimage 3.0
 * This MEX-file implements the `getsamplestatistics` function
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

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 2 );

      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::StatisticsAccumulator stats;

      if( nrhs > 1 ) {
         dip::Image const mask = dml::GetImage( prhs[ 1 ] );
         stats = dip::SampleStatistics( in, mask );
      } else {
         stats = dip::SampleStatistics( in );
      }

      plhs[ 0 ] = mxCreateDoubleMatrix( 1, 4, mxREAL );
      auto data = mxGetPr( plhs[ 0 ] );
      data[ 0 ] = stats.Mean();
      data[ 1 ] = stats.Variance();
      data[ 2 ] = stats.Skewness();
      data[ 3 ] = stats.ExcessKurtosis();

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
