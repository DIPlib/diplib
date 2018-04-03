/*
 * DIPimage 3.0
 * This MEX-file implements the `gaussf_adap` function
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
#include "diplib/analysis.h"
#include "diplib/nonlinear.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 6 );

      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::uint nDims = in.Dimensionality();
      DIP_THROW_IF( nDims < 2 || nDims > 3, dip::E::DIMENSIONALITY_NOT_SUPPORTED );

      dip::ImageArray params;
      if(( nrhs < 2 ) || mxIsEmpty( prhs[ 1 ] )) {
         // Compute orientation
         params = StructureTensorAnalysis( StructureTensor( in ),
                                           nDims == 2 ? dip::StringArray{ "orientation" }
                                                      : dip::StringArray{ "phi3", "theta3" } );
      } else {
         params = dml::GetImageArray( prhs[ 1 ] );
      }

      dip::FloatArray sigmas;
      if( nrhs > 2 ) {
         sigmas = dml::GetFloatArray( prhs[ 2 ] );
      } else {
         sigmas.resize( nDims, 0 );
         if( nDims == 2 ) {
            sigmas.front() = 2.0;
         } else {
            sigmas.back() = 2.0;
         }
      }

      dip::UnsignedArray order = { 0 };
      if( nrhs > 3 ) {
         order = dml::GetUnsignedArray( prhs[ 3 ] );
      }

      dip::UnsignedArray exponents = { 0 };
      if( nrhs > 4 ) {
         exponents = dml::GetUnsignedArray( prhs[ 4 ] );
      }

      dip::dfloat truncation = 2.0;
      if( nrhs > 5 ) {
         truncation = dml::GetFloat( prhs[ 5 ] );
      }

      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();

      dip::AdaptiveGauss( in, dip::CreateImageConstRefArray( params ), out, sigmas, order, truncation, exponents );

      plhs[ 0 ] = dml::GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
