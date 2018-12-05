/*
 * DIPimage 3.0
 *
 * (c)2017-2018, Cris Luengo.
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

#include "diplib/microscopy.h"

namespace {

void psf( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 5 );
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   out.DataType() = dip::DT_SFLOAT;
   if( nrhs > 0 ) {
      if( mxIsNumeric( prhs[ 0 ] ) && dml::IsVector( prhs[ 0 ] )) {
         out.SetSizes( dml::GetUnsignedArray( prhs[ 0 ] ));
      } else {
         dip::Image tmp = dml::GetImage( prhs[ 0 ] );
         out.SetSizes( tmp.Sizes() );
         out.SetPixelSize( tmp.PixelSize() );
      }
   }
   dip::String method = nrhs > 1 ? dml::GetString( prhs[ 1 ] ) : "PSF";
   dip::dfloat oversampling = nrhs > 2 ? dml::GetFloat( prhs[ 2 ] ) : 1.0;
   dip::dfloat amplitude = nrhs > 3 ? dml::GetFloat( prhs[ 3 ] ) : 1.0;
   if(( method == "PSF" ) || ( method == "psf" )) { // We're allowing lowercase here because the MATLAB help command forces the string in the documentation to lowercase.
      dip::IncoherentPSF( out, oversampling, amplitude );
   } else {
      if(( method == "OTF" ) || ( method == "otf" )) {
         method = "Stokseth";
      }
      dip::dfloat defocus = nrhs > 4 ? dml::GetFloat( prhs[ 4 ] ) : 0.0;
      dip::IncoherentOTF( out, defocus, oversampling, amplitude, method );
   }
   plhs[ 0 ] = dml::GetArray( out );
}

void wiener( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::Image const psf = dml::GetImage( prhs[ 1 ] );
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::dfloat reg = 1e-4;
   bool hasReg = false;
   if( nrhs > 2 ) {
      if ( mxIsDouble( prhs[ 2 ] ) && dml::IsScalar( prhs[ 2 ] )) {
         reg = dml::GetFloat( prhs[ 2 ] );
         hasReg = true;
      } else {
         reg = -1;
      }
   }
   if( reg < 0 ) {
      // image_out = wiener(image_in,psf,S,N)    (hasReg == false)
      // image_out = wiener(image_in,psf,-1,N,S) (hasReg == true)
      DML_MIN_ARGS( 4 );
      DML_MAX_ARGS( hasReg ? 5 : 4 );
      dip::Image const N = dml::GetImage( prhs[ 3 ] );
      dip::Image const S = ( !hasReg || ( nrhs > 4 )) ? dml::GetImage( prhs[ hasReg ? 4 : 2 ] ) : dip::Image{};
      dip::WienerDeconvolution( in, psf, S, N, out );
   } else {
      // image_out = wiener(image_in,psf,reg)
      DML_MAX_ARGS( 5 ); // ignore additional two input arguments
      dip::WienerDeconvolution( in, psf, out, reg );
   }
   plhs[ 0 ] = dml::GetArray( out );
}

} // namespace

// Gateway function

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {
      DML_MIN_ARGS( 1 );
      dip::String function = dml::GetString( prhs[ 0 ] );
      prhs += 1;
      nrhs -= 1;

      if( function == "psf" ) {
         psf( plhs, nrhs, prhs );
      } else if( function == "wiener" ) {
         wiener( plhs, nrhs, prhs );

      } else {
         DIP_THROW_INVALID_FLAG( function );
      }

   } DML_CATCH
}
