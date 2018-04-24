/*
 * DIPimage 3.0
 * This MEX-file implements the `threshold` function
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
#include "diplib/segmentation.h"
#include "diplib/statistics.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 3 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image out = mi.NewImage();

      dip::String method = "isodata";
      if( nrhs > 1 ) {
         method = dml::GetString( prhs[ 1 ] );
      }

      if(( method == "double" ) || ( method == "hysteresis" )) {
         dip::dfloat param1;
         dip::dfloat param2;
         if( nrhs > 2 ) {
            dip::FloatArray parameter = dml::GetFloatArray( prhs[ 2 ] );
            DIP_THROW_IF( parameter.size() != 2, dip::E::ARRAY_PARAMETER_WRONG_LENGTH );
            param1 = parameter[ 0 ];
            param2 = parameter[ 1 ];
         } else {
            auto lims = dip::MaximumAndMinimum( in );
            dip::dfloat range = lims.Maximum() - lims.Minimum();
            param1 = lims.Minimum() + range / 3.0;
            param2 = lims.Minimum() + 2.0 * range / 3.0;
         }
         if( method == "double" ) {
            dip::RangeThreshold( in, out, param1, param2 );
         } else {
            dip::HysteresisThreshold( in, out, param1, param2 );
         }
         if( nlhs > 1 ) {
            plhs[ 1 ] = dml::CreateDouble2Vector( param1, param2 );
         }
      } else if(( method == "isodata" ) || ( method == "kmeans" )) {
         dip::uint nThresholds = 1;
         if( nrhs > 2 ) {
            dip::dfloat parameter = dml::GetFloat( prhs[ 2 ] );
            if(( parameter > 1.0 ) && ( parameter <= std::numeric_limits< dip::uint16 >::max() )) {
               nThresholds = static_cast< dip::uint >( parameter );
            }
         }
         dip::FloatArray thresholds = IsodataThreshold( in, {}, out, nThresholds );
         if( nlhs > 1 ) {
            plhs[ 1 ] = dml::GetArray( thresholds );
         }
      } else {
         dip::dfloat parameter = std::numeric_limits< dip::dfloat >::infinity();
         if( nrhs > 2 ) {
            parameter = dml::GetFloat( prhs[ 2 ] );
         }
         dip::dfloat threshold = dip::Threshold( in, out, method, parameter );
         if( nlhs > 1 ) {
            plhs[ 1 ] = dml::GetArray( threshold );
         }
      }

      plhs[ 0 ] = dml::GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
