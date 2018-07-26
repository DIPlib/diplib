/*
 * DIPimage 3.0
 * This MEX-file implements the `loggabor` function
 *
 * (c)2018, Cris Luengo.
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
#include "diplib/linear.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 6 );

      dip::Image in;
      if( mxIsNumeric( prhs[ 0 ] ) && dml::IsVector( prhs[ 0 ] )) {
         in.SetSizes( dml::GetUnsignedArray( prhs[ 0 ] ));
      } else {
         in = dml::GetImage( prhs[ 0 ] );
      }

      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();

      dip::FloatArray wavelengths = nrhs > 1 ? dml::GetFloatArray( prhs[ 1 ] ) : dip::FloatArray{ 3.0, 6.0, 12.0, 24.0 };
      dip::dfloat bandwidth = nrhs > 2 ? dml::GetFloat( prhs[ 2 ] ) : 0.75;
      dip::uint nOrientations = nrhs > 3 ? dml::GetUnsigned( prhs[ 3 ] ) : 6;
      dip::String inRepresentation = nrhs > 4 ? dml::GetString( prhs[ 4 ] ) : dip::String{ dip::S::SPATIAL };
      dip::String outRepresentation = nrhs > 5 ? dml::GetString( prhs[ 5 ] ) : dip::String{ dip::S::SPATIAL };

      dip::LogGaborFilterBank( in, out, wavelengths, bandwidth, nOrientations, inRepresentation, outRepresentation );

      plhs[ 0 ] = dml::GetArray( out );

   } DML_CATCH
}
