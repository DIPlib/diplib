/*
 * DIPimage 3.0
 * This MEX-file implements the `pathopening` function
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

#include "dip_matlab_interface.h"
#include "diplib/morphology.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 4 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image out = mi.NewImage();

      dip::IntegerArray params;
      if( nrhs > 1 ) {
         params = dml::GetIntegerArray( prhs[ 1 ] );
      }
      dip::String polarity = dip::S::OPENING;
      if( nrhs > 2 ) {
         polarity = dml::GetString( prhs[ 2 ] );
      }
      dip::String mode = dip::S::NORMAL;
      if( nrhs > 3 ) {
         mode = dml::GetString( prhs[ 3 ] );
      }

      if( params.size() < 2 ) {
         dip::uint length = 7;
         if( !params.empty() ) {
            length = dip::clamp_cast< dip::uint >( params[ 0 ] );
         }
         dip::PathOpening( in, {}, out, length, polarity, mode );
      } else {
         dip::DirectedPathOpening( in, {}, out, params, polarity, mode );
      }

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
