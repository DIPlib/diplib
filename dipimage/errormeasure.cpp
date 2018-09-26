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

#include "dip_matlab_interface.h"
#include "diplib/statistics.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 4 );

      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image const reference = dml::GetImage( prhs[ 1 ] );

      dip::Image mask = ( nrhs > 2 ) ? dml::GetImage( prhs[ 2 ] ) : dip::Image{};

      dip::String method = ( nrhs > 3 ) ? dml::GetString( prhs[ 3 ] ) : "mse";
      dml::ToLower( method );

      dip::dfloat error;
      if( method == "mse" ) {
         error = dip::MeanSquareError( in, reference, mask );
      } else if( method == "rmse" ) {
         error = dip::RootMeanSquareError( in, reference, mask );
      } else if( method == "me" ) {
         error = dip::MeanError( in, reference, mask );
      } else if( method == "mae" ) {
         error = dip::MeanAbsoluteError( in, reference, mask );
      } else if( method == "idivergence" ) {
         error = dip::IDivergence( in, reference, mask );
      } else if( method == "inproduct" ) {
         error = dip::InProduct( in, reference, mask );
      } else if( method == "lnnormerror" ) {
         error = dip::LnNormError( in, reference, mask );
      } else if( method == "psnr" ) {
         error = dip::PSNR( in, reference, mask );
      } else if( method == "ssim" ) {
         error = dip::SSIM( in, reference, mask );
      } else if( method == "mutualinformation" ) {
         error = dip::MutualInformation( in, reference, mask );
      } else if( method == "dice" ) {
         error = dip::DiceCoefficient( in, reference );
      } else if( method == "jaccard" ) {
         error = dip::JaccardIndex( in, reference );
      } else if( method == "specificity" ) {
         error = dip::Specificity( in, reference );
      } else if( method == "sensitivity" ) {
         error = dip::Sensitivity( in, reference );
      } else if( method == "accuracy" ) {
         error = dip::Accuracy( in, reference );
      } else if( method == "precision" ) {
         error = dip::Precision( in, reference );
      } else if( method == "hausdorff" ) {
         error = dip::HausdorffDistance( in, reference );
      } else {
         DIP_THROW_INVALID_FLAG( method );
      }

      plhs[ 0 ] = dml::GetArray( error );

   } DML_CATCH
}
