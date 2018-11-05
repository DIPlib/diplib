/*
 * DIPimage 3.0
 * This MEX-file implements the `affine_trans` function
 *
 * (c)2018, Cris Luengo.
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

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 5 );

      dip::Image const in = dml::GetImage( prhs[ 0 ] );

      dip::FloatArray matrix;
      int index;
      if( nrhs >= 4 ) {
         // affine_trans( image_in, zoom, translation, angle [, method] )
         DIP_THROW_IF( in.Dimensionality() != 2, "When given ZOOM, TRANSLATION and ANGLE, the image must be 2D" );
         dip::FloatArray zoom = dml::GetFloatArray( prhs[ 1 ] );
         if( zoom.size() == 1 ) {
            zoom.push_back( zoom[ 0 ] );
         }
         DIP_THROW_IF( zoom.size() != 2, dip::E::ARRAY_PARAMETER_WRONG_LENGTH );
         dip::FloatArray translation = dml::GetFloatArray( prhs[ 2 ] );
         if( translation.size() == 1 ) {
            translation.push_back( translation[ 0 ] );
         }
         DIP_THROW_IF( translation.size() != 2, dip::E::ARRAY_PARAMETER_WRONG_LENGTH );
         dip::dfloat angle = dml::GetFloat( prhs[ 3 ] );
         matrix.resize( 6, 0 );
         matrix[ 0 ] =  zoom[ 0 ] * std::cos( angle );
         matrix[ 1 ] =  zoom[ 1 ] * std::sin( angle );
         matrix[ 2 ] = -zoom[ 0 ] * std::sin( angle );
         matrix[ 3 ] =  zoom[ 1 ] * std::cos( angle );
         matrix[ 4 ] = translation[ 0 ];
         matrix[ 5 ] = translation[ 1 ];
         index = 4;
         if( nlhs > 1 ) {
            plhs[ 1 ] = mxCreateDoubleMatrix( 3, 3, mxREAL );
            double* out = mxGetPr( plhs[ 1 ] );
            out[ 0 ] = matrix[ 0 ];
            out[ 1 ] = matrix[ 1 ];
            out[ 2 ] = 0.0;
            out[ 3 ] = matrix[ 2 ];
            out[ 4 ] = matrix[ 3 ];
            out[ 5 ] = 0.0;
            out[ 6 ] = matrix[ 4 ];
            out[ 7 ] = matrix[ 5 ];
            out[ 8 ] = 1.0;
         }
      } else {
         // affine_trans( image_in, R [, method] )
         dip::uint nDims = in.Dimensionality();
         mxArray const* R = prhs[ 1 ];
         DIP_THROW_IF( mxGetM( R ) != nDims, "Matrix R of wrong size" );
         dip::uint cols = mxGetN( R );
         DIP_THROW_IF(( cols != nDims ) && ( cols != nDims + 1 ), "Matrix R of wrong size" );
         double const* Rptr = mxGetPr( R );
         matrix.resize( nDims * cols );
         std::copy( Rptr, Rptr + nDims * cols, matrix.begin() );
         index = 2;
      }

      dip::String method = dip::S::LINEAR;
      if( nrhs > index ) {
         method = dml::GetString( prhs[ index ] );
      }

      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();
      dip::AffineTransform( in, out, matrix, method );

      plhs[ 0 ] = dml::GetArray( out );

   } DML_CATCH
}
