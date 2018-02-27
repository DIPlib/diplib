/*
 * DIPimage 3.0
 * This MEX-file implements the `gabor` function
 *
 * (c)2018, Cris Luengo.
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
#include "diplib/linear.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image out = mi.NewImage();
      int index = 1;

      dip::FloatArray sigmas = { 5.0 };
      if( nrhs > index ) {
         sigmas = dml::GetFloatArray( prhs[ 1 ] );
         ++index;
      }

      dip::FloatArray frequencies = { 0.15 };
      if( nrhs > index ) {
         frequencies = dml::GetFloatArray( prhs[ 2 ] );
         ++index;
      }

      if(( in.Dimensionality() == 2 ) && ( frequencies.size() == 1 )) {
         dip::dfloat frequency = frequencies[ 0 ];
         dip::dfloat direction = dip::pi;
         if( nrhs > index ) {
            direction = dml::GetFloat( prhs[ index ] );
            ++index;
         }
         frequencies = { frequency * std::cos( direction ), frequency * std::sin( direction ) };
      }

      DML_MAX_ARGS( index + 3 );

      dip::StringArray bc = {};
      if( nrhs > index ) {
         bc = dml::GetStringArray( prhs[ index ] );
         ++index;
      }
      dip::BooleanArray process = {};
      if( nrhs > index ) {
         process = dml::GetProcessArray( prhs[ index ], in.Dimensionality() );
         ++index;
      }
      dip::dfloat truncation = 3;
      if( nrhs > index ) {
         truncation = dml::GetFloat( prhs[ index ] );
      }

      dip::GaborIIR( in, out, sigmas, frequencies, bc, process, {}, truncation );

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
