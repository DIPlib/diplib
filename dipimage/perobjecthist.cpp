/*
 * DIPimage 3.0
 * This MEX-file implements the `perobjecthist` function
 *
 * (c)2018, Cris Luengo.
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
#include "diplib/histogram.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {

   try {

      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 5 );

      dip::Image const grey = dml::GetImage( prhs[ 0 ] );
      dip::Image const labels = dml::GetImage( prhs[ 1 ] );

      dip::Histogram::Configuration conf;
      if( nrhs > 2 ) {
         conf = dml::GetHistogramConfiguration( prhs[ 2 ] );
      } else {
         // Default configuration
         conf = { 0.0, 100.0, 100 };
         conf.lowerIsPercentile = true;
         conf.upperIsPercentile = true;
      }

      dip::String mode = nrhs > 3 ? dml::GetString( prhs[ 3 ] ) : dip::S::FRACTION;
      dip::String const& background = nrhs > 4 ? dml::GetString( prhs[ 4 ] ) : dip::S::EXCLUDE;

      dip::Distribution out = dip::PerObjectHistogram( grey, labels, {}, conf, mode, background );

      plhs[ 0 ] = dml::GetArray( out );

   } DML_CATCH
}
