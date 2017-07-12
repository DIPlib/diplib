/*
 * DIPimage 3.0
 * This MEX-file implements the `errormeasure` function
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

#undef DIP__ENABLE_DOCTEST
#include "dip_matlab_interface.h"
#include "diplib/statistics.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 4 );

      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image const reference = dml::GetImage( prhs[ 1 ] );

      dip::Image mask;
      dip::String method = "MSE";

      if( nrhs > 2 ) {
         mask = dml::GetImage( prhs[ 2 ] );
      }
      if( nrhs > 3 ) {
         method = dml::GetString( prhs[ 3 ] );
      }

      dip::dfloat error;
      if( method == "MSE" ) {
         error = dip::MeanSquareError( in, reference, mask );
      } else if( method == "RMSE" ) {
         error = dip::RootMeanSquareError( in, reference, mask );
      } else if( method == "ME" ) {
         error = dip::MeanError( in, reference, mask );
      } else if( method == "MAE" ) {
         error = dip::MeanAbsoluteError( in, reference, mask );
      } else if( method == "IDivergence" ) {
         error = dip::IDivergence( in, reference, mask );
      } else if( method == "InProduct" ) {
         error = dip::InProduct( in, reference, mask );
      } else if( method == "LnNormError" ) {
         error = dip::LnNormError( in, reference, mask );
      } else if( method == "PSNR" ) {
         error = dip::PSNR( in, reference, mask );
      } else if( method == "SSIM" ) {
         error = dip::SSIM( in, reference, mask );
      } else {
         DIP_THROW( dip::E::INVALID_FLAG );
      }

      plhs[ 0 ] = dml::GetArray( error );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
