/*
 * DIPimage 3.0
 * This MEX-file implements the `lee` function
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
#include "diplib/morphology.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 6 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image out = mi.NewImage();

      int index = 1;
      auto se = dml::GetKernel< dip::StructuringElement >( nrhs, prhs, index, in.Dimensionality() );

      dip::String edgeType = dip::S::TEXTURE;
      if( nrhs > index ) {
         edgeType = dml::GetString( prhs[ index ] );
         ++index;
      }
      dip::String sign = dip::S::UNSIGNED;
      if( nrhs > index ) {
         sign = dml::GetString( prhs[ index ] );
         ++index;
      }
      dip::StringArray bc;
      if( nrhs > index ) {
         bc = dml::GetStringArray( prhs[ index ] );
      }

      dip::Lee( in, out, se, edgeType, sign, bc );

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
