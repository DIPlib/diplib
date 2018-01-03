/*
 * DIPimage 3.0
 * This MEX-file implements the `rotation` function
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
#include "diplib/geometry.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 2 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image out = mi.NewImage();

      dip::dfloat angle = dml::GetFloat( prhs[ 1 ] );

      dip::uint nDims = in.Dimensionality();
      DIP_THROW_IF( nDims < 2, "Defined only for images with 2 or more dimensions" );
      dip::uint dimension1 = 0;
      dip::uint dimension2 = dimension1 + 1;
      dip::String method = "";
      dip::String boundaryCondition = "add zeros";

      if(( nrhs > 2 ) && ( mxIsChar( prhs[ 2 ] ))) {
         // rotation(image_in,angle,interpolation_method,boundary_condition)
         DIP_THROW_IF( nDims != 2, "Missing argument before INTERPOLATION_METHOD" );
         DML_MAX_ARGS( 4 );
         method = dml::GetString( prhs[ 2 ] );
         if( nrhs > 3 ) {
            boundaryCondition = dml::GetString( prhs[ 3 ] );
         }
      } else if(( nrhs == 3 ) || (( nrhs > 3 ) && mxIsChar( prhs[ 3 ]))) {
         // rotation(image_in,angle,axis,interpolation_method,boundary_condition)
         DIP_THROW_IF( nDims > 3, "For images with more than 3 dimensions, use the syntax with two DIMENSION parameters" );
         DML_MAX_ARGS( 5 );
         dip::uint axis = dml::GetUnsigned( prhs[ 2 ] );
         if( nDims == 3 ) { // Ignore value if nDims == 2.
            switch( axis ) {
               case 1:
                  dimension1 = 1;
                  dimension2 = 2;
                  break;
               case 2:
                  dimension1 = 2;
                  dimension2 = 0;
                  break;
               case 3:
                  dimension1 = 0;
                  dimension2 = 1;
                  break;
               default:
                  DIP_THROW( dip::E::PARAMETER_OUT_OF_RANGE );
            }
         }
         if( nrhs > 3 ) {
            method = dml::GetString( prhs[ 3 ] );
         }
         if( nrhs > 4 ) {
            boundaryCondition = dml::GetString( prhs[ 4 ] );
         }
      } else {
         // rotation(image_in,angle,dimension1,dimension2,interpolation_method,boundary_condition)
         DML_MAX_ARGS( 6 );
         if( nrhs > 2 ) {
            dimension1 = dml::GetUnsigned( prhs[ 2 ] );
            DIP_THROW_IF( dimension1 == 0, dip::E::PARAMETER_OUT_OF_RANGE );
            --dimension1;
         }
         if( nrhs > 3 ) {
            dimension2 = dml::GetUnsigned( prhs[ 3 ] );
            DIP_THROW_IF( dimension2 == 0, dip::E::PARAMETER_OUT_OF_RANGE );
            --dimension2;
         }
         if( nrhs > 4 ) {
            method = dml::GetString( prhs[ 4 ] );
         }
         if( nrhs > 5 ) {
            boundaryCondition = dml::GetString( prhs[ 5 ] );
         }
      }

      dip::Rotation( in, out, angle, dimension1, dimension2, method, boundaryCondition );

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
