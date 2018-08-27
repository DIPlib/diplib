/*
 * DIPimage 3.0
 * This MEX-file implements the `psf` function
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
#include "diplib/microscopy.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 0 );
      DML_MAX_ARGS( 5 );

      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();

      /*
      sizes
      method = 'PSF', 'Stokseth', 'Hopkins'
      dfloat oversampling,
      dfloat amplitude,
      dfloat defocus,  -- not for PSF
       */

      out.DataType() = dip::DT_SFLOAT;
      if( nrhs > 0 ) {
         if( mxIsNumeric( prhs[ 0 ] ) && dml::IsVector( prhs[ 0 ] )) {
            out.SetSizes( dml::GetUnsignedArray( prhs[ 0 ] ));
         } else {
            dip::Image tmp = dml::GetImage( prhs[ 0 ] );
            out.SetSizes( tmp.Sizes());
            out.SetPixelSize( tmp.PixelSize());
         }
      }

      dip::String method = nrhs > 1 ? dml::GetString( prhs[ 1 ] ) : "PSF";
      dip::dfloat oversampling = nrhs > 2 ? dml::GetFloat( prhs[ 2 ] ) : 1.0;
      dip::dfloat amplitude = nrhs > 3 ? dml::GetFloat( prhs[ 3 ] ) : 1.0;

      if( method == "PSF" ) {
         dip::IncoherentPSF( out, oversampling, amplitude );
      } else {
         if( method == "OTF" ) {
            method = "Stokseth";
         }
         dip::dfloat defocus = nrhs > 4 ? dml::GetFloat( prhs[ 4 ] ) : 0.0;
         dip::IncoherentOTF( out, defocus, oversampling, amplitude, method );
      }

      plhs[ 0 ] = dml::GetArray( out );

   } DML_CATCH
}
