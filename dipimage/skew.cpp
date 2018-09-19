/*
 * DIPimage 3.0
 * This MEX-file implements the `skew` function
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
#include "diplib/geometry.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 3 );
      DML_MAX_ARGS( 6 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image out = mi.NewImage();

      dip::dfloat shear = dml::GetFloat( prhs[ 1 ] );
      dip::uint skew = dml::GetUnsigned( prhs[ 2 ] );
      DIP_THROW_IF( skew == 0, dip::E::INVALID_PARAMETER );
      --skew;
      dip::uint axis;
      if( nrhs > 3 ) {
         axis = dml::GetUnsigned( prhs[ 3 ] );
         DIP_THROW_IF( axis == 0, dip::E::INVALID_PARAMETER );
         --axis;
      } else {
         axis = skew == 0 ? 1 : 0;
      }

      dip::String method = "";
      if( nrhs > 4 ) {
         method = dml::GetString( prhs[ 4 ] );
      }
      dip::String boundaryCondition = "";
      if( nrhs > 5 ) {
         boundaryCondition = dml::GetString( prhs[ 5 ] );
      }

      dip::Skew( in, out, shear, skew, axis, method, boundaryCondition );

      // TODO: Implement also the other form of skew, using `shearArray`.

      plhs[ 0 ] = dml::GetArray( out );

   } DML_CATCH
}
